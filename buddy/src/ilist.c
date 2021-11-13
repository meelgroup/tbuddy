#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ilist.h"

#ifdef ENABLE_TBDD
int bdd_error(int);
#define BDD_MEMORY (-1)
#endif

/*============================================
   Integer lists
============================================*/

/* Macros */
#define ILIST_BASE(ils) ((ils)-ILIST_OVHD)
#define ILIST_LENGTH(ils) ((ils)[-1])
#define IABS(x) ((x)<0?(-x):(x))
#define ILIST_MAXLENGTHFIELD(ils) ((ils)[-2])
#define ILIST_MAXLENGTH(ils) (IABS(ILIST_MAXLENGTHFIELD(ils)))

static ilist ilist_error(char *fname) {
    fprintf(stderr, "ilist error in %s\n", fname);
#if ENABLE_TBDD
    bdd_error(BDD_MEMORY);
#else
    exit(1);
#endif
    return NULL;
}

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
     ilist result = p+ILIST_OVHD;
    ILIST_LENGTH(result) = 0;
    ILIST_MAXLENGTHFIELD(result) = -max_length;
    return result;
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

/*
  Change size of ilist.  Can be shorter or longer than current.
  When lengthening, new contents are undefined
*/
ilist ilist_resize(ilist ils, int nlength) {
    int list_max_length = ILIST_MAXLENGTHFIELD(ils);
    int true_max_length = IABS(list_max_length);
    if (nlength > true_max_length) {
	if (list_max_length < 0) {
	    int *p = ILIST_BASE(ils);
	    /* Dynamically resize */
	    true_max_length *= 2;
	    if (nlength > true_max_length)
		true_max_length = nlength;
	    p = realloc(p, (true_max_length + ILIST_OVHD) * sizeof(int));
	    if (p == NULL) {
		/* Need to throw error here */
		return ilist_error("resize (dynamic)");
	    }
	    ils = p+ILIST_OVHD;
	    ILIST_MAXLENGTHFIELD(ils) = -true_max_length;
	} else 
	    /* Need to throw an error here */
	    return ilist_error("resize (static)");
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
    if (!ils) {
	/* Want to throw an exception here */
	return ilist_error("push");
    }
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
    if (!ils) {
	/* Want to throw an exception here */
	return ilist_error("fill1");
    }
    ils[0] = val1;
    return ils;
}

ilist ilist_fill2(ilist ils, int val1, int val2) {
    ils = ilist_resize(ils, 2);
    if (!ils) {
	/* Want to throw an exception here */
	return ilist_error("fill2");
    }
    ils[0] = val1;
    ils[1] = val2;
    return ils;
}

ilist ilist_fill3(ilist ils, int val1, int val2, int val3) {
    ils = ilist_resize(ils, 3);
    if (!ils) {
	/* Want to throw an exception here */
	return ilist_error("fill3");
    }
    ils[0] = val1;
    ils[1] = val2;
    ils[2] = val3;
    return ils;
}

ilist ilist_fill4(ilist ils, int val1, int val2, int val3, int val4) {
    ils = ilist_resize(ils, 4);
    if (!ils) {
	/* Want to throw an exception here */
	return ilist_error("fill4");
    }
    ils[0] = val1;
    ils[1] = val2;
    ils[2] = val3;
    ils[3] = val4;
    return ils;
}

/*
  Dynamically allocate ilist and fill from array
 */
ilist ilist_copy_list(int *ls, int length) {
    ilist rils =  ilist_new(length);
    rils = ilist_resize(rils, length);
    memcpy(rils, ls, length*sizeof(int));
    return rils;
}


/*
  Dynamically allocate ilist and copy from existing one.
 */
ilist ilist_copy(ilist ils) {
    return ilist_copy_list(ils, ilist_length(ils));
}

/*
  Reverse elements in ilist
 */
void ilist_reverse(int *ils) {
    int left = 0;
    int right = ilist_length(ils)-1;
    while (left < right) {
	int v = ils[left];
	ils[left] = ils[right];
	ils[right] = v;
	left++;
	right--;
    }
}


/*
  Print elements of an ilist separated by sep
 */
void ilist_print(ilist ils, FILE *out, char *sep) {
    int i;
    char *space = "";
    if (ils == TAUTOLOGY_CLAUSE) {
	fprintf(out, "TAUT");
	return;
    }
    for (i = 0; i < ilist_length(ils); i++) {
	fprintf(out, "%s%d", space, ils[i]);
	space = sep;
    }
}
