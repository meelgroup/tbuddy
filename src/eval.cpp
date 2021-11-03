#include "bdd.h"
#include "clause.h"

// Buddy paramters
#define BUDDY_NODES (2*1000*1000)
#define BUDDY_CACHE_RATIO 4


static int next_id = 0;

class Term {
private:
  int id;
  bool is_active;
  bdd *rootp;

public:
  Term (bdd &r) { id = next_id++; rootp = new bdd(r); is_active = true; }
  Term (Clause *clp) { id = next_id++; rootp = new bdd(build_from_clause(clp)); is_active = true; }
  
  void deactivate() {
    delete rootp;
    is_active = false;
  }

  bool active() {
    return is_active;
  }

  bdd get_root() { return *rootp; }

  int get_id() { return id; }

private:
  
  bdd build_from_clause(Clause *clp) {
    if (clp->tautology())
      return bddtrue;
    bdd result = bddfalse;
    for (int i = clp->length()-1; i >= 0; i--) {
      int32_t lit = (*clp)[i];
      bdd blit = lit < 0 ? bdd_nithvar(-lit) : bdd_ithvar(lit);
      result |= blit;
    }
    return result;
  }

};

class TermSet {
private:
  int min_active;
  std::vector<Term *> terms;
  int32_t max_variable;
  int verblevel;
  // Statistics
  int and_count;
  int quant_count;
  int max_bdd;

public:

  TermSet(CNF &cnf, int verb) {
    verblevel = verb;
    bdd_init(BUDDY_NODES,BUDDY_NODES/BUDDY_CACHE_RATIO);
    bdd_setcacheratio(BUDDY_CACHE_RATIO);
    max_variable = cnf.max_variable();
    bdd_setvarnum(max_variable+1);
    for (int i = 0; i < cnf.clause_count(); i++)
      add(new Term(cnf[i]));
    min_active = 0;
    and_count = 0;
    quant_count = 0;
    max_bdd = 0;
  }
  
  void add(Term *tp) {
    max_bdd = std::max(max_bdd, bdd_nodecount(tp->get_root()));
    terms.push_back(tp);
  }

  Term *conjunct(Term *tp1, Term *tp2) {
    bdd nroot = tp1->get_root() & tp2->get_root();
    add(new Term(nroot));
    tp1->deactivate();
    tp2->deactivate();
    and_count++;
    return terms[terms.size()-1];
  }

  Term *equantify(Term *tp, Clause *clp) {
    int32_t *varset = &(*clp)[0];
    bdd varbdd = bdd_makeset(varset, clp->length());
    bdd nroot = bdd_exist(tp->get_root(), varbdd);
    add(new Term(nroot));
    tp->deactivate();
    quant_count++;
    return terms[terms.size()-1];
  }

  Term *equantify(Term *tp, int32_t var) {
    bdd varbdd = bdd_ithvar(var);
    bdd nroot = bdd_exist(tp->get_root(), varbdd);
    add(new Term(nroot));
    tp->deactivate();
    quant_count++;
    return terms[terms.size()-1];
  }

  // Form conjunction of terms until reduce to <= 1 term
  // Effectively performs a tree reduction
  // Return final bdd
  bdd tree_reduce() {
    Term *tp1, *tp2;
    while (true) {
      while (min_active < terms.size() && !terms[min_active]->active())
	min_active++;
      if (min_active >= terms.size())
	// Didn't find any terms.  Formula is tautology
	return bddtrue;
      tp1 = terms[min_active++];
      while (min_active < terms.size() && !terms[min_active]->active())
	min_active++;
      if (min_active >= terms.size()) {
	// There was only one term left
	bdd result = tp1->get_root();
	tp1->deactivate();
	return result;
      }
      tp2 = terms[min_active++];
      Term *tpn = conjunct(tp1, tp2);
    }
  }

  bdd bucket_reduce() {
    std::vector<int> *buckets = new std::vector<int>[max_variable+1];
    int tcount = 0;
    int bcount = 0;
    for (int i = min_active; i < terms.size(); i++) {
      Term *tp = terms[i];
      if (!tp->active())
	continue;
      bdd root = tp->get_root();
      if (root == bddfalse)
	// Formula is trivially false
	return bddfalse;
      if (root != bddtrue) {
	int top = bdd_var(root);
	if (buckets[top].size() == 0)
	  bcount++;
	buckets[top].push_back(tp->get_id());
	tcount++;
      }
    }
    if (verblevel >= 1)
      std::cout << "Placed " << tcount << " terms into " << bcount << " buckets." << std::endl;

    for (int bvar = 1 ; bvar <= max_variable; bvar++) {
      while (buckets[bvar].size() > 1) {
	Term *tp1 = terms[buckets[bvar].back()];
	buckets[bvar].pop_back();
	Term *tp2 = terms[buckets[bvar].back()];
	buckets[bvar].pop_back();
	Term *tpn = conjunct(tp1, tp2);
	bdd root = tpn->get_root();
	if (root == bddfalse) {
	  if (verblevel >= 3)
	    std::cout << "Bucket " << bvar << " Conjunction of terms " 
		      << tp1->get_id() << " and " << tp2->get_id() << " yields FALSE" << std::endl;
	  return bddfalse;
	}
	int top = bdd_var(root);
	if (verblevel >= 3)
	  std::cout << "Bucket " << bvar << " Conjunction of terms " 
		    << tp1->get_id() << " and " << tp2->get_id() << " yields term " 
		    << tpn->get_id() << " with top variable " << top << std::endl;
	buckets[top].push_back(tpn->get_id());
      }
      if (buckets[bvar].size() == 1) {
	Term *tp = terms[buckets[bvar].back()];
	buckets[bvar].pop_back();
	Term *tpn = equantify(tp, bvar);
	bdd root = tpn->get_root();
	if (root == bddtrue) {
	  if (verblevel >= 3)
	    std::cout << "Bucket " << bvar << " Quantification of term " 
		      << tp->get_id() << " yields TRUE" << std::endl;
	} else {
	  int top = bdd_var(root);
	  buckets[top].push_back(tpn->get_id());
	  if (verblevel >= 3)
	    std::cout << "Bucket " << bvar << " Quantification of term " 
		      << tp->get_id() << " yields term " << tpn->get_id() 
		      << " with top variable " << top << std::endl;
	}
      }
    }
    // If get here, formula must be satisfiable
    return bddtrue;
  }

  void show_statistics() {
    std::cout << and_count << " conjunctions, " << quant_count << " quantifications." << std::endl;
    std::cout << "Max BDD size = " << max_bdd << std::endl;
  }

};

bool solve(FILE *cnf_file, bool bucket, int verblevel) {
  CNF cset = CNF(cnf_file);
  if (cset.failed()) {
    if (verblevel >= 1)
      std::cout << "Aborted" << std::endl;
    return false;
  }
  if (verblevel >= 1)
    if (verblevel >= 1)
      std::cout << "Read " << cset.clause_count() << " clauses.  " 
		<< cset.max_variable() << " variables" << std::endl;
  TermSet tset = TermSet(cset, verblevel);
  bdd r = bucket ? tset.bucket_reduce() : tset.tree_reduce();
  if (r == bddtrue)
    std::cout << "Tautology" << std::endl;
  else if (r == bddfalse)
    std::cout << "Unsatisfiable" << std::endl;
  else {
    std::cout << "Satisfiable.  BDD size = " << bdd_nodecount(r) << std::endl;
    if (verblevel >= 3)
      std::cout << "BDD: " << r << std::endl;
  }
  if (verblevel >= 1)
    tset.show_statistics();
  bdd_done();
  return true;
}

