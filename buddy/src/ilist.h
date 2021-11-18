#ifndef _ILIST_H
#define _ILIST_H

#include <stdio.h>
#include <limits.h>

/*============================================
   Integer lists
============================================*/

/* Allow this headerfile to define C++ constructs if requested */
#ifdef __cplusplus
#define CPLUSPLUS
#endif

#ifdef CPLUSPLUS
extern "C" {
#endif

/*
  Data type ilist is used to represent clauses and clause id lists.
  These are simply lists of integers, where the value at position -1
  indicates the list length and the value at position -2 indicates the
  maximum list length.  The value at position -2 is positive for
  statically-allocated ilists and negative for ones that can be
  dynamically resized.
*/
typedef int *ilist;
  
/*
  Difference between ilist maximum length and number of allocated
  integers
 */
#define ILIST_OVHD 2

/*
  Pseudo-clause representing tautology
 */
#define TAUTOLOGY_CLAUSE ((ilist) INT_MAX)

/* 
   Convert an array of ints to an ilist.  Don't call free_ilist on
   this one!  The size of the array should be max_length + ILIST_OVHD
   Will be statically sized
*/
extern ilist ilist_make(int *p, int max_length);

/* Allocate a new ilist. */
extern ilist ilist_new(int max_length);

/* Free allocated ilist.  Only call on ones allocated via ilist_new */
extern void ilist_free(ilist ils);

/* Return number of elements in ilist */
extern int ilist_length(ilist ils);

/*
  Change size of ilist.  Can be shorter or longer than current.
  When lengthening, new contents are undefined
*/
extern ilist ilist_resize(ilist ils, int nlength);

/*
  Add new value(s) to end of ilist.
  For dynamic ilists, the value of the pointer may change
*/
extern ilist ilist_push(ilist ils, int val);

/*
  Populate ilist with 1, 2, 3, or 4 elements.
  For dynamic ilists, the value of the pointer may change
 */
extern ilist ilist_fill1(ilist ils, int val1);
extern ilist ilist_fill2(ilist ils, int val1, int val2);
extern ilist ilist_fill3(ilist ils, int val1, int val2, int val3);
extern ilist ilist_fill4(ilist ils, int val1, int val2, int val3, int val4);

/*
  Dynamically allocate ilist and copy from existing one.
 */
extern ilist ilist_copy(ilist ils);

/*
  Dynamically allocate ilist and fill from array
 */
extern ilist ilist_copy_list(int *ls, int length);

/*
  Reverse elements in ilist
 */
extern void ilist_reverse(int *ls);

/*
  Print elements of an ilist separated by sep
 */
extern void ilist_print(ilist ils, FILE *out, char *sep);

/*
  Format string of elements of an ilist separated by sep
 */
extern void ilist_format(ilist ils, char *out, char *sep, int maxlen);
    

#ifdef CPLUSPLUS
}
#endif


#endif /* _ILIST_H */
/* EOF */

