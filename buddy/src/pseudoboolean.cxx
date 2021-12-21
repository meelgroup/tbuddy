#include <unordered_map>
#include <set>

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
    tbdd_done();
}

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

/// Methods & functions for xor constraints

xor_constraint::xor_constraint(ilist vars, int p, tbdd &vfun) {
    pseudo_xor_created ++;
    variables = vars;
    phase = p;
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
    }
    return xcp;
}

/// Various ways to compute the sum of a list of xor constraints

///////////////////////////////////////////////////////////////////
// Graph summation:
// Define cost of an addition to be the number of nonzero coefficients in the result.
// Heuristic: On each step, add a pair of arguments that will be of minimum cost.
// The sum graph provides a mechanism for implementing that heuristic.
// There is a node for each remaining argument, and an undirected edge (i,i')
// if nodes i & j share at least one nonzero coeffienct.
// The edges are stored as an ordered set.
// Each step involves removing the least edge, (i,i'), (where i < i'), updating
// node i to be the sum of i & i', and then updating the edges.
// The edges for the new node i will be a subset of the edges (i,j) and (i',j).
///////////////////////////////////////////////////////////////////

/* Determine whether two xor constraints have overlapping coefficients */
static bool xoverlap(xor_constraint *xcp1, xor_constraint *xcp2) {
    ilist list1 = xcp1->get_variables();
    ilist list2 = xcp2->get_variables();
    int i1 = 0;
    int i2 = 0;
    int len1 = ilist_length(list1);
    int len2 = ilist_length(list2);
    while (i1 < len1 && i2 < len2) {
	int v1 = list1[i1];
	int v2 = list2[i2];
	if (v1 < v2)
	    i1++;
	else if (v2 < v1)
	    i2++;
	else 
	    return true;
    }
    return false;
}

/* Determine the cost of adding two xor constraints */
/* 
   Result is 64 bits, where upper bits represent cost function,
   and lower bits make the value unique.
 */
static int64_t xcost(xor_constraint *xcp1, xor_constraint *xcp2, int lower) {
    ilist list1 = xcp1->get_variables();
    ilist list2 = xcp2->get_variables();
    int i1 = 0;
    int i2 = 0;
    int len1 = ilist_length(list1);
    int len2 = ilist_length(list2);
    int upper = 0;
    while (i1 < len1 && i2 < len2) {
	int v1 = list1[i1];
	int v2 = list2[i2];
	if (v1 < v2) {
	    upper++;
	    i1++;
	} else if (v2 < v1) {
	    upper++;
	    i2++;
	} else {
	    // Coefficients will cancel
	    i1++; i2++;
	}
    }
    upper += (len1-i1);
    upper += (len2-i2);
    return ((int64_t) upper << 32) | lower;
}


// Data structure for list edge
class sgraph_edge {
public:
    sgraph_edge(int n1, int n2, int64_t c)
    { node1 = n1, node2 = n2; cost = c; show("Adding"); }

    ~sgraph_edge() { show("Deleting"); }

    // Nodes numbered by their position in the list of nodes
    // Ordered node1 < node2
    int node1;
    int node2;
    int64_t cost;

    bool operator<(sgraph_edge &oedge) { return cost < oedge.cost; }

    void show(const char *prefix) {
	int upper = (cost >> 32);
	int lower = (cost & 0xFFFFFFF);
	printf("%s: Edge %d <--> %d.  Cost = %d/%d\n", prefix, node1, node2, upper, lower);
    }
};

// Data structure for adjacency list element
class sgraph_adj {

public:
    sgraph_adj(int n, sgraph_edge *e) { neighbor = n; edge = e; next = NULL; }

    int neighbor;
    sgraph_edge *edge;
    // Put in linked list ordered by ascending node ID
    sgraph_adj *next;

};    
    
static sgraph_adj *insert_adj(sgraph_adj *alist, sgraph_adj *ae) {
    if (alist == NULL || ae->neighbor < alist->neighbor) {
	ae->next = alist;
	return ae;
    }
    sgraph_adj *pos = alist;
    while (pos->next && ae->neighbor > pos->neighbor)
	pos = pos->next;
    ae->next = pos;
    pos->next = ae;
    return alist;
}

class sum_graph {

public:
    sum_graph(xor_constraint **xlist, int xcount, int variable_count, unsigned seed) {
	// Initial RNG.  Should make this class-specific
	srandom(seed);
	nodes = xlist;
	real_node_count = node_count = xcount;
	adj_lists = new sgraph_adj*[node_count];
	// Build inverse map from variables to nodes.  Use to keep track of
	// which nodes share common variables
	// Generate graph edges while building up reverse map
	std::set<int> *imap = new std::set<int>[variable_count+1];
	for (int x = xcount-1; x >= 0; x--) {
	    ilist variables = nodes[x]->get_variables();
	    for (int i = 0; i < ilist_length(variables); i++) {
		int v = variables[i];
		if (imap[v].count(x) == 0) {
		    for (auto rit = imap[v].rbegin(); rit != imap[v].rend(); rit++) {
			int ox = *rit;
			add_edge(x, ox);
		    }
		    imap[v].insert(x);
		}
	    }
	}
	delete[] imap;
    }

    ~sum_graph() {
	nodes = NULL;
	edges.clear();
	delete[] adj_lists;
    }

    xor_constraint *get_sum() {
	// Reduce the graph
	while (edges.size() > 0) {
	    sgraph_edge *e = *edges.begin();
	    edges.erase(e);
	    int x = e->node1;
	    int nx = e->node2;
	    xor_constraint *xc = xor_plus(nodes[x], nodes[nx]);
	    delete nodes[x];
	    delete nodes[nx];
	    nodes[x] = xc;
	    nodes[nx] = NULL;
	    real_node_count--;
	    contract_edge(e);
	    delete e;
	}	
	// Add up any remaining nodes (one per component of graph)
	xor_constraint *sum = NULL;
	for (int x = 0; x < node_count; x++) {
	    if (nodes[x] != NULL) {
		if (sum == NULL)
		    sum = nodes[x];
		else {
		    xor_constraint *nsum = xor_plus(sum, nodes[x]);
		    delete sum;
		    delete nodes[x];
		    sum = nsum;
		    nodes[x] = NULL;
		    real_node_count--;
		}
	    }
	}	
	return sum;
    }    

    void show(const char *prefix) {
	printf("%s: %d nodes, %d edges\n", prefix, real_node_count, (int) edges.size());
	for (int x = 0; x < node_count; x++) {
	    if (nodes[x] == NULL)
		continue;
	    printf("    Node %d.  Constraint ", x);
	    nodes[x]->show(stdout);
	    sgraph_adj *ax = adj_lists[x];
	    while (ax != NULL) {
		ax->edge->show("        ");
		ax = ax->next;
	    }
	}
    }


private:
    xor_constraint **nodes;
    
    int node_count;
    int real_node_count;

    // Adjacency lists.  One per node
    sgraph_adj **adj_lists;
    // Edges, ordered by increasing cost
    std::set<sgraph_edge *> edges;

    // This should be fixed to be local to class
    int new_lower() {
	//	return random();
	// Temporarily determinize
	return edges.size() + 1;
    }

    // Add new edge. Guarantee that x < ox.
    void add_edge(int x, int ox) {
	int64_t cost = xcost(nodes[x], nodes[ox], new_lower());
	sgraph_edge *e = new sgraph_edge(x, ox, cost);
	edges.insert(e);

	sgraph_adj *ax = new sgraph_adj(ox, e);
	// Ordered insertion
	adj_lists[x] = insert_adj(adj_lists[x], ax);
	sgraph_adj *aox = new sgraph_adj(x, e);
	adj_lists[ox] = insert_adj(adj_lists[ox], aox);
    }

    // This function gets called when edge de has been selected
    // and the constraint for node2 has been added to that of node1
    // Now must build merged adjacency list.
    void contract_edge(sgraph_edge *de) {
	int x = de->node1;
	int ox = de->node2;
	sgraph_adj *ax = adj_lists[x];
	adj_lists[x] = NULL;
	sgraph_adj *aox = adj_lists[ox];
	adj_lists[ox] = NULL;
	sgraph_adj **tail = &adj_lists[x];
	// Build new adjacency list for x;
	while (ax != NULL && aox != NULL) {
	    int nx = ax->neighbor;
	    if (nx == ox) {
		// Here's the edge were contracting
		sgraph_adj *save = ax;
		ax = ax->next;
		delete save;
		continue;
	    }
	    int nox = aox->neighbor;
	    if (nox == x) {
		// Here's the edge were contracting
		sgraph_adj *save = aox;
		aox = aox->next;
		delete save;
		continue;
	    }
	    if (nx == nox) {
		// The two nodes shared a common neighbor.
		if (xoverlap(nodes[x], nodes[nx])) {
		    // Reuse existing edge and adjacency element
		    sgraph_edge *e = ax->edge;
		    edges.erase(e);
		    e->show("Erasing");
		    e->cost = xcost(nodes[x], nodes[nx], new_lower());
		    // Reinsert using new cost
		    e->show("Reinserting");
		    edges.insert(e);
		    *tail = ax;
		    ax = ax->next;
		    tail = &((*tail)->next);
		    *tail = NULL;
		    sgraph_adj *save = aox;
		    aox = aox->next;
		    delete save;
		    continue;
		} else {
		    // This edge can be discarded
		    sgraph_edge *e = ax->edge;
		    e->show("Discarding");
		    edges.erase(e);
		    delete e;
		    sgraph_adj *save = ax;
		    ax = ax->next;
		    delete save;
		    save = aox;
		    aox = aox->next;
		    delete save;
		    continue;
		}
	    }
	    if (nx < nox)  {
		// Update edge for nx
		sgraph_edge *e = ax->edge;
		e->show("Erasing");
		edges.erase(e);
		e->show("Updating_1");
		e->cost = xcost(nodes[x], nodes[nx], new_lower());
		edges.insert(e);
		*tail = ax;
		ax = ax->next;
		tail = &((*tail)->next);
		*tail = NULL;
		continue;
	    } else {
		// Repurpose edge for nox;
		sgraph_edge *e = aox->edge;
		e->show("Erasing");
		edges.erase(e);
		e->cost = xcost(nodes[x], nodes[nox], new_lower());
		if (e->node1 == ox)
		    e->node1 = x;
		else
		    e->node2 = x;
		e->show("Repurposing_1");
		aox->neighbor = x;
		edges.insert(e);
		*tail = aox;
		aox = aox->next;
		tail = &((*tail)->next);
		*tail = NULL;
		continue;
	    }
	}
	while (ax != NULL) {
	    // Update edge for nx
	    sgraph_edge *e = ax->edge;
	    e->show("Erasing");
	    edges.erase(e);
	    e->show("Updating_2");
	    int nx = ax->neighbor;
	    e->cost = xcost(nodes[x], nodes[nx], new_lower());
	    edges.insert(e);
	    *tail = ax;
	    ax = ax->next;
	    tail = &((*tail)->next);
	    *tail = NULL;
	}
	while (aox != NULL) {
	    // Repurpose edge for nox;
	    sgraph_edge *e = aox->edge;
	    e->show("Erasing");
	    edges.erase(e);
	    int nox = aox->neighbor;
	    e->cost = xcost(nodes[x], nodes[nox], new_lower());
	    if (e->node1 == ox)
		e->node1 = x;
	    else
		e->node2 = x;
	    e->show("Repurposing_2");
	    edges.insert(e);
	    *tail = aox;
	    aox = aox->next;
	    tail = &((*tail)->next);
	    *tail = NULL;
	}
	de->show("Contracted");
	show("Following contraction");
    }
};



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

// Chosen method for computing sum
xor_constraint *trustbdd::xor_sum_list(xor_constraint **xlist, int len, int maxvar) {
    if (len <= 4)
	return xor_sum_list_linear(xlist, len);
    sum_graph g(xlist, len, maxvar, 1);
    return g.get_sum();
}

xor_set::~xor_set() {
    xlist.clear();
}

void xor_set::add(xor_constraint &con) {
    pseudo_init();
    // Make local copy of the constraint
    xor_constraint *ncon = new xor_constraint(con);
    ilist vars = ncon->get_variables();
    int n = ilist_length(vars);
    if (n > 0 && vars[n-1] > maxvar)
	maxvar = vars[n-1];

    xlist.push_back(ncon);
}

xor_constraint *xor_set::sum() {
    xor_constraint *xsum = xor_sum_list(xlist.data(), xlist.size(), maxvar);
    xlist.clear();
    return xsum;
}
