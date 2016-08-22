
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
	rslt_out = re.match("^(?=[MDCLXVI])M*(C[MD]|D?C{0,3})(X[CL]|L?X{0,3})(I[XV]|V?I{0,3})$", check_str, re.IGNORECASE)

	return rslt_out != None

# this unrolls mix roman numeral values into improper values
# this makes addition and subtraction easier
# i.e. IV -> IIII, IX -> VIIII, XL -> XXXX,
# XC -> LXXXX, CD -> CCCC, CM -> DCCCC

def rn_numeral_digit_unmix(rn_str):
	
	return rn_str

if __name__ == "__main__":


	print sys.argv
