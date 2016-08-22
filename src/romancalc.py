
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
	
	out_str = ""
	# method is to add in each digit time the number of occurrences in the
	# original string, in magnitude order
	# order from left to right is MDCLXVI
	rn_str = rn_str.upper() # work in upper case
	out_str += 'M' * rn_str.count('M')
	out_str += 'D' * rn_str.count('D')
	out_str += 'C' * rn_str.count('C')
	out_str += 'L' * rn_str.count('L')
	out_str += 'X' * rn_str.count('X')
	out_str += 'V' * rn_str.count('V')
	out_str += 'I' * rn_str.count('I')

	return out_str

if __name__ == "__main__":


	print sys.argv
