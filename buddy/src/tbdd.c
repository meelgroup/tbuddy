#include <stdlib.h>

#include "tbdd.h"

/* Macros */
#define ILIST_BASE(ils) ((ils)-ILIST_OVHD)
#define ILIST_LENGTH(ils) ((ils)[-1])
#define IABS(x) ((x)<0?(-x):(x))
#define ILIST_MAXLENGTHFIELD(ils) ((ils)[-2])
#define ILIST_MAXLENGTH(ils) (IABS(ILIST_MAXLENGTHFIELD(ils)))
/* 
   Convert an array of ints to an ilist.  Don't call free_ilist on
   this one!  The size of the array should be max_length + ILIST_OVHD
   Will be statically sized
*/
ilist ilist_make(int *p, int max_length) {
    ilist result = p+ILIST_OVHD;
    ILIST_LENGTH(result) = 0;
    ILIST_MAXLENGTHFIELD(result) = max_length;
    return result;
}

/* Allocate a new ilist. */
ilist ilist_new(int max_length) {
    int *p = calloc(max_length + ILIST_OVHD, sizeof(int));
    return ilist_make(p, -max_length); 
}

/* Free allocated ilist.  Only call on ones allocated via ilist_new */
void ilist_free(ilist ils) {
    int *p = ILIST_BASE(ils);
    free(p);
}

/* Return number of elements in ilist */
int ilist_length(ilist ils) {
    return ILIST_LENGTH(ils);
}

static ilist ilist_resize(ilist ils, int nlength) {
    int list_max_length = ILIST_MAXLENGTH(ils);
    int true_max_length = IABS(list_max_length);
    if (nlength > true_max_length) {
	if (list_max_length < 0) {
	    int *p = ILIST_BASE(ils);
	    /* Dynamically resize */
	    true_max_length *= 2;
	    if (nlength > true_max_length)
		true_max_length = nlength;
	    p = realloc(p, true_max_length * sizeof(int) + ILIST_OVHD);
	    if (p == NULL) {
		/* Need to throw error here */
		return NULL;
	    }
	    ils = p+ILIST_OVHD;
	    ILIST_MAXLENGTHFIELD(ils) = -true_max_length;
	} else 
	    /* Need to throw an error here */
	    return NULL;
    }
    ILIST_LENGTH(ils) = nlength;
    return ils;
}

/*
  Add new value(s) to end of ilist.
  For dynamic ilists, the value of the pointer may change
*/
ilist ilist_push(ilist ils, int val) {
    int length = ILIST_LENGTH(ils);
    int nlength = length+1;
    ils = ilist_resize(ils, nlength);
    ils[length] = val;
    ILIST_LENGTH(ils) = nlength;
    return ils;
}

/*
  Populate ilist with 1, 2, 3, or 4 elements.
  For dynamic ilists, the value of the pointer may change
 */
ilist ilist_fill1(ilist ils, int val1) {
    ils = ilist_resize(ils, 1);
    ils[0] = val1;
    return ils;
}

ilist ilist_fill2(ilist ils, int val1, int val2) {
    ils = ilist_resize(ils, 2);
    ils[0] = val1;
    ils[1] = val2;
    return ils;
}

ilist ilist_fill3(ilist ils, int val1, int val2, int val3) {
    ils = ilist_resize(ils, 3);
    ils[0] = val1;
    ils[1] = val2;
    ils[2] = val3;
    return ils;
}

ilist ilist_fill4(ilist ils, int val1, int val2, int val3, int val4) {
    ils = ilist_resize(ils, 4);
    ils[0] = val1;
    ils[1] = val2;
    ils[2] = val3;
    ils[3] = val4;
    return ils;

}
