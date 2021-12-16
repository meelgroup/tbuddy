#include <queue>
#include <unordered_map>
#include <unordered_set>

#include "pseudoboolean.h"
#include "prover.h"

using namespace trustbdd;

#define BUFLEN 2048
// For formatting information
static char ibuf[BUFLEN];

/*
  Statistics gathering
 */
static int pseudo_xor_created = 0;
static int pseudo_xor_unique = 0;
static int pseudo_total_length = 0;
static int pseudo_plus_computed = 0;
static int pseudo_plus_unique = 0;
static int pseudo_arg_clause_count = 0;

// Track previously generated xor constraints
// As consequence, all of the TBDDs for the xor constraints will
// be kept, preventing the deletion of their unit clauses
static std::unordered_map<int, xor_constraint*> xor_map;

// Track previously generated argument pairs for sum computation
static std::unordered_set<int64_t> plus_set;

static int show_xor_buf(char *buf, ilist variables, int phase, int maxlen);
static void pseudo_info_fun(int vlevel);

static bool initialized = false;

static void pseudo_init() {
    if (!initialized) {
	tbdd_add_info_fun(pseudo_info_fun);
    }
    initialized = true;
}

static void pseudo_info_fun(int vlevel) {
    if (vlevel < 1)
	return;
    printf("Number of XOR constraints used: %d\n", pseudo_xor_created);
    printf("Number of unique XOR constraints: %d\n", pseudo_xor_unique);
    if (pseudo_xor_unique > 0)
	printf("Average (unique) constraint size: %.2f\n", (double) pseudo_total_length / pseudo_xor_unique);
    printf("Number of XOR additions performed: %d\n", pseudo_plus_computed);
    printf("Number of unique XOR additions: %d\n", pseudo_plus_unique);
    printf("Number of clauses generated from arguments: %d\n", pseudo_arg_clause_count);
}


static void pseudo_done(int vlevel) {
    for (auto p = xor_map.begin(); p != xor_map.end(); p++) {
	xor_constraint *xcp = p->second;
	delete xcp;
    }
    xor_map.clear();
    plus_set.clear();
    tbdd_done();
}

/// Note: Following are functions to set up a priority queue for summation
///       Didn't find any selection that was superior to simple, breadth-first evaluation

/*
  Sort integers in ascending order
*/
int int_compare(const void *i1p, const void *i2p) {
    int i1 = *(int *) i1p;
    int i2 = *(int *) i2p;
    if (i1 < i2)
	return -1;
    if (i1 > i2)
	return 1;
    return 0;
}

/*
  Construct priority key for constraint
*/
static int64_t priority_key1(ilist vars) {
    //Primary = first literal.  Secondary = length
    int64_t weight = ilist_length(vars) == 0 ? 0 : ((int64_t) vars[0] << 32) | ((int64_t) ilist_length(vars) << 0);
    return -weight;
}

static int64_t priority_key2(ilist vars) {
    //Primary = length.  Secondary = first literal
    int64_t weight = ilist_length(vars) == 0 ? 0 : ((int64_t) vars[0] << 0) | ((int64_t) ilist_length(vars) << 32) ;
    return -weight;
}

void show_key(int64_t weight) {
    int upper = -weight >> 32;
    int lower = -weight & 0xFFFFFFFF;
    printf("Key = %d/%d", upper, lower);
}

static int64_t priority_key(ilist vars) {
    return priority_key2(vars);
}

static void show_xor(FILE *outf, ilist variables, int phase) {
    if (variables == NULL)
	fprintf(outf, "NULL");
    if (ilist_length(variables) == 0)
	fprintf(outf, "=2 %d", phase);
    else {
	fprintf(outf, "=2 %d 1.", phase);
	ilist_print(variables, outf, " 1.");
    }
}

static int show_xor_buf(char *buf, ilist variables, int phase, int maxlen) {
    if (variables == NULL)
	return snprintf(buf, maxlen, "NULL");
    if (ilist_length(variables) == 0)
	return snprintf(buf, maxlen, "=2 %d", phase);
    else {
	int len = snprintf(buf, maxlen, "=2 %d 1.", phase);
	if (len >= maxlen)
	    return 0;
	int xlen = ilist_format(variables, buf+len, " 1.", maxlen-len);
	return len + xlen;
    }
}

/* Form xor sum of coefficients */
/* Assumes both sets of variables are in ascending order  */
static ilist coefficient_sum(ilist list1, ilist list2) {
    int i1 = 0;
    int i2 = 0;
    int len1 = ilist_length(list1);
    int len2 = ilist_length(list2);
    ilist result = ilist_new(len1);
    while (i1 < len1 && i2 < len2) {
	int v1 = list1[i1];
	int v2 = list2[i2];
	if (v1 < v2) {
	    result = ilist_push(result, v1);
	    i1++;
	} else if (v2 < v1) {
	    result = ilist_push(result, v2);
	    i2++;
	} else {
	    i1++; i2++;
	}
    }
    while (i1 < len1) {
	int v1 = list1[i1];
	result = ilist_push(result, v1);
	i1++;
    }
    while (i2 < len2) {
	int v2 = list2[i2];
	result = ilist_push(result, v2);
	i2++;
    }
    return result;
}

/*
  Build BDD representation of XOR (phase = 1) or XNOR (phase = 0)
*/
static bdd bdd_build_xor(ilist variables, int phase) {
    qsort((void *) variables, ilist_length(variables), sizeof(int), int_compare);

    bdd r = phase ? bdd_false() : bdd_true();
    int i;
    for (i = 0; i < ilist_length(variables); i++) {
	bdd lit = bdd_ithvar(variables[i]);
	r = bdd_xor(r, lit);
    }
    return r;
}

/*
  Use BDD representation of XOR constraint as canonical representation.
  Keep table of created constraints.
  Sets xfun to derived BDD
 */
static xor_constraint* find_constraint(ilist variables, int phase, bdd &xfun) {
    xfun = bdd_build_xor(variables, phase);
    int id = bdd_nameid(xfun);
    if (xor_map.count(id) > 0) {
	if (verbosity_level >= 3) {
	    show_xor_buf(ibuf, variables, phase, BUFLEN);
	    printf("Reusing constraint N%d.  %s\n", id, ibuf);
	}
	return new xor_constraint(*xor_map[id]);
    } else
	return NULL;
}

static void save_constraint(xor_constraint *xcp) {
    int id = xcp->get_nameid();
    ilist variables = xcp->get_variables();
    int phase = xcp->get_phase();
    xor_map[id] = new xor_constraint(*xcp);
    if (verbosity_level >= 2) {
	show_xor_buf(ibuf, variables, phase, BUFLEN);
	printf("Creating constraint N%d.  %s\n", id, ibuf);
    }
    pseudo_xor_unique ++;
    pseudo_total_length += ilist_length(variables);
}

/// Note: Following was attempt to reuse previously computed sums.
//   Found that did not significantly reduce overall work

static int64_t plus_key(xor_constraint *arg1, xor_constraint *arg2) {
    int id1 = arg1->get_nameid();
    int id2 = arg2->get_nameid();
    if (id1 < id2)
	return ((int64_t) id1 << 32) | id2;
    else
	return ((int64_t) id2 << 32) | id1;
}

// See if sum has already been computed.  If so, return result
// Otherwise NULL
static xor_constraint* check_plus(xor_constraint *arg1, xor_constraint *arg2) {
    int64_t key = plus_key(arg1, arg2);
    if (plus_set.count(key) == 0) {
	return NULL;
    }
    ilist nvariables = coefficient_sum(arg1->get_variables(), arg2->get_variables());
    int nphase = arg1->get_phase() ^ arg2->get_phase();
    bdd xfun;
    pseudo_plus_computed++;
    return find_constraint(nvariables, nphase, xfun);
}

static void record_plus(xor_constraint *arg1, xor_constraint *arg2) {
    int64_t key = plus_key(arg1, arg2);
    plus_set.insert(key);
}


/// Methods & functions for xor constraints

xor_constraint::xor_constraint(ilist vars, int p, tbdd &vfun) {
    pseudo_xor_created ++;
    variables = vars;
    phase = p;
    key = priority_key(vars);
    bdd xfun;
    xor_constraint *xcp = find_constraint(variables, phase, xfun);
    if (xcp == NULL) {
	validation = tbdd_validate(xfun, vfun);
	save_constraint(this);
    } else
	validation = xcp->validation;
}

// When generating DRAT proof, either reuse or generate validation
xor_constraint::xor_constraint(ilist vars, int p) {
    pseudo_xor_created ++;
    variables = vars;
    phase = p;
    key = priority_key(vars);
    bdd xfun;
    xor_constraint *xcp = find_constraint(variables, phase, xfun);
    if (xcp == NULL) {
	int start_clause = total_clause_count;
	validation = tbdd_from_xor(variables, phase);
	save_constraint(this);
	pseudo_arg_clause_count += (total_clause_count - start_clause);
    }
    else
	validation = xcp->validation;
}

int xor_constraint::validate_clause(ilist clause) {
    return tbdd_validate_clause(clause, validation);
}

void xor_constraint::show(FILE *out) {
    fprintf(out, "Xor Constraint: Node N%d validates ", tbdd_nameid(validation));
    show_xor(out, variables, phase);
    fprintf(out, "\n");
}

xor_constraint* trustbdd::xor_plus(xor_constraint *arg1, xor_constraint *arg2) {
    ilist nvariables = coefficient_sum(arg1->variables, arg2->variables);
    int nphase = arg1->phase ^ arg2->phase;
    pseudo_plus_computed++;
    bdd xfun;
    xor_constraint *xcp = find_constraint(nvariables, nphase, xfun);
    if (xcp == NULL) {
	pseudo_plus_unique++;
	tbdd nvalidation = tbdd_and(arg1->validation, arg2->validation);
	xcp = new xor_constraint(nvariables, nphase, nvalidation);
	record_plus(arg1, arg2);
    }
    return xcp;
}

/// Various ways to compute the sum of a list of xor constraints


// Linear evaluation.  Good for small sets
static xor_constraint *xor_sum_list_linear(xor_constraint **xlist, int len) {
    if (len == 0)
	return new xor_constraint();
    xor_constraint *sum = xlist[0];
    for (int i = 1; i < len; i++) {
	xor_constraint *a = xlist[i];
	xor_constraint *nsum = xor_plus(sum, a);
	delete a;
	delete sum;
	sum = nsum;
	if (!sum->is_feasible())
	    break;
    }
    return sum;
}

// Breadth-first computation.  Effectively forms binary tree of additions
static xor_constraint *xor_sum_list_bf(xor_constraint **xlist, int len) {
    if (len == 0)
	return new xor_constraint();
    // Use breadth-first addition
    xor_constraint **xbuf = new xor_constraint*[2*len];
    for (int i = 0; i < len; i++)
	xbuf[i] = xlist[i];
    // Left and right-most positions that have been used in the buffer 
    int left = 0;
    int right = len-1;
    while (left < right) {
	xor_constraint *arg1 = xbuf[left++];
	xor_constraint *arg2 = xbuf[left++];
	xbuf[++right] = xor_plus(arg1, arg2);
	delete arg1;
	delete arg2;
	if (!xbuf[right]->is_feasible())
	    break;
    }
    xor_constraint *sum = xbuf[right];
    delete[] xbuf;
    return sum;
}

// Breadth-first computation, but greedily check if sum has already been computed with another argument
//   This did not work as well as was hoped
static xor_constraint *xor_sum_list_bf_check(xor_constraint **xlist, int len) {
    if (len == 0)
	return new xor_constraint();
    xor_constraint **xbuf = new xor_constraint*[2*len];
    for (int i = 0; i < len; i++)
	xbuf[i] = xlist[i];
    // Left and right-most positions that have been used in the buffer 
    int left = 0;
    int right = len-1;
    while (left < right) {
	int i1, i2;
	bool reused = false;
	// Loop invariant: xbuf[right] != NULL
	xor_constraint *arg1, *arg2, *nsum;
	do {
	    i1 = left;
	    arg1 = xbuf[left++];
	} while (arg1 == NULL); // By LI, this will eventually succeed
	if (left == right)
	    break;
	// See if arg1 has already been added to one of the other arguments
	for (int idx = left; idx <= right; idx++) {
	    i2 = idx;
	    xor_constraint *oarg = xbuf[idx];
	    if (oarg == NULL)
		continue;
	    nsum = check_plus(arg1, oarg);
	    if (nsum != NULL) {
		delete arg1;
		delete oarg;
		reused = true;
		// May temporarily violate loop invariant
		xbuf[idx] = NULL;
		break;
	    }
	}
	if (nsum == NULL) {
	    do {
		i2 = left;
		arg2 = xbuf[left++];
	    } while (arg2 == NULL);
	    nsum = xor_plus(arg1, arg2);
	    delete arg1; 
	    delete arg2;
	}
	// Loop invariant restored
	xbuf[++right] = nsum;
	if (!nsum->is_feasible())
	    break;
    }
    xor_constraint *sum = xbuf[right];
    delete[] xbuf;
    return sum;
}


// Used for priority queue version of summation

class xcomp {
public:
    bool operator() (xor_constraint *arg1, xor_constraint *arg2) {
	return xor_less(arg1, arg2);
    }

};

// Sum using priority queue.  Did not do as well as BF.
static xor_constraint *xor_sum_list_pq(xor_constraint **xlist, int len) {
    if (len == 0)
	return new xor_constraint();
    std::priority_queue<xor_constraint*, std::vector<xor_constraint*>, xcomp> pq;

    for (int i = 0; i < len; i++)
	pq.push(xlist[i]);

    while (pq.size() > 1) {
	xor_constraint *arg1 = pq.top();
	pq.pop();
	xor_constraint *arg2 = pq.top();
	pq.pop();
	xor_constraint *sum = xor_plus(arg1, arg2);
	pq.push(sum);
	delete arg1;
	delete arg2;
	if (!sum->is_feasible())
	    return sum;
    }
    return pq.top();
}

xor_constraint *trustbdd::xor_sum_list(xor_constraint **xlist, int len) {
    if (len <= 4)
	return xor_sum_list_linear(xlist, len);
    else
	return xor_sum_list_bf(xlist, len);
}

xor_set::~xor_set() {
    xlist.clear();
}

void xor_set::add(xor_constraint &con) {
    pseudo_init();
    // Make local copy of the constraint
    xor_constraint *ncon = new xor_constraint(con);
    xlist.push_back(ncon);
}

xor_constraint *xor_set::sum() {
    xor_constraint *xsum = xor_sum_list(xlist.data(), xlist.size());
    xlist.clear();
    return xsum;
}
