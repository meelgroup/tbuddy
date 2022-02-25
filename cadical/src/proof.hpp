#ifndef _proof_h_INCLUDED
#define _proof_h_INCLUDED

namespace CaDiCaL {

/*------------------------------------------------------------------------*/

class File;
struct Clause;
struct Internal;
class Observer;

/*------------------------------------------------------------------------*/

// Provides proof checking and writing through observers.

class Proof {

  Internal * internal;

  vector<int> clause;           // of external literals
  vector<Observer *> observers; // owned, so deleted in destructor

  void add_literal (int internal_lit);  // add to 'clause'
  void add_literals (Clause *);         // add to 'clause'

  void add_literals (const vector<int> &);      // ditto

  void add_original_clause (int64_t); // notify observers of original clauses
  void add_derived_clause (int64_t);  // notify observers of derived clauses
  void delete_clause (int64_t);       // notify observers of deleted clauses
  void finalize_clause (int64_t);     // notify observers of active clauses

public:

  Proof (Internal *);
  ~Proof ();

  void connect (Observer * v) { observers.push_back (v); }

  // Add original clauses to the proof (for online proof checking).
  //
  void add_original_clause (int64_t id, const vector<int> &);

  // Add derived (such as learned) clauses to the proof.
  //
  void add_derived_empty_clause (int64_t id);
  void add_derived_unit_clause (int64_t id, int unit);
  void add_derived_clause (Clause *);
  void add_derived_clause (int64_t id, const vector<int> &);

  void delete_clause (int64_t, const vector<int> &);
  void delete_clause (Clause *);

  // notify observers of active clauses (deletion after empty clause)
  //
  void finalize_clause (int64_t, const vector<int> &);
  void finalize_clause (Clause *);
  void finalize_clause_ext (int64_t, const vector<int> &);

  // These two actually pretend to add and remove a clause.
  //
  void flush_clause (Clause *);           // remove falsified literals
  void strengthen_clause (Clause *, int); // remove second argument

  void add_todo (const vector<int64_t> &);

  void flush ();
};

}

#define PROOF_TODO(proof, s, ...) if (proof) { \
  LOG ("PROOF missing chain (" s ")"); \
  proof->add_todo({__VA_ARGS__}); }

#endif
