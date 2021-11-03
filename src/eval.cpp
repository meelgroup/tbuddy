#include "bdd.h"
#include "clause.h"

static int next_id = 0;

class Term {
private:
  int id;
  bool is_active;
public:
  bdd *rootp;
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
  
public:

  TermSet(CNF &cnf) {
    bdd_init(5000000,10000000);
    bdd_setvarnum(cnf.max_variable()+1);
    for (int i = 0; i < cnf.clause_count(); i++)
      add(new Term(cnf[i]));
    min_active = 0;
  }
  
  void add(Term *tp) {
    terms.push_back(tp);
  }

  void conjunct(Term *tp1, Term *tp2) {
    bdd nroot = tp1->get_root() & tp2->get_root();
    add(new Term(nroot));
    tp1->deactivate();
    tp2->deactivate();
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
      conjunct(tp1, tp2);
    }
  }
};

bool solve(FILE *cnf_file) {
  CNF cset = CNF(cnf_file);
  if (cset.failed()) {
    std::cout << "Aborted" << std::endl;
    return false;
  }
  std::cout << "Read " << cset.clause_count() << " clauses.  " << cset.max_variable() << " variables" << std::endl;
  TermSet tset = TermSet(cset);
  bdd r = tset.tree_reduce();
  if (r == bddtrue)
    std::cout << "Tautology" << std::endl;
  else if (r == bddfalse)
    std::cout << "Unsatisfiable" << std::endl;
  else {
    std::cout << "Satisfiable.  BDD size = " << bdd_nodecount(r) << std::endl;
    std::cout << "BDD: " << r << std::endl;
  }
  bdd_done();
  return true;
}

