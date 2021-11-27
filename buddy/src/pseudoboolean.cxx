#include "pseudoboolean.h"

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

/* Form xor sum of coefficients */
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


xor_constraint::xor_constraint(ilist vars, int p, tbdd &vfun) {
  variables = vars;
  phase = p;
  bdd xfun = bdd_build_xor(variables, phase);
  validation = tbdd_validate(xfun, vfun);
}

#if 0
xor_constraint::xor_constraint(ilist variables, int phase) {
  bdd xfun = bdd_build_xor(variables, phase);
  validation = tbdd_trust(xfun);
}
#endif

int xor_constraint::validate_clause(ilist clause) {
  return tbdd_validate_clause(clause, validation);
}

#if 0
xor_constraint xor_constraint::plus(xor_constraint &other) {
  ilist nvariables = xor_sum(variables, other.variables);
  int nphase = phase ^ other.phase;
  tbdd nvalidation = tbdd_and(validation, other.validation);

  printf("XOR sum: ");
  show_xor(stdout, variables, phase);
  printf("  +  ");
  show_xor(stdout, other.variables, other.phase);
  printf("   -->  ");
  show_xor(stdout, nvariables, nphase);
  printf("\n");
  return xor_constraint(nvariables, nphase, nvalidation);
}
#endif

void xor_constraint::accum(xor_constraint &other) {
  ilist nvariables = xor_sum(variables, other.variables);
  int nphase = phase ^ other.phase;
  tbdd nvalidation = tbdd_and(validation, other.validation);

#if 0
  printf("XOR sum: ");
  show_xor(stdout, variables, phase);
  printf("  +  ");
  show_xor(stdout, other.variables, other.phase);
  printf("   -->  ");
  show_xor(stdout, nvariables, nphase);
  printf("\n");
#endif
  ilist_free(variables);
  variables = nvariables;
  phase = nphase;
  validation = nvalidation;
}


void xor_constraint::show(FILE *out) {
  fprintf(out, "Xor Constraint: Node N%d validates ", bdd_nameid(validation.get_root().get_BDD()));
  show_xor(out, variables, phase);
  fprintf(out, "\n");
}

xor_constraint *sum_list(xor_constraint **xlist, int len) {
  if (len == 0)
    return new xor_constraint();
  xor_constraint s;
  for (int i = 0; i < len; i++) {
    xor_constraint a = *xlist[i];
    s.accum(a);
    if (!s.is_feasible())
      break;
  }
  return new xor_constraint(s);
}

