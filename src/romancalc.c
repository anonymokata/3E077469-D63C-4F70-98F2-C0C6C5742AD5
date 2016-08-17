/*******************************************
 *  Roman calculator                       *
 *  20160730   July-30-2016 created        *
 *  Micah Wilson                           *
 *******************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "romancalc.h"


//char *rdigits=RN_1000 RN_500 RN_100 RN_50 RN_10 RN_5 RN_1;	// roman numerals in order
// this defines the rdigits roman numeral digit list
// and is define withing the romancalc.h for testing
DEF_RDIGITS


/*******************************************************
 * rnum_valid_numeral_str
 * int rnum_valid_numeral_str(char *rn_exp_str);
 *
 *   checks that value is roman numeral
 *
 * input:
 *	     rn_str  NULL terminated string containg
 *                 capitalized roman numeral
 * returns:
 *    int error
 *******************************************************/
int rnum_valid_numeral_str(char *rn_str){
	char str_out_tmp[TSTR_LEN];									// temp store output value for re-running loop
	memset(str_out_tmp, 0, TSTR_LEN);							// init with null terminators
	

	return RNUM_ERR_NONE;
}
/* end of romancalc.c */
