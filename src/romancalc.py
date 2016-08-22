
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
##   Since M = 1000, D = 500, C = 100, L = 50, X = 10, V = 5, I = 1
##   then the order from left to right is MDCLVI within any numeral string

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
def rn_unmixed_compare(rn_A, rn_B):
	rn_A = rn_A.upper() # work in upper case
	rn_B = rn_B.upper() # work in upper case
	rslt_out = 0
	for idx in rn_digits:
		rslt_out = rn_A.count(idx) - rn_B.count(idx)
		if rslt_out != 0:					# if not equal, then show result
			return (rslt_out/abs(rslt_out))	# return sign only
	return 0

#  romancalc_suite_rn_unmixed_subt_LG_SML
#    mid process method test, this python routine is not final value
#    test subtraction, routine performs A - B,
#    where
#    (1) always A > B
#    (2) equality check to ensure A>B ALWAYS done before this routine
#    and (3) A and B are both unmixed digits, by rn_numeral_digit_unmix
# 
#  ensuring that A>B occurs outside of this python routine
#  ensuriong that A and B are unmixed occurrs outside of this python routine
#  RESULTS:
#    result will always be positive
#    result will NEVER BE ZERO
#    result will be unmixed romannumeral
# NOTE: terminology for subtraction
# Difference = Minuend - Subtrahend

def rn_unmixed_subt_LG_SML(rn_A, rn_B):
	rn_A = rn_A.upper() # work in upper case
	rn_B = rn_B.upper() # work in upper case
	
	idx_b_range = range(0, len(rn_B))		# iterating backwards through b
	
	# perform the quick dirty subtraction of digits that we can do without borrowing
	# this does straight digit subtraction, on digits that it can
	# by this I mean if a digit from the rn_B is als in rn_A, then it removes
	# the digit from both rn_A and rn_B, ie subtracts the digit from bot
	
	for idx_b in reversed(idx_b_range):			# loop from smallest numeral to larges(right to left)
		idx_c = rn_A.rfind(rn_B[idx_b])			# look for each char
		if idx_c >= 0:							# did we find smaller value within
			rn_A = rn_A[:idx_c]+rn_A[idx_c+1:]	# slice out(subtract) that Numeral digit
			rn_B = rn_B[:idx_b]+rn_B[idx_b+1:]	# slice out(subtract) that Numeral digit
		else:									# else we need to borrow
			rn_A = rn_unmixed_borrow(rn_A, rn_B)# calculate a Minuend that is borrowable
			rn_A = rn_unmixed_subt_LG_SML(rn_A, rn_B)	# finish the subtraction recursively
			break

	return rn_A

#  romancalc_suite_rn_unmixed_borrow
#    will borrow appropriate value
#    such that rn_B can be subtracted from RN_A
#    will find the next larger in magnitude of
#    rn_B within rn_A, and change split the value
#    into the next smaller value so that a borrow
#    can be made
#
def rn_unmixed_borrow(rn_A, rn_B):
	# rn_brw roman numear browwing array
	# first  value = value we need be able to subtract
	# second value = next higher in magnitude we can borrow from
	# third  value = value split to borrow from
	# this mimic the actual borrow process,
	# check next higher digit,
	#     if exists then spllit and return
	#     if doesn't exist then check next higher
	#         if exists then split
	#         return, and split again
	#         until done
	rn_brw = [('I', 'V', 'IIIII'),
			  ('V', 'X', 'VV'),
			  ('X', 'L', 'XXXXX'),
			  ('L', 'C', 'LL'),
			  ('C', 'D', 'CCCCC'),
			  ('D', 'M', 'DD')]
	rn_A = rn_A.upper() # work in upper case
	rn_B = rn_B.upper() # work in upper case
	
	rslt_out = ""

	idx_brw = [x[0] for x in rn_brw].index(rn_B)
	idx_c = rn_A.rfind(rn_brw[idx_brw][1])
	
	if idx_c >= 0:			# if found a value we can borrow from,
		rslt_out = rn_A[:idx_c]
		rslt_out += rn_brw[idx_brw][2]
		rslt_out += rn_A[idx_c+1:]
	else:
		#simple concept if the digit above the one we need to borrow from deos not exist, then
		# check the value above that for a borrowable value, and do this recursively to use same algorithm
		rslt_out = rn_unmixed_borrow(rn_A, rn_brw[idx_brw][1])		# check next higher digit
		rslt_out = rn_unmixed_borrow(rslt_out, rn_B)				# then re-check if borrowable digit

	rslt_out = rn_numeral_digit_sort(rslt_out)						# keep digits in proper numerical magnitude order
	return rslt_out

#  rn_subtraction_full
#	  full subtraction of two numerals
#     conditions  A > B, or A < B or A == B
#     A == "" or B = "" which represents zero
#  RESULTS:
#     if A >  B then POSITIVE RESULT
#     if A == B then ZERO   returning empty string
#     if A <  B then NEGATIVE RESULT
#          and the result will be preceeded by negative sign
#
# NOTE: terminology for subtraction
# Difference = Minuend - Subtrahend

def rn_subtraction_full(rn_A, rn_B):
	rn_A = rn_A.upper() # work in upper case
	rn_B = rn_B.upper() # work in upper case
	
	rslt_out = ""
	
	return rslt_out


if __name__ == "__main__":


	print sys.argv
