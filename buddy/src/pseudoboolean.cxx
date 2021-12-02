#include "pseudoboolean.h"
#include "prover.h"
#include <queue>

#define BUFLEN 2048

// For formatting information
char ibuf[BUFLEN];

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
    int64_t weight = ilist_length(vars) == 0 ? 0 : ((int64_t) vars[0] << 32) | ((int64_t) ilist_length(vars) << 0);
    return -weight;
}

static int64_t priority_key2(ilist vars) {
    int64_t weight = ilist_length(vars) == 0 ? 0 : ((int64_t) vars[0] << 0) | ((int64_t) ilist_length(vars) << 32) ;
    return -weight;
}

void show_key(int64_t weight) {
    int upper = -weight >> 32;
    int lower = -weight & 0xFFFFFFFF;
    printf("Key = %d/%d", upper, lower);
}

static int64_t priority_key(ilist vars) {
    return priority_key1(vars);
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
	int xlen = ilist_format(variables, buf+len, " 1.", maxlen-len);
	return len + xlen;
    }
}

/* Form xor sum of coefficients */
/* Assumes both sets of variables are in ascending order  */
static ilist xor_sum(ilist list1, ilist list2) {
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


xor_constraint::xor_constraint(ilist vars, int p, tbdd &vfun) {
    variables = vars;
    phase = p;
    bdd xfun = bdd_build_xor(variables, phase);
    if (verbosity_level >= 2) {
	//	show_xor_buf(ibuf, vars, p, BUFLEN);
	print_proof_comment(2, "Validate BDD node N%d representing Xor constraint", bdd_nameid(xfun.get_BDD()));
    }
    validation = tbdd_validate(xfun, vfun);
    key = priority_key(vars);
}

// When generating DRAT proof, need to construct XOR from
// clauses so that checker will accept the result
xor_constraint::xor_constraint(ilist vars, int p) {
    variables = vars;
    phase = p;
    validation = tbdd_from_xor(variables, phase);
    key = priority_key(vars);
}

int xor_constraint::validate_clause(ilist clause) {
    return tbdd_validate_clause(clause, validation);
}

void xor_constraint::show(FILE *out) {
    fprintf(out, "Xor Constraint: Node N%d validates ", bdd_nameid(validation.get_root().get_BDD()));
    show_xor(out, variables, phase);
    fprintf(out, "\n");
}

xor_constraint *xor_plus(xor_constraint *arg1, xor_constraint *arg2) {
    ilist nvariables = xor_sum(arg1->variables, arg2->variables);
    int nphase = arg1->phase ^ arg2->phase;
    tbdd nvalidation = tbdd_and(arg1->validation, arg2->validation);

#if 0
    printf("XOR sum: ");
    show_xor(stdout, arg1->variables, arg2->phase);
    printf("  +  ");
    show_xor(stdout, arg1->variables, arg2->phase);
    printf("   -->  ");
    show_xor(stdout, nvariables, nphase);
    printf("\n");
#endif
    return new xor_constraint(nvariables, nphase, nvalidation);
}

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

static xor_constraint *xor_sum_list_bf(xor_constraint **xlist, int len) {
    if (len == 0)
	return new xor_constraint();
    // Use breadth-first addition
    xor_constraint **xbuf = new xor_constraint*[len];
    // Do first level of additions on xlist
    for (int i = 0; i < len-1; i+=2) {
	xbuf[i/2] = xor_plus(xlist[i], xlist[i+1]);
	delete xlist[i];
	delete xlist[i+1];
    }
    if (len %2 == 1)
	xbuf[(len-1)/2] = xlist[len-1];
    // Left and right-most positions that have been used in the buffer 
    int left = 0;
    int right = (len-1)/2;
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

class xcomp {
public:
    bool operator() (xor_constraint *arg1, xor_constraint *arg2) {
	return xor_less(arg1, arg2);
    }

};

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

xor_constraint *xor_sum_list(xor_constraint **xlist, int len) {
    if (len <= 4)
	return xor_sum_list_linear(xlist, len);
    else
	return xor_sum_list_bf(xlist, len);
}

xor_set::~xor_set() {
    //  for (int i = 0; i < xlist.size(); i++)
    //    delete xlist[i];
}

void xor_set::add(xor_constraint *con) {
    xlist.push_back(con);
}

xor_constraint *xor_set::sum() {
    return xor_sum_list(xlist.data(), xlist.size());
    xlist.resize(0,0);
}