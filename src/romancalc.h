#ifndef __ROMANCALC_H__
#define __ROMANCALC_H__
#include <stdlib.h>
#include <stdbool.h>
//*******************************************/
//*  Roman calculator                       *
//*  20160730   July-30-2016 created        *
//*  Micah Wilson							*
//* Rules for inputs
//* Digit	Value	MaxDigit
//*	I		   1		3
//*	V		   5		1
//*	X		  10		3
//*	L		  50		1
//*	C		 100		3
//*	D		 500		1
//*	M		1000		3					*
//*											*
//* Large Test Inputs MMMDCCCLXXXVIII		3888	*
//* Large Test Inputs MMMMMMMDCCLXXVI		7776
//* (Using defined roman numerals)
//* Max Output string width 15+1 (null terminated)
//* Max Input  string width 15+1 (null terminated)
//********************************************/

// each roman numeral can be maximum input
// length of 512 roman numeral digits
// allows the the user to be add bit digits
#define MAX_STR_LEN_ROMAN_INPUT (512)
#define MAX_STR_LEN_ROMAN_NUM ((MAX_STR_LEN_ROMAN_INPUT*4)+100)

// allows for expansion and contraction of roman numerals
#define TSTR_LEN (MAX_STR_LEN_ROMAN_NUM * 16)

// string single digit numerals
#define RN_1000   "M"
#define RN_500    "D"
#define RN_100    "C"
#define RN_50     "L"
#define RN_10     "X"
#define RN_5      "V"
#define RN_1      "I"

// char single digit numerals
#define RN_1000ch ( RN_1000 [0] )
#define RN_500ch  ( RN_500  [0] )
#define RN_100ch  ( RN_100  [0] )
#define RN_50ch   ( RN_50   [0] )
#define RN_10ch   ( RN_10   [0] )
#define RN_5ch    ( RN_5    [0] )
#define RN_1ch    ( RN_1    [0] )

// proper multi numeral sequences
#define RN_900  RN_100 RN_1000
#define RN_400  RN_100 RN_500
#define RN_90   RN_10  RN_100
#define RN_40   RN_10  RN_50
#define RN_9    RN_1   RN_10
#define RN_4    RN_1   RN_5

// improper multi numeral sequences
#define TWO_500s  RN_500 RN_500
#define TWO_50s   RN_50  RN_50
#define TWO_5s    RN_5   RN_5

#define FIVE_100s RN_100 RN_100 RN_100 RN_100 RN_100
#define FIVE_10s  RN_10  RN_10  RN_10  RN_10  RN_10
#define FIVE_1s   RN_1   RN_1   RN_1   RN_1   RN_1

#define FOUR_100s RN_100 RN_100 RN_100 RN_100
#define FOUR_10s  RN_10  RN_10  RN_10  RN_10
#define FOUR_1s   RN_1   RN_1   RN_1   RN_1

#define IMP_900 RN_500 FOUR_100s
#define IMP_400 FOUR_100s
#define IMP_90  RN_50  FOUR_10s
#define IMP_40  FOUR_10s
#define IMP_9   RN_5   FOUR_1s
#define IMP_4   FOUR_1s

// addition delimeter
#define DELIMETER_LIST "+-"

#define DEF_RDIGITS const char *rdigits=RN_1000 RN_500 RN_100 RN_50 RN_10 RN_5 RN_1;	// roman numerals in order
#define DEF_RDIGITS_LEN (strlen(rdigits)+1)						// rdigits length plus null

extern const char *rdigits;

typedef enum {
	RNUM_ERR_NONE               = 0,
} rnum_err_enum ;

#endif

