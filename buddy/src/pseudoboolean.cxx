#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

#include "pseudoboolean.h"
#include "prover.h"

using namespace trustbdd;

#define BUFLEN 2048
// For formatting information
static char ibuf[BUFLEN];

// Standard seed value
#define DEFAULT_SEED 123456

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
static void pseudo_done_fun();

static bool initialized = false;

static void pseudo_init() {
    if (!initialized) {
	tbdd_add_info_fun(pseudo_info_fun);
	tbdd_add_done_fun(pseudo_done_fun);
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


static void pseudo_done_fun() {
    for (auto p : xor_map) {
	xor_constraint *xcp = p.second;
	delete xcp;
    }
    xor_map.clear();
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
    }
    pseudo_xor_unique ++;
    pseudo_total_length += ilist_length(variables);
}

///////////////////////////////////////////////////////////////////
// Methods & functions for xor constraints
///////////////////////////////////////////////////////////////////


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




///////////////////////////////////////////////////////////////////
// To aid debugging
// Managing pairs of ints packed into 64-bit word
// Use these both as edge identifier
// and as cost+unique value
//
// When combining cost functions with unique identifiers
// Have upper 32 bits represent cost
// and lower 32 bits represent unique value, 
//   assigned either sequentially or via a pseudo-random sequence
///////////////////////////////////////////////////////////////////

/*
  32-bit words packed into pairs
 */
static int64_t pack(int upper, int lower) {
    return ((int64_t) upper << 32) | lower;
}


static int64_t ordered_pack(int x1, int x2) {
    return x1 < x2 ? pack(x1, x2) : pack(x2, x1);
}

static int upper(int64_t pair) {
    return (int) (pair>>32);
}

static int lower(int64_t pair) {
    return (int) (pair & 0xFFFFFFFF);
}


/// Various ways to compute the sum of a list of xor constraints

///////////////////////////////////////////////////////////////////
// Graph summation:
// Define cost of an addition to be the number of nonzero coefficients in the result.
// Heuristic: On each step, add a pair of arguments that will be of minimum cost.
// The sum graph provides a mechanism for implementing that heuristic.
// There is a node for each remaining argument, and an undirected edge (i,i')
// if nodes i & j share at least one nonzero coeffienct.
//
// The edges are stored as a map from cost to edge object.
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

/* 
   Determine the cost of adding two xor constraints
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
    return pack(upper, lower);
}


// Pseudo-random number generator based on the Lehmer MINSTD RNG
// Use to both randomize selection and to provide a unique value
// for each cost.

class sequencer {

public:
    sequencer(void) { set_seed(DEFAULT_SEED); }

    void set_seed(uint64_t s) { seed = s == 0 ? 1 : s; next() ; next(); }

    uint32_t next() { seed = (seed * mval) % groupsize; return seed; }

private:
    uint64_t seed;
    const uint64_t mval = 48271;
    const uint64_t groupsize = 2147483647LL;

};

// Data structure for list edge
class sgraph_edge {
public:
    sgraph_edge(int n1, int n2, int64_t c) {
	node1 = n1, node2 = n2; cost = c;  
	if (verbosity_level >= 3 ) show("Adding");
    }

    ~sgraph_edge() { if (verbosity_level >= 3) show("Deleting"); }

    // Nodes numbered by their position in the list of nodes
    // Ordered node1 < node2
    int node1;
    int node2;
    int64_t cost;

    void show(const char *prefix) {
	printf("%s: Edge %d <--> %d.  Cost = %d/%d\n", prefix, node1, node2, upper(cost), lower(cost));
    }
};


class sum_graph {

public:
    sum_graph(xor_constraint **xlist, int xcount, int variable_count, unsigned seed) {
	seq.set_seed(seed);
	nodes = xlist;
	real_node_count = node_count = xcount;
	neighbors = new std::set<int> [node_count];
	int real_variable_count = 0;
	// Build inverse map from variables to nodes.  Use to keep track of
	// which nodes share common variables
	// Generate graph edges at the same time
	std::set<int> *imap = new std::set<int>[variable_count];
	for (int n1 = 0; n1 < node_count; n1++) {
	    ilist variables = nodes[n1]->get_variables();
	    for (int i = 0; i < ilist_length(variables); i++) {
		int v = variables[i];
		for (int n2 : imap[v-1]) {
		    if (edge_map.count(ordered_pack(n1, n2)) == 0)
			add_edge(n1, n2);
		}
		if (imap[v-1].size() == 0)
		    real_variable_count++;
		imap[v-1].insert(n1);
	    }
	}
	delete[] imap;
	if (verbosity_level >= 1) {
	    printf("Summing over graph with %d nodes, %d edges, %d variables\n", xcount, (int) edge_map.size(), real_variable_count);
	}
	if (verbosity_level >= 2)
	    show("Initial");
    }

    ~sum_graph() {
	nodes = NULL;
	edges.clear();
	edge_map.clear();
	delete [] neighbors;
    }

    xor_constraint *get_sum() {
	// Reduce the graph
	while (edges.size() > 0) {
	    sgraph_edge *e = edges.begin()->second;
	    edges.erase(e->cost);
	    int n1 = e->node1;
	    int n2 = e->node2;
	    xor_constraint *xc = xor_plus(nodes[n1], nodes[n2]);
	    delete nodes[n1];
	    delete nodes[n2];
	    nodes[n2] = NULL;
	    real_node_count--;
	    if (xc->is_degenerate()) {
		nodes[n1] = NULL;
		real_node_count--;
		if (verbosity_level >= 2)
		    e->show("Deleting min");
		if (verbosity_level >= 3)
		    show("After deletion");

	    } else {
		nodes[n1] = xc;
		if (verbosity_level >= 2)
		    e->show("Contracting");
		contract_edge(e);
		if (verbosity_level >= 3)
		    show("After contraction");
	    }
	    delete e;
	}
	xor_constraint *sum = new xor_constraint();
	// Add up any remaining nodes (one per component of graph)
	for (int n = 0; n < node_count; n++) {
	    if (nodes[n] != NULL) {
		xor_constraint *nsum = xor_plus(sum, nodes[n]);
		delete sum;
		delete nodes[n];
		sum = nsum;
		nodes[n] = NULL;
		real_node_count--;
	    }
	}	
	return sum;
    }


    void show(const char *prefix) {
	printf("%s: %d nodes, %d edges\n", prefix, real_node_count, (int) edges.size());
	for (int n1 = 0; n1 < node_count; n1++) {
	    if (nodes[n1] == NULL)
		continue;
	    printf("    Node %d.  Constraint ", n1);
	    nodes[n1]->show(stdout);
	    printf("\n");
	    for (int n2 : neighbors[n1]) {
		sgraph_edge *e = edge_map[ordered_pack(n1, n2)];
		e->show("        ");
	    }
	}
    }


private:
    // Set of constraints, which serve as graph nodes
    // These get deleted as summation proceeds
    xor_constraint **nodes;
    
    int node_count;
    int real_node_count;

    // Edges, indexed by cost
    std::map<int64_t,sgraph_edge *> edges;

    // For each node, the set of adjacent nodes
    std::set<int> *neighbors;
    
    // For each edge (n1, n2), a mapping from the pair to the edge
    std::unordered_map<int64_t,sgraph_edge*> edge_map;

    // For assigning unique values to cost
    sequencer seq;

    // Assign new unique ID
    int new_lower() {
	return (int) seq.next();
    }

    // Add new edge.
    void add_edge(int n1, int n2) {
	if (n1 > n2) 
	    { int t = n1; n1 = n2; n2 = t; }  // Reorder nodes
	int64_t cost = xcost(nodes[n1], nodes[n2], new_lower());
	sgraph_edge *e = new sgraph_edge(n1, n2, cost);
	edges[cost] = e;
	int64_t pair = pack(n1, n2);
	edge_map[pair] = e;
	neighbors[n1].insert(n2);
	neighbors[n2].insert(n1);
    }

    void remove_edge(sgraph_edge *e) {
	int n1 = e->node1;
	int n2 = e->node2;
	edges.erase(e->cost);
	int64_t pair = pack(n1, n2);
	edge_map.erase(pair);
	neighbors[n1].erase(n2);
	neighbors[n2].erase(n1);
    }

    // This function gets called when edge de has been selected
    // and the constraint for node2 has been added to that of node1
    // Now must build merged adjacency list.
    void contract_edge(sgraph_edge *de) {
	std::set<int> new_neighbors;
	std::vector<sgraph_edge *> dedges;
	int n1 = de->node1;
	int n2 = de->node2;
	// Determine new neighbors of n1
	for (int nn1 : neighbors[n1]) {
	    if (nn1 == n2)
		// This edge is being contracted
		continue;
	    sgraph_edge *e = edge_map[ordered_pack(n1, nn1)];
	    dedges.push_back(e);
	    if (xoverlap(nodes[n1], nodes[nn1]))
		// This looks interesting
		new_neighbors.insert(nn1);
	}
	for (int nn2 : neighbors[n2]) {
	    if (nn2 == n1)
		// This edge is being contracted
		continue;
	    sgraph_edge *e = edge_map[ordered_pack(n2, nn2)];
	    dedges.push_back(e);
	    if (new_neighbors.count(nn2) > 0)
		// Already have this one
		continue;
	    if (xoverlap(nodes[n1], nodes[nn2]))
		// This looks interesting
		new_neighbors.insert(nn2);
	}
	for (sgraph_edge *e : dedges) {
	    remove_edge(e);
	    delete e;
	}
	// Add new edges for n1
	neighbors[n1].clear();
	neighbors[n2].clear();
	for (int nn1 : new_neighbors) {
	    add_edge(n1, nn1);
	}

    }
};



// Linear evaluation.  Good for small sets
static xor_constraint *xor_sum_list_linear(xor_constraint **xlist, int len) {
    if (len == 0)
	return new xor_constraint();
    xor_constraint *sum = xlist[0];
    xlist[0] = NULL;
    for (int i = 1; i < len; i++) {
	xor_constraint *a = xlist[i];
	xlist[i] = NULL;
	xor_constraint *nsum = xor_plus(sum, a);
	delete a;
	delete sum;
	sum = nsum;
    }
    return sum;
}

// Breadth-first computation.  Effectively forms binary tree of additions
static xor_constraint *xor_sum_list_bf(xor_constraint **xlist, int len) {
    if (len == 0)
	return new xor_constraint();
    // Use breadth-first addition
    xor_constraint **xbuf = new xor_constraint*[2*len];
    for (int i = 0; i < len; i++) {
	xbuf[i] = xlist[i];
	xlist[i] = NULL;
    }
    // Left and right-most positions that have been used in the buffer 
    int left = 0;
    int right = len-1;
    while (left < right) {
	xor_constraint *arg1 = xbuf[left++];
	xor_constraint *arg2 = xbuf[left++];
	xbuf[++right] = xor_plus(arg1, arg2);
	delete arg1;
	delete arg2;
    }
    xor_constraint *sum = xbuf[right];
    delete[] xbuf;
    return sum;
}

// Chosen method for computing sum
xor_constraint *trustbdd::xor_sum_list(xor_constraint **xlist, int len, int maxvar) {
    if (len <= 4)
 	return xor_sum_list_linear(xlist, len);
    //    return xor_sum_list_bf(xlist, len);
    sum_graph g(xlist, len, maxvar, DEFAULT_SEED);
    return g.get_sum();
}

///////////////////////////////////////////////////////////////////
// Support for Gauss-Jordan elimination
///////////////////////////////////////////////////////////////////

// Pivots
class pivot {
public:
    pivot(int eid, int var, int64_t c) {
	equation_id = eid;
	variable = var;
	cost = c;
    }

    ~pivot() { if (verbosity_level >= 3) show("Deleting"); }

    int equation_id;
    int variable;
    int64_t cost;

    void show(const char *prefix) {
	printf("%s: Pivot Eid = %d.  Var = %d.  Cost = %d/%d\n", prefix, equation_id, variable, upper(cost), lower(cost));
    }
};

class gauss {
public:

    gauss (xor_constraint **xlist, int xcount, ilist exvars, int vcount, unsigned seed) {
	seq.set_seed(seed);
	equations = xlist;
	equation_count = remaining_equation_count = xcount;
	variable_count = vcount;
	int real_variable_count = 0;
	int real_exvar_count = 0;
	for (int i = 0; i < ilist_length(exvars); i++)
	    external_variables.insert(exvars[i]);
	// Build inverse map
	imap = new std::set<int>[variable_count];
	for (int eid = 0; eid < equation_count; eid++) {
	    ilist vars = equations[eid]->get_variables();
	    for (int i = 0; i < ilist_length(vars); i++) {
		int v = vars[i];
		imap[v-1].insert(eid);
	    }
	}
	pivot_list = new pivot*[variable_count];
	for (int v = 1; v <= variable_count; v++) {
	    pivot *piv = choose_pivot(v);
	    pivot_list[v-1] = piv;
	    if (piv) {
		int64_t pcost = piv->cost;
		real_variable_count++;
		if (external_variables.count(v) > 0)
		    real_exvar_count ++;
		pivot_selector[pcost] = piv;
	    }
	}
	if (verbosity_level >= 1) {
	    printf("Performing Gauss-Jordan elimination with %d equations, %d  variables (%d external)\n",
		   xcount, real_variable_count, real_exvar_count);
	}
    }

    ~gauss() {
	for (int eid = 0; eid < equation_count; eid++) {
	    xor_constraint *eq = equations[eid];
	    if (eq) delete eq;
	}
	for (xor_constraint *eq : saved_equations)
	    if (eq) delete eq;
	for (pivot *piv : saved_pivots)
	    if (piv) delete piv;
	for (int v = 1; v <= variable_count; v++) {
	    pivot *piv = pivot_list[v-1];
	    if (piv) delete piv;
	}
	delete [] imap;
    }


    void show(const char *prefix) {
	printf("%s status\n", prefix);
	printf("  %d remaining equations, %d variables\n", remaining_equation_count, (int) pivot_selector.size());
	if (remaining_equation_count > 0) {
	    for (int eid= 0; eid < equation_count; eid++) {
		if (equations[eid] == NULL)
		    continue;
		printf("    Equation #%d: ", eid);
		equations[eid]->show(stdout);
		printf("\n");
	    }
	}
	if (saved_equations.size() > 0) {
	    printf("  %d saved equations\n", (int) saved_equations.size());
	    for (int eid = 0; eid < saved_equations.size(); eid++) {
		printf("    Pivot variable %d.  Equation: ", saved_pivots[eid]->variable);
		saved_equations[eid]->show(stdout);
		printf("\n");
	    }
	}
    }

    void gauss_jordan(xor_set &nset) {
	// Gaussian elimination
	bool infeasible = false;
	if (verbosity_level >= 2) {
	    show("Initial");
	}
	int step_count = 0;
	while (!infeasible && remaining_equation_count > 0) {
	    infeasible = gauss_step();
	    step_count++;
	    if (verbosity_level >= 3) {
		char mbuf[30];
		sprintf(mbuf, "Step #%d", step_count);
		show(mbuf);
	    }
	}
	// Fix up the final result
	nset.clear();
	if (infeasible) {
	    xor_constraint *seq = saved_equations[0];
	    nset.add(*seq);
	    if (verbosity_level >= 1) {
		printf("Gauss-Jordan completed.  %d steps.  System infeasible\n", step_count);
	    }
	} else if (saved_equations.size() > 0) {
	    // Not degenerate
	    jordanize();
	    for (xor_constraint *eq : saved_equations)
		nset.add(*eq);
	    if (verbosity_level >= 1) {
		printf("Gauss-Jordan completed.  %d steps.  %d final equations\n", step_count, (int) saved_equations.size());
	    }
	}
    }

private:
    // The set of external variables
    std::unordered_set<int> external_variables;
    // The set of equations.  Get set to NULL as eliminated
    xor_constraint **equations;
    // The number of original equations
    int equation_count;
    // The number of active equations
    int remaining_equation_count;
    // Equations saved after elimination
    std::vector<xor_constraint*> saved_equations;
    // Ordered list of pivots having external variables that were used
    std::vector<pivot *>saved_pivots;
    // Number of variables.  Numbered from 1 .. variable_count
    int variable_count;
    // Mapping for (decremented) variable to equation IDs
    std::set<int> *imap;
    // Chosen pivot for each variable
    pivot **pivot_list;
    // Mapping from cost function to pivot
    std::map<int64_t,pivot*> pivot_selector;
    // Pseudo RNG to both randomize pivot selection and to generate unique IDs for pivots
    sequencer seq;

    // Assign new unique ID
    int new_lower() {
	return (int) seq.next();
    }

    // Choose best pivot for specified variable
    pivot *choose_pivot(int var) {
	int64_t best_cost = INT64_MAX;
	int best_eid = -1;
	int cols = imap[var-1].size();
	for (int eid : imap[var-1]) {
	    ilist row = equations[eid]->get_variables();
	    int c = (cols-1)*(ilist_length(row)-1);
	    if (external_variables.count(var) > 0)
		// Penalty for external variable.
		// Will have cost > any internal variable
		c += equation_count * variable_count;
	    int64_t cost = pack(c, new_lower());
	    if (cost < best_cost) {
		best_cost = cost;
		best_eid = eid;
	    }
	}
	if (best_eid < 0)
	    return NULL;
	pivot *piv = new pivot(best_eid, var, best_cost);
	return piv;
    }

    // Perform one step of Gaussian elimination
    // Return true if infeasible equation encountered
    bool gauss_step() {
	std::set<int> touched;  // Track variables that are involved
	pivot *piv = pivot_selector.begin()->second;
	pivot_selector.erase(piv->cost);
	if (verbosity_level >= 2) {
	    piv->show("Using");
	}
	int peid = piv->equation_id;
	int pvar = piv->variable;
	pivot_list[pvar-1] = NULL;
	xor_constraint *peq = equations[peid];
	equations[peid] = NULL;
	remaining_equation_count--;
	ilist pvars = peq->get_variables();
	// Remove any references from inverse map
	for (int i = 0; i < ilist_length(pvars); i++) {
	    int v = pvars[i];
	    imap[v-1].erase(peid);
	    if (v != pvar)
		touched.insert(v);
	}
	// Perform eliminination operation on other equations
	for (int eid : imap[pvar-1]) {
	    xor_constraint *eq = equations[eid];
	    ilist evars = eq->get_variables();
	    // Remove any references from inverse map
	    for (int i = 0; i < ilist_length(evars); i++) {
		int v = evars[i];
		if (v != pvar) {
		    imap[v-1].erase(eid);
		    touched.insert(v);
		}
	    }
	    // Add the equations
	    xor_constraint *neq = xor_plus(peq, eq);
	    delete eq;
	    if (neq->is_infeasible()) {
		if (verbosity_level >= 2)
		    printf("Infeasible equation #%d + #%d encountered\n", peid, eid);
		equations[eid] = NULL;
		// Cancel any saved equations or pivots
		for (xor_constraint *seq : saved_equations)
		    delete seq;
		for (pivot *spiv : saved_pivots)
		    delete spiv;
		saved_equations.clear();
		saved_pivots.clear();
		saved_equations.push_back(neq);
		saved_pivots.push_back(piv);
		return true;
	    } else if (neq->is_degenerate()) {
		equations[eid] = NULL;
		remaining_equation_count--;
		delete neq;
	    } else {
		equations[eid] = neq;
		// Update inverse map
		ilist nvars = neq->get_variables();
		for (int i = 0; i < ilist_length(nvars); i++) {
		    int v = nvars[i];
		    imap[v-1].insert(eid);
		}
	    }
	}
	imap[pvar-1].clear();
	if (external_variables.count(pvar) > 0) {
	    saved_equations.push_back(peq);
	    saved_pivots.push_back(piv);
	} else {
	    delete peq;
	    delete piv;
	}
	// Update pivots for variables that were touched
	for (int tv : touched) {
	    pivot *opiv = pivot_list[tv-1];
	    pivot_selector.erase(opiv->cost);
	    delete opiv;
	    pivot *npiv = choose_pivot(tv);
	    pivot_list[tv-1] = npiv;
	    if (npiv)
		pivot_selector[npiv->cost] = npiv;
	}
	return false;
    }

    // Convert saved equations into Jordan form
    void jordanize() {
	for (int peid = saved_equations.size()-1; peid > 0; peid--) {
	    xor_constraint *peq = saved_equations[peid];
	    int pvar = saved_pivots[peid]->variable;
	    delete saved_pivots[peid];
	    saved_pivots[peid] = NULL;
	    for (int eid = pvar-1; eid >= 0; eid--) {
		xor_constraint *eq = saved_equations[eid];
		ilist vars = eq->get_variables();
		if (ilist_is_member(vars, pvar)) {
		    xor_constraint *neq = xor_plus(eq, peq);
		    delete eq;
		    saved_equations[eid] = neq;
		}
	    }
	}
	if (verbosity_level >= 2)
	    show("After Jordanizing");
    }
};


///////////////////////////////////////////////////////////////////
// xor_set operations
///////////////////////////////////////////////////////////////////


xor_set::~xor_set() {
    clear();
}

void xor_set::add(xor_constraint &con) {
    pseudo_init();
    if (con.is_degenerate())
	// Don't want this degenerate constraints
	return;
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
    clear();
    return xsum;
}

bool xor_set::is_degenerate() {
    return xlist.size() == 0;
}

bool xor_set::is_infeasible() {
    if (xlist.size() != 1)
	return false;
    return xlist[0]->is_infeasible();
}

void xor_set::clear() {
    for (xor_constraint *xc : xlist) {
	if (xc == NULL)
	    delete xc;
    }
    xlist.clear();
    maxvar = 0;
}

void xor_set::gauss_jordan(ilist external_variables, xor_set &nset) {
    gauss g(xlist.data(), xlist.size(), external_variables, maxvar, 1);
    g.gauss_jordan(nset);
}
