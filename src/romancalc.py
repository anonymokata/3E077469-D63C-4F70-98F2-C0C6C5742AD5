
import sys
import re

rn_digits = ['M','D','C','L','X','V','I']

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
	for idx in rn_digits:
		out_str += idx * rn_str.count(idx)

	return out_str

# roman numeral reduction
 #  reduces multiple digits into the next higher value
 #  these larger number of digits either occur due to addition
 #  or possibly the result of borrowing during subtraction
 #  i.e.
 # IIIII -> V,
 # VV -> X,
 # XXXXX -> L,
 # LL -> C,
 # CCCCC -> D,
 # DD -> M
def rn_numeral_digit_reduction(rn_str):
	out_str = rn_str.upper() # work in upper case
	out_str = out_str.replace("IIIII","V",20)
	out_str = out_str.replace("VV"   ,"X",20)
	out_str = out_str.replace("XXXXX","L",20)
	out_str = out_str.replace("LL"   ,"C",20)
	out_str = out_str.replace("CCCCC","D",20)
	out_str = out_str.replace("DD"   ,"M",20)
	return out_str

# this re-mixes roman numeral values into proper values
# i.e.
# DCCCC -> CM,
#  CCCC -> CD,
# LXXXX -> XC,
#  XXXX -> XL,
# VIIII -> IX,
#  IIII -> IV,

def rn_numeral_digit_remix(rn_str):
	out_str = rn_str.upper() # work in upper case
	out_str = out_str.replace("DCCCC" ,"CM")
	out_str = out_str.replace( "CCCC" ,"CD")
	out_str = out_str.replace("LXXXX" ,"XC")
	out_str = out_str.replace( "XXXX" ,"XL")
	out_str = out_str.replace("VIIII" ,"IX")
	out_str = out_str.replace( "IIII" ,"IV")
	return out_str


 # rn_compare
 # compares 2 roman numerals, A is first value, B is second
 # both A and B MUST be unmixed/unrolled i.e. NO "IV" must be "IIII"
 # A > B = 1
 # A == B = 0
 # A < B = -1
# start by comparing largest magnitude digit
# if not equal then return result
# else if 0 then can't determine until check next lower digit
def rn_compare(rn_A, rn_B):
	rn_A = rn_A.upper() # work in upper case
	rn_B = rn_B.upper() # work in upper case
	rslt_out = 0
	for idx in rn_digits:
		rslt_out = rn_A.count(idx) - rn_B.count(idx)
		if rslt_out != 0:					# if not equal, then show result
			return (rslt_out/abs(rslt_out))	# return sign only
	return 0

if __name__ == "__main__":


	print sys.argv
