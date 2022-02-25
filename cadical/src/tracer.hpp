#ifndef _tracer_h_INCLUDED
#define _tracer_h_INCLUDED

#include "observer.hpp" // Alphabetically after 'tracer'.

// Proof tracing to a file (actually 'File') in DRAT format.

namespace CaDiCaL {

class Tracer : public Observer {
  Internal * internal;
  File * file;
  bool binary, lrat;
  void put_binary_zero ();
  void put_binary_lit (int external_lit);
  void put_binary_unsigned (int64_t n);
  void put_binary_signed (int64_t n);
  int64_t added, deleted;
public:

  Tracer (Internal *, File * file, bool binary, bool lrat); // own and delete 'file'
  ~Tracer ();

  void add_original_clause (int64_t, const vector<int> &);
  void add_derived_clause (int64_t, const vector<int64_t> *, const vector<int> &);
  void delete_clause (int64_t, const vector<int> &);
  void finalize_clause (int64_t, const vector<int> &);
  void add_todo (const vector<int64_t> &);
  bool closed ();
  void close ();
  void flush ();
};

}

#endif
