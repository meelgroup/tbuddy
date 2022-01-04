#include <stdio.h>
#include <stdlib.h>
#include "prover.h"
#include "kernel.h"


/* Global variables exported by prover */
proof_type_t proof_type = PROOF_FRAT;
int verbosity_level = 1;
int *variable_counter = NULL;
int *clause_id_counter = NULL;
int total_clause_count = 0;
int input_clause_count = 0;
int input_variable_count = 0;
int max_live_clause_count = 0;

/* Global variables used by prover */
static FILE *proof_file = NULL;
/* 
   For LRAT, only need to keep dictionary of input clauses.
   For DRAT & FRAT, need dictionary of all clauses in order to delete them.
*/

static bool do_binary = false;
static ilist *all_clauses = NULL;
static int alloc_clause_count = 0;
static int live_clause_count = 0;
static ilist deferred_deletion_list = NULL;
static bool empty_clause_detected = false;

// Buffer used when generating binary files
static unsigned char *dest_buf = NULL;
static size_t dest_buf_len = 0;

// Parameters
// Cutoff betweeen large and small allocations (in terms of clauses)
#define BUDDY_THRESHOLD 1000
//#define BUDDY_THRESHOLD 10
#define BUDDY_NODES_LARGE (2*1000*1000)
//#define BUDDY_NODES_LARGE (1000)
#define BUDDY_NODES_SMALL (2* 100*1000)
#define BUDDY_CACHE_RATIO 8
#define BUDDY_INCREASE_RATIO 20

// How many clauses should allocated for clauses
#define INITIAL_CLAUSE_COUNT 1000


/* Useful static functions */


/* API functions */
int prover_init(FILE *pfile, int *var_counter, int *cls_counter, ilist *input_clauses, proof_type_t ptype, bool binary) {
    empty_clause_detected = false;
    proof_type = ptype;
    do_binary = binary;
    if (do_binary) {
	dest_buf_len = 100;
	dest_buf = malloc(dest_buf_len);
	if (!dest_buf)
	    return bdd_error(BDD_MEMORY);
    }
    proof_file = pfile;

    variable_counter = var_counter;
    input_variable_count = *variable_counter;

    clause_id_counter = cls_counter;
    if (clause_id_counter) {
	input_clause_count = total_clause_count = *clause_id_counter;
	live_clause_count = max_live_clause_count = total_clause_count;
    }
    if (proof_type != PROOF_NONE) {
    	alloc_clause_count = input_clause_count + INITIAL_CLAUSE_COUNT;
	all_clauses = calloc(alloc_clause_count, sizeof(ilist));
	if (all_clauses == NULL)
	    return bdd_error(BDD_MEMORY);
	print_proof_comment(1, "Proof of CNF file with %d variables and %d clauses", input_variable_count, input_clause_count);
	int cid;
	for (cid = 0; cid < alloc_clause_count; cid++)
		all_clauses[cid] = TAUTOLOGY_CLAUSE;
	if (input_clauses) {
	    for (cid = 0; cid < input_clause_count; cid++) {
		all_clauses[cid] = ilist_copy(input_clauses[cid]);
		if (print_ok(2)) {
		    fprintf(proof_file, "c Input Clause #%d: ", cid+1);
		    ilist_print(all_clauses[cid], proof_file, " ");
		    fprintf(proof_file, " 0\n");
		}
	    }
	}
    }

    deferred_deletion_list = ilist_new(100);


    int bnodes = input_clause_count < BUDDY_THRESHOLD ? BUDDY_NODES_SMALL : BUDDY_NODES_LARGE;
    int bcache = bnodes/BUDDY_CACHE_RATIO;
    int bincrease = bnodes/BUDDY_INCREASE_RATIO;
    int rval = bdd_init(bnodes, bcache);

    bdd_setcacheratio(BUDDY_CACHE_RATIO);
    bdd_setmaxincrease(bincrease);
    bdd_setvarnum(input_variable_count+1);
    bdd_disable_reorder();
    return rval;
}

void prover_done() {
    free(dest_buf);
    if (proof_type == PROOF_FRAT)
	/* Do final garbage collection to delete remaining clauses */
	bdd_gbc();
    //    if (deferred_deletion_list)
    //	ilist_free(deferred_deletion_list);
}


/* Print clause */
void print_clause(FILE *out, ilist clause) {
    int i;
    if (clause == TAUTOLOGY_CLAUSE) {
	fprintf(out, "TAUT");
	return;
    }
    char *bstring = "[";
    for (i = 0; i < ilist_length(clause); i++) {
	int lit = clause[i];
	if (lit == TAUTOLOGY) 
	    fprintf(out, "%sTRUE", bstring);
	else if (lit == -TAUTOLOGY)
	    fprintf(out, "%sFALSE", bstring);
	else
	    fprintf(out, "%s%d", bstring, lit);
	bstring = ", ";
    }
    fprintf(out, "]");
}

/* Helper function for clause cleaning.  Sort literals to put variables in descending order */
int literal_compare(const void *l1p, const void *l2p) {
     int l1 = *(int *) l1p;
     int l2 = *(int *) l2p;
     unsigned u1 = l1 < 0 ? -l1 : l1;
     unsigned u2 = l2 < 0 ? -l2 : l2;
     if (u2 > u1)
	 return 1;
     if (u2 < u1)
	 return -1;
     return 0;
}

ilist clean_clause(ilist clause) {
    if (clause == TAUTOLOGY_CLAUSE)
	return clause;
    int len = ilist_length(clause);
    if (len == 0)
	return clause;
    /* Sort the literals */
    qsort((void *) clause, ilist_length(clause), sizeof(int), literal_compare);
    int geti = 0;
    int puti = 0;
    int plit = 0;
    while (geti < len) {
	int lit = clause[geti++];
	if (lit == TAUTOLOGY)
	    return TAUTOLOGY_CLAUSE;
	if (lit == -TAUTOLOGY)
	    continue;
	if (lit == plit)
	    continue;
	if (lit == -plit)
	    return TAUTOLOGY_CLAUSE;
	clause[puti++] = lit;
	plit = lit;
    }
    clause = ilist_resize(clause, puti);
    return clause;
}

#if DO_TRACE
/* Look for specified ID  among clause or hints */
static void trace_list(ilist list, int step_id, char *msg) {
    int i;
    if (list == TAUTOLOGY_CLAUSE)
	return;
    int len = ilist_length(list);
    for (i = 0; i < len; i++) {
	int id = list[i];
	if (id == TRACE_CLAUSE) {
	    printf("TRACE.  Found %d on step #%d: %s [", TRACE_CLAUSE, step_id, msg);
	    ilist_print(list, stdout, " ");
	    printf("]\n");
	}
    }
}
#endif /* DO_TRACE */


static ilist clean_hints(ilist hints) {
    int len = ilist_length(hints);
    int geti = 0;
    int puti = 0;
    while (geti < len) {
	int lit = hints[geti++];
	if (lit != TAUTOLOGY)
	    hints[puti++] = lit;
    }
    hints = ilist_resize(hints, puti);
    return hints;
}


/*
  Compress integer list into byte sequence.
  Unsigned char array passed in to fill.
  Should be 5x longer than number of int's
 */
static int check_buffer(int icount) {
    size_t len = 5*icount;
    if (len > dest_buf_len) {
	dest_buf_len = len;
	dest_buf = realloc(dest_buf, len);
	if (dest_buf == NULL) 
	    return bdd_error(BDD_MEMORY);
    }
    return 0;
}

/* Convert integer into byte sequence.  Return number of bytes */
static int int_byte_pack(int x, unsigned char *dest) {
    unsigned char *d = dest;
    unsigned u = x < 0 ? 2*(-x)+1 : 2*x;
    while (u >= 128) {
	unsigned char b = u & 0x7F;
	u >>= 7;
	*d++ =  b+128;
    }
    *d++ = u;
    return d - dest;
}

/* Convert integer list into byte sequence.  Return number of bytes */
static int ilist_byte_pack(ilist src_list, unsigned char *dest) {
    int i;
    unsigned char *d = dest;
    for (i = 0; i < ilist_length(src_list); i++) {
	d += int_byte_pack(src_list[i], d);
    }
    return d - dest;
}

/* Return clause ID */
/* For DRAT proof, hints can be NULL */
int generate_clause(ilist literals, ilist hints) {
    if (proof_type == PROOF_NONE)
	return TAUTOLOGY;
    ilist clause = clean_clause(literals);
    int cid = ++(*clause_id_counter);
    int rval = 0;
    hints = clean_hints(hints);
    unsigned char *d = dest_buf;
    if (do_binary) {
	check_buffer(ilist_length(literals) + ilist_length(hints) + 4);
	d = dest_buf;
    }

#if DO_TRACE
    trace_list(clause, cid, "Generated clause");
    trace_list(hints, cid, "Supplied hints");
#endif

    if (clause == TAUTOLOGY_CLAUSE)
	return TAUTOLOGY;
    if (!empty_clause_detected) {
	if (do_binary)
	    *d++ = 'a';
	else if (proof_type == PROOF_FRAT)
	    fprintf(proof_file, "a ");
	if (proof_type == PROOF_LRAT || proof_type == PROOF_FRAT) {
	    if (do_binary) {
		d += int_byte_pack(cid, d);
	    } else {
		rval = fprintf(proof_file, "%d ", cid);
		if (rval < 0)
		    bdd_error(BDD_FILE);
	    }
	}
	if (do_binary) {
	    d += ilist_byte_pack(clause, d);
	} else
	    ilist_print(clause, proof_file, " ");

	if (proof_type == PROOF_LRAT) {
	    if (do_binary) {
		d += int_byte_pack(0, d);
		d += ilist_byte_pack(hints, d);
	    } else {
		rval = fprintf(proof_file, " 0 ");
		if (rval < 0) 
		    bdd_error(BDD_FILE);
		rval = ilist_print(hints, proof_file, " ");
		if (rval < 0) 
		    bdd_error(BDD_FILE);
	    }
	}
	if (proof_type == PROOF_FRAT) {
	    if (do_binary) {
		d += int_byte_pack(0, d);
		*d++ = 'l';
		d += ilist_byte_pack(hints, d);
	    } else {
		rval = fprintf(proof_file, " 0 l ");
		if (rval < 0) 
		    bdd_error(BDD_FILE);
		rval = ilist_print(hints, proof_file, " ");
		if (rval < 0) 
		    bdd_error(BDD_FILE);
	    }
	}
	if (do_binary) {
	    d += int_byte_pack(0, d);
	    rval = fwrite(dest_buf, 1, d - dest_buf, proof_file);
	    if (rval < 0)
		bdd_error(BDD_FILE);
	} else {
	    rval = fprintf(proof_file, " 0\n");
	    if (rval < 0) 
		bdd_error(BDD_FILE);
	}
    }
    total_clause_count++;
    live_clause_count++;
    max_live_clause_count = MAX(max_live_clause_count, live_clause_count);
    
    if (proof_type == PROOF_DRAT || proof_type == PROOF_FRAT) {
	/* Must store copy of clause */
	if (cid >= alloc_clause_count) {
	    /* must expand */
	    int new_alloc_clause_count = alloc_clause_count * 2;
	    int i;
	    all_clauses = realloc(all_clauses, new_alloc_clause_count * sizeof(ilist));
	    if (all_clauses == NULL)
		bdd_error(BDD_MEMORY);
	    for (i = alloc_clause_count; i < new_alloc_clause_count; i++) 
		all_clauses[i] = TAUTOLOGY_CLAUSE;
	    alloc_clause_count = new_alloc_clause_count;
	}
	all_clauses[cid-1] = ilist_copy(clause);
    }
    empty_clause_detected = ilist_length(clause) == 0;
    return cid;
}

/* For FRAT, have special clauses */
extern void insert_frat_clause(FILE *pfile, char cmd, int clause_id, ilist literals, bool binary) {
    ilist clause = clean_clause(literals);
    int rval = 0;
    unsigned char *d = dest_buf;

    if (binary) {
	check_buffer(ilist_length(clause) + 3);
	d = dest_buf;
	d += int_byte_pack(cmd, d);
	d += int_byte_pack(clause_id, d);
	d += ilist_byte_pack(clause, d);
	d += int_byte_pack(0, d);
	rval = fwrite(dest_buf, 1, d - dest_buf, pfile);
	if (rval < 0)
	    bdd_error(BDD_FILE);
    } else {
	rval = fprintf(pfile, "%c %d ", cmd, clause_id);
	if (rval < 0)
	    bdd_error(BDD_FILE);
	rval = ilist_print(clause, pfile, " ");
	if (rval < 0) 
	    bdd_error(BDD_FILE);
	rval = fprintf(pfile, " 0\n");
	if (rval < 0) 
	    bdd_error(BDD_FILE);
    }
}

void delete_clauses(ilist clause_ids) {
    int rval;
    unsigned char *d = dest_buf;
    if (empty_clause_detected && proof_type != PROOF_FRAT)
	return;
    clause_ids = clean_hints(clause_ids);

#if DO_TRACE
    trace_list(clause_ids, *clause_id_counter, "Deleted clauses");
#endif

    if (proof_type == PROOF_LRAT) {
	if (do_binary) {
	    check_buffer(ilist_length(clause_ids) + 3);
	    d = dest_buf;
	    *d++ = 'd';
	    //	    d += int_byte_pack(*clause_counter, d);
	    d += ilist_byte_pack(clause_ids, d);
	    d += int_byte_pack(0, d);
	    rval = fwrite(dest_buf, 1, d - dest_buf, proof_file);
	    if (rval < 0)
		bdd_error(BDD_FILE);
	} else {
	    rval = fprintf(proof_file, "%d d ", *clause_id_counter);
	    if (rval < 0)
		bdd_error(BDD_FILE);
	    ilist_print(clause_ids, proof_file, " ");
	    rval = fprintf(proof_file, " 0\n");
	    if (rval < 0) 
		bdd_error(BDD_FILE);
	}
    } else {
	// DRAT or FRAT
	int i;
	for (i = 0; i < ilist_length(clause_ids); i++) {
	    int cid = clause_ids[i];
	    ilist clause = all_clauses[cid-1];
	    if (clause == TAUTOLOGY_CLAUSE)
		continue;
	    if (ilist_length(clause) <= 1 && proof_type == PROOF_DRAT)
		// Don't delete unit clauses in DRAT
		continue;
	    if (do_binary) {
		check_buffer(ilist_length(clause) + 3);
		d = dest_buf;
		*d++ = 'd';
		if (proof_type == PROOF_FRAT)
		    d += int_byte_pack(cid, d);
		d += ilist_byte_pack(clause, d);
		d += int_byte_pack(0, d);
		rval = fwrite(dest_buf, 1, d - dest_buf, proof_file);
		if (rval < 0)
		    bdd_error(BDD_FILE);
	    } else {
		rval = fprintf(proof_file, "d ");
		if (rval < 0) 
		    bdd_error(BDD_FILE);
		if (proof_type == PROOF_FRAT) {
		    rval = fprintf(proof_file, "%d ", cid);
		    if (rval < 0)
			bdd_error(BDD_FILE);
		}
		rval = ilist_print(clause, proof_file, " ");
		if (rval < 0) 
		    bdd_error(BDD_FILE);
		rval = fprintf(proof_file, " 0\n");
		if (rval < 0) 
		    bdd_error(BDD_FILE);
	    }
	    ilist_free(clause);
	    all_clauses[cid-1] = TAUTOLOGY_CLAUSE;
	}
    }
    live_clause_count -= ilist_length(clause_ids);
}

/* Some deletions must be deferred until top-level apply completes */
void defer_delete_clause(int clause_id) {
    deferred_deletion_list = ilist_push(deferred_deletion_list, clause_id);
}

void process_deferred_deletions() {
    if (deferred_deletion_list && ilist_length(deferred_deletion_list) > 0) {
	print_proof_comment(2, "Performing deferred deletions of %d clauses", ilist_length(deferred_deletion_list));
	delete_clauses(deferred_deletion_list);
	deferred_deletion_list = ilist_resize(deferred_deletion_list, 0);
    }
}


/* Retrieve input clause.  NULL if invalid */
ilist get_input_clause(int id) {
    if (id > input_clause_count)
	return NULL;
    return all_clauses[id-1];
}

bool print_ok(int vlevel) {
    if (proof_type == PROOF_NONE)
	return false;
    if (do_binary)
	return false;
    if (verbosity_level < vlevel+1)
	return false;
    if (proof_type != PROOF_FRAT && empty_clause_detected)
	return false;
    return true;
}

void print_proof_comment(int vlevel, const char *fmt, ...) {
    int rval;
    if (print_ok(vlevel)) {
	va_list vlist;
	rval = fprintf(proof_file, "c ");
	if (rval < 0) 
	    bdd_error(BDD_FILE);
	va_start(vlist, fmt);
	rval = vfprintf(proof_file, fmt, vlist);
	if (rval < 0) 
	    bdd_error(BDD_FILE);
	va_end(vlist);
	rval = fprintf(proof_file, "\n");
	if (rval < 0) 
	    bdd_error(BDD_FILE);
    }
}

/* 
   Fill ilist with defining clause.
   ils should be big enough for 3 elemeents.
   Result should be processed with clean_clause()
*/
ilist defining_clause(ilist ils, dclause_t dtype, int nid, int vid, int hid, int lid) {
    switch(dtype) {
    case DEF_HU:
	ils = ilist_fill3(ils,  nid, -vid, -hid);
	break;
    case DEF_LU:
	ils = ilist_fill3(ils,  nid,  vid, -lid);
	break;
    case DEF_HD:
	ils = ilist_fill3(ils, -nid, -vid,  hid);
	break;
    case DEF_LD:
	ils = ilist_fill3(ils, -nid,  vid,  lid);
	break;
    default:
	/* Should throw exception here */
	ils = TAUTOLOGY_CLAUSE;
    }
    return ils;
}

/******* Support for Apply Proof generation *****/

/*
  Maxima
 */
#define MAX_CLAUSE 4
#define MAX_HINT 8

/*
  Enumerated type for the hint types
 */
typedef enum { HINT_RESHU, HINT_ARG1HD, HINT_ARG2HD, HINT_OPH, HINT_RESLU, HINT_ARG1LD, HINT_ARG2LD, HINT_OPL, HINT_EXTRA } jtype_t;

#define HINT_COUNT HINT_EXTRA

/* There are 8 basic hints, plus possibly an extra one from an intermediate clause */
const char *hint_name[HINT_COUNT+1] = {"RESHU", "ARG1HD", "ARG2HD", "OPH", "RESLU", "ARG1LD", "ARG2LD", "OPL", "EXTRA"};

/*
  Data structures used during proof generation
 */

static int hint_id[HINT_COUNT+1];
static int hint_buf[HINT_COUNT+1][MAX_CLAUSE+ILIST_OVHD];
static ilist hint_clause[HINT_COUNT+1];
static bool hint_used[HINT_COUNT+1];

static jtype_t hint_hl_order[HINT_COUNT] = 
    { HINT_RESHU, HINT_ARG1HD, HINT_ARG2HD, HINT_OPH, HINT_RESLU, HINT_ARG1LD, HINT_ARG2LD, HINT_OPL };

static jtype_t hint_lh_order[HINT_COUNT] = 
    { HINT_RESLU, HINT_ARG1LD, HINT_ARG2LD, HINT_OPL, HINT_RESHU, HINT_ARG1HD, HINT_ARG2HD, HINT_OPH };

static jtype_t hint_h_order[HINT_COUNT/2] = 
    { HINT_RESHU, HINT_ARG1HD, HINT_ARG2HD, HINT_OPH };

/* This one gets used in second half of split proof */
static jtype_t hint_l_order[HINT_COUNT/2+1] = 
    { HINT_EXTRA, HINT_RESLU, HINT_ARG1LD, HINT_ARG2LD, HINT_OPL };


static char hstring[1024];

static void initialize_hints() {
    jtype_t hi;
    for (hi = (jtype_t) 0; hi < HINT_COUNT+1; hi++) {
	hint_id[hi] = TAUTOLOGY;
	hint_clause[hi] = ilist_make(hint_buf[hi], 3);
    }
}

static void complete_hints() {
    jtype_t hi;
    for (hi = (jtype_t) 0; hi < HINT_COUNT+1; hi++) {
	if (hint_id[hi] == TAUTOLOGY)
	    hint_clause[hi] = TAUTOLOGY_CLAUSE;
	else {
	    hint_clause[hi] = clean_clause(hint_clause[hi]);
	    if (hint_clause[hi] == TAUTOLOGY_CLAUSE)
		hint_id[hi] = TAUTOLOGY;
	}
    }
}

static void show_hints() {
    jtype_t hi;
    for (hi = (jtype_t) 0; hi < HINT_COUNT+1; hi++) {
	if (hint_id[hi] != TAUTOLOGY) {
	    fprintf(proof_file, "c    %s: #%d = [", hint_name[hi], hint_id[hi]);
	    ilist_print(hint_clause[hi], proof_file, " ");
	    fprintf(proof_file, "]\n");
	}
    }
}


static ilist target_and(ilist ils, BDD l, BDD r, BDD s) {
    return ilist_fill3(ils, -XVAR(l), -XVAR(r), XVAR(s));
}

static ilist target_imply(ilist ils, BDD l, BDD r) {
    return ilist_fill2(ils, -XVAR(l), XVAR(r));
}
	

static bool rup_check(ilist target_clause, jtype_t *horder, int hcount) {
    int ubuf[8+ILIST_OVHD];
    ilist ulist = ilist_make(ubuf, 8);
    int cbuf[MAX_CLAUSE+ILIST_OVHD];
    ilist cclause = ilist_make(cbuf, MAX_CLAUSE);
    int oi, hi, li, ui;
    for (ui = 0; ui < ilist_length(target_clause); ui++)
	ilist_push(ulist, -target_clause[ui]);
    if (print_ok(4)) {
	fprintf(proof_file, "c RUP start.  Target = [");
	ilist_print(target_clause, proof_file, " ");
	fprintf(proof_file, "]\n");
    }
    for (hi = 0; hi < HINT_COUNT; hi++) 
	hint_used[hi] = false;
    for (oi = 0; oi < hcount; oi++) {
	jtype_t hi = horder[oi];
	if (hint_id[hi] != TAUTOLOGY) {
	    ilist clause = hint_clause[hi];
	    /* Operate on copy of clause so that can manipulate */
	    ilist_resize(cclause, 0);
	    for (li = 0; li < ilist_length(clause); li++)
		ilist_push(cclause, clause[li]);
	    if (print_ok(4)) {
		fprintf(proof_file, "c   RUP step.  Units = [");
		ilist_print(ulist, proof_file, " ");
		fprintf(proof_file, "] Clause = %s\n", hint_name[hi]);
	    }
	    li = 0;
	    while (li < ilist_length(cclause)) {
		int lit = cclause[li];
		if (print_ok(5)) {
		    fprintf(proof_file, "c     cclause = [");
		    ilist_print(cclause, proof_file, " ");
		    fprintf(proof_file, "]  ");
		}
		bool found = false;
		for (ui = 0; ui < ilist_length(ulist); ui++) {
		    if (lit == -ulist[ui]) {
			found = true;
			break;
		    }
		    if (lit == ulist[ui]) {
			if (print_ok(5))
			    fprintf(proof_file, "Unit %d Found.  Creates tautology\n", -lit);
			return false;
		    }
		}
		if (found) { 
		    if (print_ok(5))
			fprintf(proof_file, "Unit %d found.  Deleting %d\n", -lit, lit);
		    if (ilist_length(cclause) == 1) {
			print_proof_comment(4, "   Conflict detected");
			/* Conflict detected */
			hint_used[hi] = true;
			return true;
		    } else {
			/* Remove lit from cclause by swapping with last one */
			int nlength = ilist_length(cclause)-1;
			cclause[li] = cclause[nlength];
			ilist_resize(cclause, nlength);
		    }
		} else {
		    if (print_ok(5))
			fprintf(proof_file, "Unit %d NOT found.  Keeping %d\n", -lit, lit);
		    li++;
		}
	    }
	    if (ilist_length(cclause) == 1) {
		/* Unit propagation */
		print_proof_comment(5, "  Unit propagation of %d", cclause[0]);
		ilist_push(ulist, cclause[0]);
		hint_used[hi] = true;
	    }
	}
    }
    /* Didn't find conflict */
    print_proof_comment(4, "  RUP failed");
    return false;
}



int justify_apply(int op, BDD l, BDD r, int splitVar, TBDD tresl, TBDD tresh, BDD res) {
    int tbuf[MAX_CLAUSE+ILIST_OVHD];
    ilist targ = ilist_make(tbuf, MAX_CLAUSE);
    int itbuf[MAX_CLAUSE+ILIST_OVHD];
    ilist itarg = ilist_make(itbuf, MAX_CLAUSE);
    int abuf[8+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 8);
    int dbuf[1+ILIST_OVHD];
    ilist del = ilist_make(dbuf, 1);
    int oi, hi, li;

    int jid = 0;
    if (op == bddop_andj) {
	targ = clean_clause(target_and(targ, l, r, res));
	print_proof_comment(2, "Generating proof that N%d & N%d --> N%d", bdd_nameid(l), bdd_nameid(r), bdd_nameid(res));
	print_proof_comment(3, "splitVar = %d, tresl.root = N%d, tresh.root = N%d", splitVar, bdd_nameid(tresl.root), bdd_nameid(tresh.root));
    } else {
	targ = clean_clause(target_imply(targ, l, r));
	print_proof_comment(2, "Generating proof that N%d --> N%d", bdd_nameid(l), bdd_nameid(r));
	print_proof_comment(3, "splitVar = %d", splitVar);
    }
    if (targ == TAUTOLOGY_CLAUSE) {
	print_proof_comment(2, "Tautology");
	return TAUTOLOGY;
    }
    if (print_ok(3)) {
	fprintf(proof_file, "Target clause = [");
	ilist_print(targ, proof_file, " ");
	fprintf(proof_file, "]\n");
    }
    

    /* Prepare the candidates */
    initialize_hints();

    if (LEVEL(l) == splitVar) {
	hint_id[HINT_ARG1LD] = bdd_dclause(l, DEF_LD);
	hint_clause[HINT_ARG1LD] = defining_clause(hint_clause[HINT_ARG1LD], DEF_LD, XVAR(l), splitVar, XVAR(HIGH(l)), XVAR(LOW(l)));
	hint_id[HINT_ARG1HD] = bdd_dclause(l, DEF_HD);
	hint_clause[HINT_ARG1HD] = defining_clause(hint_clause[HINT_ARG1HD], DEF_HD, XVAR(l), splitVar, XVAR(HIGH(l)), XVAR(LOW(l)));
    }

    BDD ll = LEVEL(l) == splitVar ? LOW(l) : l;
    BDD lh = LEVEL(l) == splitVar ? HIGH(l) : l;
    BDD rl = LEVEL(r) == splitVar ? LOW(r) : r;
    BDD rh = LEVEL(r) == splitVar ? HIGH(r) : r;

    if (op == bddop_imptstj) {
	if (LEVEL(r) == splitVar) {
	    hint_id[HINT_RESLU] = bdd_dclause(r, DEF_LU);
	    hint_clause[HINT_RESLU] = defining_clause(hint_clause[HINT_RESLU], DEF_LU, XVAR(r), splitVar, XVAR(HIGH(r)), XVAR(LOW(r)));
	    hint_id[HINT_RESHU] = bdd_dclause(r, DEF_HU);
	    hint_clause[HINT_RESHU] = defining_clause(hint_clause[HINT_RESHU], DEF_HU, XVAR(r), splitVar, XVAR(HIGH(r)), XVAR(LOW(r)));
	}
	hint_id[HINT_OPL] = tresl.clause_id;
	hint_clause[HINT_OPL] = target_imply(hint_clause[HINT_OPL], ll, rl);
	hint_id[HINT_OPH] = tresh.clause_id;
	hint_clause[HINT_OPH] = target_imply(hint_clause[HINT_OPH], lh, rh);
    } else {
	if (LEVEL(r) == splitVar) {
	    hint_id[HINT_ARG2LD] = bdd_dclause(r, DEF_LD);
	    hint_clause[HINT_ARG2LD] = defining_clause(hint_clause[HINT_ARG2LD], DEF_LD, XVAR(r), splitVar, XVAR(HIGH(r)), XVAR(LOW(r)));
	    hint_id[HINT_ARG2HD] = bdd_dclause(r, DEF_HD);
	    hint_clause[HINT_ARG2HD] = defining_clause(hint_clause[HINT_ARG2HD], DEF_HD, XVAR(r), splitVar, XVAR(HIGH(r)), XVAR(LOW(r)));
	}
	if (tresl.root != tresh.root) {
	    hint_id[HINT_RESLU] = bdd_dclause(res, DEF_LU);
	    hint_clause[HINT_RESLU] = defining_clause(hint_clause[HINT_RESLU], DEF_LU, XVAR(res), splitVar, XVAR(HIGH(res)), XVAR(LOW(res)));
	    hint_id[HINT_RESHU] = bdd_dclause(res, DEF_HU);
	    hint_clause[HINT_RESHU] = defining_clause(hint_clause[HINT_RESHU], DEF_HU, XVAR(res), splitVar, XVAR(HIGH(res)), XVAR(LOW(res)));
	}
	hint_id[HINT_OPL] = tresl.clause_id;
	hint_clause[HINT_OPL] = target_and(hint_clause[HINT_OPL], ll, rl, tresl.root);
	hint_id[HINT_OPH] = tresh.clause_id;
	hint_clause[HINT_OPH] = target_and(hint_clause[HINT_OPH], lh, rh, tresh.root);
    }

    complete_hints();
    if (print_ok(3)) {
	print_proof_comment(3, "Hints:");
	show_hints();
    }

    bool checked = false;
    if (hint_id[HINT_OPH] == TAUTOLOGY) {
	/* Try for single clause proof */
	if (rup_check(targ, hint_hl_order, HINT_COUNT)) {
	    checked = true;
	    for (oi = 0; oi < HINT_COUNT; oi++) {
		hi = hint_hl_order[oi];
		if (hint_used[hi])
		    ilist_push(ant, hint_id[hi]);
	    }
	    jid = generate_clause(targ, ant);
	}

    }
    if (!checked && hint_id[HINT_OPL] == TAUTOLOGY) {
	if (rup_check(targ, hint_lh_order, HINT_COUNT)) {
	    checked = true;
	    for (oi = 0; oi < HINT_COUNT; oi++) {
		hi = hint_lh_order[oi];
		if (hint_used[hi])
		    ilist_push(ant, hint_id[hi]);
	    }
	    jid = generate_clause(targ, ant);
	}
    }
    if (!checked) {
	ilist_push(itarg, -splitVar);
	for (li = 0; li < ilist_length(targ); li++)
	    ilist_push(itarg, targ[li]);
	itarg = clean_clause(itarg);
	if (!rup_check(itarg, hint_h_order, HINT_COUNT/2)) {
	    fprintf(proof_file, "c  Uh-Oh.  RUP check failed in first half of proof.  Target = [");
	    ilist_print(itarg, proof_file, " ");
	    fprintf(proof_file, "].\n");
	    bdd_error(TBDD_PROOF);
	}
	for (oi = 0; oi < HINT_COUNT/2; oi++) {
	    hi = hint_h_order[oi];
	    if (hint_used[hi])
		ilist_push(ant, hint_id[hi]);
	}
	int iid = generate_clause(itarg, ant);
	hint_id[HINT_EXTRA] = iid;
	hint_clause[HINT_EXTRA] = itarg;
	if (!rup_check(targ, hint_l_order, HINT_COUNT/2+1)) {
	    fprintf(proof_file, "c  Uh-Oh.  RUP check failed in second half of proof.  Target = [");
	    ilist_print(itarg, proof_file, " ");
	    fprintf(proof_file, "].\n");
	    bdd_error(TBDD_PROOF);

	}
	ilist_resize(ant, 0);
	for (oi = 0; oi < HINT_COUNT/2+1; oi++) {
	    hi = hint_l_order[oi];
	    if (hint_used[hi])
		ilist_push(ant, hint_id[hi]);
	}
	// Negate ID to show that two clauses were generated
	//	jid = -generate_clause(targ, ant);
	jid = generate_clause(targ, ant);
	ilist_fill1(del, iid);
	delete_clauses(del);
    }
    return jid;
}


#if 0
/* Initialize set of hints with TAUTOLOGY, and the hint clauses to empty clauses  */
void fill_hints(jtype_t hint_id[HINT_COUNT], int hint_buf[HINT_COUNT][3+ILIST_OVHD], ilist hint_clause[HINT_COUNT]) {
    jtype_t i;
    for (i = (jtype_t) 0; i < HINT_COUNT; i++) {
	hint_id[i] = TAUTOLOGY;
	hint_clause[i] = ilist_make(hint_buf[i], 3);
    }
}


/* Justify results of apply operation.  Return clause ID */
int justify_apply(ilist target_clause, int split_variable, jtype_t hint_id[HINT_COUNT], ilist hint_clause[HINT_COUNT]) {
    int abuf[HINT_COUNT+1+ILIST_OVHD];
    ilist ant = ilist_make(abuf, HINT_COUNT);
    target_clause = clean_clause(target_clause);
    if (target_clause == TAUTOLOGY_CLAUSE)
	return TAUTOLOGY;
    jtype_t i;
    for (i = (jtype_t) 0; i < HINT_COUNT; i++) {
	if (hint_id[i] == TAUTOLOGY)
	    hint_clause[i] = TAUTOLOGY_CLAUSE;
	else {
	    hint_clause[i] = clean_clause(hint_clause[i]);
	    if (hint_clause[i] == TAUTOLOGY_CLAUSE)
		hint_id[i] = TAUTOLOGY;
	    else
		ilist_push(ant, hint_id[i]);
	}
    }
    return generate_clause(target_clause, ant);
}
#endif


