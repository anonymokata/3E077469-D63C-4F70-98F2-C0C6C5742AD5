#!/usr/bin/python

import sys
import re
import os
import time
import threading
import lcm
from exlcm import rn_packet_t

rn_digits = ['M','D','C','L','X','V','I']

rn_lcm_ch_to_srv = "ROMAN_CALC_TO_SRV"		# channel receiveing data from client
rn_lcm_ch_to_cli = "ROMAN_CALC_TO_CLI"		# channel sending data back to client
rn_server_done = 0							#used to flag when server needs to exit
#rn_lcm_provider = "udpm://239.255.255.255:7667"
rn_lcm_provider = ""

glbl_client_pkt	= ("",-1)						# create global client packet
glbl_client_rxed = 0							# used signal when packet received



#     lcm_globals_return
#        returns lcm globals for use by external programs
#        mainly fo ruse by
#        if a value is set to none, then make into empty string
#        to avoid hassles in passing back and forth values
def lcm_globals_return():
	lcl_glbls = (rn_lcm_ch_to_srv, rn_lcm_ch_to_cli, rn_lcm_provider)
	if lcl_glbls[0] == None
		lcl_glbls[0] = ""

	if lcl_glbls[1] == None
		lcl_glbls[1] = ""

	if lcl_glbls[2] == None
		lcl_glbls[2] = ""

	return lcl_glbls

#     lcm_globals_set
#         c call back routine to set the global values
#         values set to empty string indicate that the value was
#         desired to set as None or NULL, i.e. not used
def lcm_globals_set(arg_glbl_tpls):
	global rn_lcm_ch_to_srv						# ensure this is not a local var
	global rn_lcm_ch_to_cli						# ensure this is not a local var
	global rn_lcm_provider						# ensure this is not a local var
	
	if arg_glbl_tpls[0] == ""					# if value was empty stirng
		rn_lcm_ch_to_srv = None					# then we actually wanted a NULL
	else
	rn_lcm_ch_to_srv = arg_glbl_tpls[0]			# set global values

	if arg_glbl_tpls[0] == ""					# if value was empty stirng
		rn_lcm_ch_to_srv = None					# then we actually wanted a NULL
	else
	rn_lcm_ch_to_cli = arg_glbl_tpls[1]			# set global values
	
	if arg_glbl_tpls[0] == ""					# if value was empty stirng
		rn_lcm_ch_to_srv = None					# then we actually wanted a NULL
	else
	rn_lcm_provider	 = arg_glbl_tpls[2]			# set global values

	return lcm_globals_return()					# respond back with values for checking

#     lcm_opener
#        used to set up the communications channel
#        common code avoid code duplication
def lcm_opener(arg_lcm_provider):
	if (arg_lcm_provider == "") or (arg_lcm_provider == None):
		lcl_lcm = lcm.LCM()
	else:
		lcl_lcm = lcm.LCM(arg_lcm_provider)
	return lcl_lcm

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
			rn_A = rn_unmixed_borrow(rn_A, rn_B[idx_b])# calculate a Minuend that is borrowable
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
	
	rn_A = rn_numeral_digit_unmix(rn_A)					# unroll for simplified math
	rn_B = rn_numeral_digit_unmix(rn_B)					# unroll for simplified math
	
	rslt_cmp = rn_unmixed_compare(rn_A, rn_B)			# see if result is zero pos or neg
	if rslt_cmp == 0:									# if values equal the result will be zero
		rslt_out = ""									# or empty string (re-inforce this concept
	else:
		if rslt_cmp > 0:								# if A>B result will be positive
			rslt_out = rn_unmixed_subt_LG_SML(rn_A, rn_B)# perform subtraction
		else:											# else A< B result will be negative
			rslt_out = '-' + rn_unmixed_subt_LG_SML(rn_B, rn_A)# perform subtraction

		rslt_out = rn_numeral_digit_reduction(rslt_out) # reduce any multi digits to next higher digit
		rslt_out = rn_numeral_digit_remix(rslt_out)     # mix back in any proper numerals IIII->IV etc

	return rslt_out

#   rn_addition_full
#     process of roman numeral addition
#     1) unmix mid value subtractions to improper values
#     2) concatenate 2 values
#     3) sort/group for simplification to higher digits (reduction)
#     4) performsimplification to higher digits (reduction)
#     5) remix any necessary value from improper to proper ie IIII-> IV, CCCC->CD etc.
#     6) return value
def rn_addition_full(rn_A, rn_B):
	rn_A = rn_A.upper() # work in upper case
	rn_B = rn_B.upper() # work in upper case

	rslt_out = ""
	rn_A = rn_numeral_digit_unmix(rn_A)					# unroll for simplified math
	rn_B = rn_numeral_digit_unmix(rn_B)					# unroll for simplified math
	
	rslt_out = rn_A + rn_B								# combine input digits, simple form of add
	
	rslt_out = rn_numeral_digit_sort(rslt_out)			# sort digits for addition,
	rslt_out = rn_numeral_digit_reduction(rslt_out)		# reduce any multi digits to next higher digit
	rslt_out = rn_numeral_digit_remix(rslt_out)			# mix back in any proper numerals IIII->IV etc
	
	return rslt_out

#                            0 = no errors/warnings
#                           information value
#                not used no point           -1 = result is zero
#                           error value
#                           -2 = multiple operators
#                           -3 = invalid expression
#                                   (characters not allowed either not numerals
#                                      not allowed operators '+' or '-')
#                           -4 = invalid values left side
#                           -5 = invalid values right side
def rn_exp_error_strings(arg_err_num):
	if arg_err_num == 0:
		return "No Errors/Warnings"
	elif arg_err_num == -1:
		return "-I   Result is Zero"
	elif arg_err_num == -2:
		return "-II  Multiple Operators"
	elif arg_err_num == -3:
		return "-III  Invalid Expression"
	elif arg_err_num == -4:
		return "-IV  Invalid Value on Left Side of Operator"
	elif arg_err_num == -5:
		return "-V  Invalid Value on Right Side of Operator"
	elif arg_err_num == -99:
		return "Calculation Thread/Server Response TimeOut"
	return "General Error"

def rn_exp_error_display(arg_err_num):
	return "<ERROR: CODE "+rn_exp_error_strings(arg_err_num)+" >"

#     rn_expression_process
#          this adds or subtracts 2 roman numerals
#
#          process the mathemeatical espression
#          this is a single operator expression only 1 operator per expression
#               either a '+' or '-'
#          report back simple errors
#               excessive opperators (more than 1)
#               proper roman numerals only checked
#               empty input report back as zero
#          expression    value1 +- value2
#               either no or 1 operator, if no operator the numeral is reported
#                  back, if the expression is a proper roman numeral or error
#                  is reported
#               if either side of a correct operator is blank it
#                    is treated as zero and the expression is still calculated
#                    giving  the mathematical result
#          results will be a tuple,
#                    index 0 of the tuple will be the ascii result representation
#                        index 0 is type string value and will never be null
#                    index 1 of the tuple will be the integer error/result code
#                        index 1 is type int and will be one of the following
#                            0 = no errors/warnings
#                           information value
#                not used no point           -1 = result is zero
#                           error value
#                           -2 = multiple operators
#                           -3 = invalid expression
#                                   (characters not allowed either not numerals
#                                      not allowed operators '+' or '-')
#                           -4 = invalid values left side
#                           -5 = invalid values right side
#               positive results have no preceding sign
#               negative results will be preceeded by a '-'
#               for addition results will be always positive
#               for subtraction the result is calculated
#                    value1 >  value2    positive result
#                    value2 == value2    empty result (zero value)
#                    value1 <  value2    negative result
#
def rn_process_expression(rn_exp):
	rn_exp = rn_exp.upper()						# work in upper case
	rn_exp = rn_exp.strip()						# trim leading and trailing white space
	
	rn_rslt_str = ""							# init result strings
	rn_rslt_err = 0								# init error value
	idx_op = 0									# index of the operator
	cnt_ops_plus = 0							# init counter for number of plus operators
	cnt_ops_minus = 0							# init counter for number of minus operator
	rn_val_left  = ""
	rn_val_right = ""
	
	cnt_ops_plus  = rn_exp.count('+')
	cnt_ops_minus = rn_exp.count('-')
	cnt_ops = cnt_ops_minus + cnt_ops_plus		# get total count
	if cnt_ops > 1:								# if too many operators
		rn_rslt_str = ""						# empty the response
		rn_rslt_err = -2						# set the error appropriately
	elif cnt_ops == 0:							# if no operators, then maybe just hex value
		if not(rn_numeral_validate_bool(rn_exp)):# no ops then may be individual value
			rn_rslt_str = ""					# empty the response
			rn_rslt_err = -3					# so indicate invalid expression
		else:									# no operator valid expression
			rn_rslt_str = rn_exp				# then it's just a Roman Numeral
			rn_rslt_err = 0						# just a value
	else:										# single operator
		if cnt_ops_plus:						# if we are adding
			idx_op = rn_exp.find('+')			# find where to split expression
		else:									# else we're subtracting
			idx_op = rn_exp.find('-')			# find where to split expression
		if idx_op > 0:							# if operator is not at beginning
			rn_val_left = rn_exp[:idx_op]		# then there is a left value
			rn_val_left = rn_val_left.strip()	# trim trailing/leading whitespace
		if idx_op < (len(rn_exp)-1):			# if operator is not at the end
			rn_val_right = rn_exp[idx_op+1:]	# then there is a right
			rn_val_right = rn_val_right.strip()	# trim trailing/leading whitespace
		if   len(rn_val_left ) and not(rn_numeral_validate_bool(rn_val_left)): # if left value is invalid
			rn_rslt_str = ""					# empty the response
			rn_rslt_err = -4					# so indicate invalid expression
		elif len(rn_val_right) and not(rn_numeral_validate_bool(rn_val_right)): # if left value
			rn_rslt_str = ""					# empty the response
			rn_rslt_err = -5					# so indicate invalid expression
		else:									# good expression so process it
			if cnt_ops_plus:					# if adding
												# add the values
				rn_rslt_str = rn_addition_full(rn_val_left, rn_val_right)
			else:								# else performing subtraction
												# subtract the value
				rn_rslt_str = rn_subtraction_full(rn_val_left, rn_val_right)

	if rn_rslt_err != 0:						# if there is an err then
		rn_rslt_str = rn_exp_error_display(rn_rslt_err) # then build error response message

	rn_tuple_out = (rn_rslt_str, rn_rslt_err)
	return rn_tuple_out

#   rn_tx_lcm_packet
#        packs data into lcm communicaitons packet and publishes it to listeners
#
#      arg_ch is the channel to publish the packet to
#      arg_exp_n_rslt      if going to server then this holds the expression
#                          if response from server then this is the result
#      arg_cmd_n_err       if going to server then this is command for server
#                             0 = calculate expression result
#                             1 = kill server
#                          if response from server then result code are that
#                              is rn_rslt_err tuple from rn_process_expression
def rn_lcm_tx_packet(arg_ch, arg_exp_n_rslt, arg_cmd_n_err):
	rslt_err = 0								# reserve right to return err
	
	pkt = rn_packet_t()							# create packet
	pkt.timestamp = 0							# time stamp it
	pkt.cmd_n_err  = arg_cmd_n_err				# set command or error
	pkt.exp_n_rslt = arg_exp_n_rslt				# set the data portion
	
	lc = lcm_opener(rn_lcm_provider)				# lcm code
	lc.publish(arg_ch, pkt.encode())
	return rslt_err

# rn_lcm_server_handler
## server handler for packet
def rn_lcm_server_handler(channel, data):
	global rn_server_done						# define global variable
	srvr_pkt = rn_packet_t.decode(data)
	if srvr_pkt.cmd_n_err == 0:					# if command to calculate result
												# send data to packet
		rslt_tpl = rn_process_expression(srvr_pkt.exp_n_rslt)
												# send back result
		# return calculation and error code
		rn_lcm_tx_packet(rn_lcm_ch_to_cli, rslt_tpl[0], rslt_tpl[1])
	else:
		rn_server_done = 1						# kill the server
	return

#	rn_server
#		is the server process runs until exit conditions are met
#           exit conditions are
#                --serverdown command
#                arg_duration  has expired
#                arg_responses has been met
#           arg_duration  duration in seconds to run
#                0    run in server mode
#                >0   exit after given number of seconds
#           arg_responses max responses to give before exiting
#                0    run in server mode
#                >0   exit after given count  of responses
#			start roman calc server process
#			process will exit when the server receives a kill command
#
#			commands received through communication channel
#				--serverdown
def rn_server(arg_duration = 1, arg_responses = 1):
	global rn_server_done						# define global variable
	time_end = time.time() + arg_duration
	rn_server_done = 0							# signal server exit if done

	lcl_response_cnt = arg_responses			# loop this number of times

	lc = lcm_opener(rn_lcm_provider)			# lcm code
												# setup our channel receiver
	rn_svr_subscription = lc.subscribe(rn_lcm_ch_to_srv, rn_lcm_server_handler)
	while rn_server_done == 0:					# loop until ready to exit
		lcl_to = lc.handle_timeout(500)			# listen but check every 500 seconds
												# for abort command or loop timeout
		if lcl_to > 0:							# if not timed out, but pkt rxed
			lcl_response_cnt -= 1				# received one more

		if ( ( ( arg_duration  > 0 ) and ( time.time() > time_end ) ) or
			( ( arg_responses > 0 ) and ( lcl_response_cnt == 0   ) ) ):
			rn_server_done = 1					# time to end the loop

	lc.unsubscribe(rn_svr_subscription)
	return 0


# rn_lcm_client_handler
# server handler for packet
def rn_lcm_client_handler(channel, data):
	global glbl_client_pkt						#define global variable
	global glbl_client_rxed						#define global variable
	lcl_client_pkt = rn_packet_t.decode(data)
	glbl_client_pkt = (lcl_client_pkt.exp_n_rslt, lcl_client_pkt.cmd_n_err)
	glbl_client_rxed = 1						#signal we received packet

	return

def rn_client(arg_exp):
	global glbl_client_pkt						#define global variable
	global glbl_client_rxed						#define global variable
	rlst_to = 0									# handler time out rsult
	glbl_client_pkt	= ("",-1)					# create global client packet
	glbl_client_rxed = 0						# reset rxed flag


	lc = lcm_opener(rn_lcm_provider)			# lcm code
	# setup our channel receiver
	rn_cli_subscription = lc.subscribe(rn_lcm_ch_to_cli, rn_lcm_client_handler)

	rn_lcm_tx_packet(rn_lcm_ch_to_srv, arg_exp, 0)

	rslt_to = lc.handle_timeout(8000)			# listen but check every 500 mseconds
	lc.unsubscribe(rn_cli_subscription)
	
	if rslt_to > 0:								# if server/thread did not time out
		return glbl_client_pkt[0]				# return the actual result
	return rn_exp_error_display( -99 )			# server thread timed out

#		rn_test_coms_using_threads
#		threaded comunications tester
def rn_test_coms_using_threads (arg_exp):
	rn_rslt_str = ""
	# start the server thread to receive 1 message or 1 second
	rn_thread = threading.Thread(target=rn_server, args=(10,1))
	rn_thread.start()							# start the thread
	rn_rslt_str = rn_client(arg_exp)			# call cleint requesting result
	rn_thread.join()							# rejoin the thread
	return rn_rslt_str

def rn_help_display():
	sp_0 = ""									# no space,used to make code "line up" visually
	sp_1 = "    "								# space for 'tabbing' values over
	sp_2 = sp_1+sp_1							# tab doubled
	sp_3 = sp_1+sp_1+sp_1						# tab trippled
	sp_4 = sp_1+sp_1+sp_1+sp_1					# tab quadrupled
	Op_nm1 = "ValueI"							# name of first value
	Op_nm2 = "ValueII"							# name of second value
	help_str = "\n"
	help_str += sp_0+"Help\n"
	help_str += sp_1+"Command syntax '"+os.path.basename(__file__)+"' "+Op_nm1+" Operator "+Op_nm2+"\n"

	help_str += sp_2+"A valid expression may either be:\n"
	help_str += sp_3+"No Operator and a Single Roman Numeral, will return that Numeral\n"
	help_str += sp_3+"A Numeral Followed Either Operator, will Return That Numeral\n"
	help_str += sp_4+"i.e. Numeral plus/minus nothing\n"
	help_str += sp_3+"Plus  Operator Followed by a Numeral, Returns That Numeral, Nothing Plus Numeral\n"
	help_str += sp_3+"Minus Operator Followed by a Numeral, Returns Negative of that Numeral\n"
	help_str += sp_3+"i.e. Nothing(Zero) Minus a Numeral, will Return a Negative Value, '-X' is negative X\n"
	
	help_str += sp_2+Op_nm1+" + "+Op_nm2+" will Return the Result of the Addition of the Two values \n"
	
	help_str += sp_2+Op_nm1+" - "+Op_nm2+" Returns Result of the Subtraction of "+Op_nm1+" - "+Op_nm2+" \n"
	help_str += sp_3+"if "+Op_nm1+"   >    "+Op_nm2+" Returns a Positive Result, a Numeral Preceded by No Sign\n"
	help_str += sp_3+"if "+Op_nm1+"   <    "+Op_nm2+" Returns a Negative Result, a Numeral Preceded by a minus Sign\n"
	help_str += sp_3+"if "+Op_nm1+" equals "+Op_nm2+" Returns '' (Nothing which is euqivalent of a Roman Numeral Zero)\n"
	
	help_str += sp_2+"Operators Allowed: either '+' or '-'\n"
	help_str += sp_2+""+Op_nm1+" and "+Op_nm2+": Valid Roman Numerals\n"
	help_str += sp_3+"Consisting of Upper or Lower Case Digits M,D,C,L,X,V,I only.\n"
	help_str += sp_2+"Numerals cannot contain spaces between any II digits.\n"
	help_str += sp_3+"Numerals cannot contain characters other than valid Roman Digits.\n"
	help_str += sp_1+"<ERROR: CODE -II Multiple Operators >\n"
	help_str += sp_2+"More than I mathematical operaotr within\n"
	help_str += sp_2+"the expression is not allowed\n"
	help_str += sp_2+"i.e. 'X++X' 'X--X' 'X+-X' 'X-+X'\n"
	help_str += sp_2+"     '+X+X' 'X+X+' '-X-X' 'X-X-'\n"
	help_str += sp_2+"     '-X+X' 'X-X+' '+X-X' 'X+X-'\n"
	help_str += sp_2+"      '++X'  'X++'  '--X'  'X--'\n"
	help_str += sp_2+"      '+-X'  'X+-'  '-+X'  'X-+'\n"
	help_str += sp_1+"<ERROR: CODE -III Invalid Expression >\n"
	help_str += sp_2+"Expression with No Mathematical Operator and Non-Numeral Digits with the Value\n"
	help_str += sp_1+"ERROR CODES -IV and -V are returned when invalid characters\n"
	help_str += sp_2+"are found within a numeral on either side of the mathemical\n"
	help_str += sp_2+"operator. See above for definition of Valid Numeral Digits\n"
	help_str += sp_1+"<ERROR: CODE -IV Invalid Value on Left Side of Operator >\n"
	help_str += sp_1+"<ERROR: CODE -V Invalid Value on Right Side of Operator >\n"
	help_str += sp_1+"\n"
	help_str += sp_1+"NOTE: Spaces or Tabs ar allowed at the beginning and end of the epxression\n"
	help_str += sp_1+"and directly before and after the operator.\n"
	help_str += sp_1+"i.e. ' X' 'X' 'X +X' 'X + X' ' X+X' 'X+X '\n"
	help_str += sp_1+"\n"
	help_str += sp_1+"FOR HELP: Empty input will print this,\n"
	help_str += sp_2+" or "+Op_nm1+" can be the following'-?'  or '--help'\n"
	return help_str								# give the help string


def main():
	import sys
	argc = len(sys.argv)						# get number of arguments
	arg_str = ""								# used to build arg list

	if argc == 1:								# no input so display help
		return rn_help_display()
	if argc > 1:								# only proces if any input
		if (sys.argv[1] == "-?") or (sys.argv[0] == "--help"):
			return rn_help_display()
		else:										# else process the value
			arg_str = sys.argv[1]					# use first argument
			for arg_idx in sys.argv[2:]:			# iterate through rest if any args
				arg_str = arg_str + ' ' + arg_idx	#tack on each additional argument
	# process arguments using lcm modules and send out the result
			return rn_test_coms_using_threads(arg_str)
	return rn_help_display()

if __name__ == "__main__":
	exit(main())
