
import sys
import re


def TestRoutineCheck__in_str__out_str(check_str):
	return check_str

def TestRoutineCheck__in_str__out_int(check_str):
	
	return 	int(check_str)

def TestRoutineCheck__in_str__out_bool(check_str):
	
	return 	check_str == "TEST_STRING"

# validate that a roman numeral digit string is a proper value
# this takes into account how many of which digits and properly
# subtracted mid-string digits ie IV and IX and CM, etc
# this uses a standard roman numeral regular expression

def rn_numeral_validate_bool(check_str):

# use regalar expression pattern to validate
	check_str = check_str.upper() # work in upper case
	rslt_out = re.match("^(?=[MDCLXVI])M*(C[MD]|D?C{0,3})(X[CL]|L?X{0,3})(I[XV]|V?I{0,3})$", check_str, re.IGNORECASE)

	return rslt_out != None

# this unrolls mix roman numeral values into improper values
# this makes addition and subtraction easier
# i.e. IV -> IIII, IX -> VIIII, XL -> XXXX,
# XC -> LXXXX, CD -> CCCC, CM -> DCCCC

def rn_numeral_digit_unmix(rn_str):
	rn_str = rn_str.upper() # work in upper case
	rn_str = rn_str.replace("IV", "IIII")
	rn_str = rn_str.replace("IX", "VIIII")
	rn_str = rn_str.replace("XL", "XXXX")
	rn_str = rn_str.replace("XC", "LXXXX")
	rn_str = rn_str.replace("CD", "CCCC")
	rn_str = rn_str.replace("CM", "DCCCC")
	
	return rn_str

#  rn_numeral_digit_sort
#*  ensure digits with roman numeral string are sorted in value order
# *  Since M = 1000, D = 500, C = 100, L = 50, X = 10, V = 5, I = 1
# *  then the order from left to right is MDCLVI within any numeral string

def rn_numeral_digit_sort(rn_str):

	return rn_str

if __name__ == "__main__":


	print sys.argv
