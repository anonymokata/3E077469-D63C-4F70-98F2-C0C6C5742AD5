#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <check.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <Python.h>
#include "../src/romancalc.h"

/****** Routines for setting up and tearing Python interperter interface ******/
/**************************************************************************
 * append_Py_src_path
 * append python source path for interperetor WARNING ONLY CALL AFTER Python Interpertor
 * had been initialized by "Py_Initialize" AND ONLY BEFORE Python Interpretor has been
 * shut down by "Py_Finalize"
 *
 * Input
 *    arg_NameSrcPath    Null terminated string representing python source
 *                       where program or test python code wil be called from
 *
 **************************************************************************/
void append_Py_src_path(char* arg_NameSrcPath){
#define TPATH_LEN 256
#define TPATH_FMT_LEN 20
	char tpath[TPATH_LEN+TPATH_FMT_LEN];
	
	if(arg_NameSrcPath && strlen(arg_NameSrcPath))
	{
		if(strlen(arg_NameSrcPath) > TPATH_LEN){
			printf("\n ERROR: file: %s", __FILE__);
			printf("   fuction: %s", __func__);
			printf("   line: %i, ", __LINE__);
			printf(" path name too long: -->%s<--\n",arg_NameSrcPath);
			exit(-1);
		}
		memset(tpath, 0,TPATH_LEN+TPATH_FMT_LEN);
		strcat(tpath, "sys.path.append('");
		strcat(tpath, arg_NameSrcPath);
		strcat(tpath,"')");
		
		PyRun_SimpleString(tpath);
	}
}
// MARK:  python module setup *********
/**************************************************************************
 * pyenv_setup
 *   py environment setup
 *   initialize python interpretor and sets user defined source paths if needed
 * Inputs
 *      arg_NameSrcPath              user defined python source path for ..
 *      arg_NameSrcPath_1            .. test or program source code three ..
 *      arg_NameSrcPath_2            .. in case user has more than one path
 **************************************************************************/
void pyenv_setup(char* arg_NameSrcPath, char* arg_NameSrcPath_1, char* arg_NameSrcPath_2){
	Py_Initialize();										// initialize Interpretor

	/* setup python library paths */
	PyRun_SimpleString("import sys; import ctypes; sys.path.append('.')");
	append_Py_src_path(arg_NameSrcPath);					// add in program source path
	append_Py_src_path(arg_NameSrcPath_1);					// add in program source path
	append_Py_src_path(arg_NameSrcPath_2);					// add in program source path
}

// MARK:  python environmnet teardown *********
/**************************************************************************
 * pyenv_teardown
 *     shut down py interpretor
 *   extra code can be added here if needed
 **************************************************************************/
void pyenv_teardown(void){
	Py_Finalize();
}

// MARK:  python module setup *********
/**************************************************************************
 * pymodule_setup
 *    used to import python source file/module to be tested/used
 * Inputs
 *    arg_modulename                Null terminated string of package module
 * Return
 *    PyObject*                     Python interpretor pointer to module
 **************************************************************************/
PyObject *pymodule_setup(char * arg_modulename){
	PyObject *lcl_pName = NULL, *lcl_pModule = NULL;
	/* setup python module for testing */
	lcl_pName = PyString_FromString(arg_modulename);		// setup python moule name string
	
	/* Error checking of pName left out */
	lcl_pModule = PyImport_Import(lcl_pName);				// import the module
	Py_DECREF(lcl_pName);									// no longer needed, so dump

	return lcl_pModule;
}

// MARK:  python module teardown *********
/**************************************************************************
 * pymodule_teardown
 *   used to release memory related to module/package file of python
 *   source file
 * input
 *   arg_pModule                  python pointer to python module
 **************************************************************************/
void pymodule_teardown(PyObject* arg_pModule){
	Py_DECREF(arg_pModule);
}

/**************************************************************************
 * pyfunc_teardown
 *   used to release memory related to function/method
 * input
 *   arg_pFunc                    python pointer to python function
 **************************************************************************/
void pyfunc_teardown(PyObject *arg_pFunc){
	if(arg_pFunc)
		Py_DECREF(arg_pFunc);
}

/**************************************************************************
 * pyfunc_setup
 *   used to check existance and initialize pointer to method/function
 *   which should exist in the "module" pointed to by arg_pModule
 * Inputs
 *    arg_pModule                 pointer to python module/package/file
 *                                where funcion/method that we wish to
 *                                access/use should exist
 *    arg_funcname                name of function/method we want to sue
 * Outputs
 *    PyObject*                   Python Object pointer to function/method
 *                                we want to use, this is needed by 
 *                                python/C API
 **************************************************************************/
PyObject* pyfunc_setup(PyObject *arg_pModule, char *arg_funcname){
	PyObject *lcl_pFunc = NULL;
	
	// check to see if function exists and is callable
	lcl_pFunc = PyObject_GetAttrString(arg_pModule, arg_funcname);

	if(lcl_pFunc && PyCallable_Check(lcl_pFunc)){
		return lcl_pFunc;									// return new module
	}
	
	// we got here do to error to release any memory
	pyfunc_teardown(lcl_pFunc);								// free unneeded object
	return NULL;											// failure
}

// MARK: python string input string output call
/**************************************************************************
 * pycall__in_str__out_str
 *   used for calling python routines that will return a python string
 *   multi argument call interface to python routines
 *   the output
 * Inputs:
 *   arg_NameMod         module name to be tested
 *   arg_NameFnc         function name to be tested
 *   argc                interger count of arguments being sent to python
 *                       function
 *   ...                 argument list of strings, which will be packed
 *                       into tuple
 *   NOTE: argc and the number of arguments MUST BE EQUAL or this will crash
 *
 * Output:
 *   char*               the output from the python routine
 *                       On failure returns empty string NOT NULL,
 *                  (NOT NULL was choice, subject to improvemental changes)
 **************************************************************************/
char* pycall__in_str__out_str(char* arg_NameMod, char* arg_NameFnc, int argc, ... ){
	va_list args;											// arguments
	char * lcl_arg_str = NULL;								// local argument string from argument list
	int idx_arg;											// argument list indexer
	PyObject *lcl_pMod = NULL;								// module name of function to be tested
	PyObject *lcl_pFnc = NULL;								// function to be tested
	PyObject *lcl_pArgs = NULL;								// python argument list
	PyObject *lcl_pString = NULL;							// local string for creating arg list
	PyObject *lcl_pValue = NULL;							// local returned value from the python routine
	char *cResult_str = NULL;								// C string result from py routine

	lcl_pMod	= pymodule_setup(arg_NameMod);				// point to module being tested
	ck_assert_ptr_ne(lcl_pMod, NULL);						// ensure we got back pointer
	lcl_pFnc	= pyfunc_setup(lcl_pMod, arg_NameFnc);		// point to function being tested

	if(argc > 0){											// build argument list only if
		// we ar sending the args
		// else lcl_pArgs is preloaded with NULL
		
		lcl_pArgs = PyTuple_New(argc);						// create python argument list
		
		va_start( args, argc );								// start argument iterating
		
		for(idx_arg = 0; idx_arg < argc; idx_arg++){
			lcl_arg_str = va_arg( args, char*);				// pull next value and
			if(lcl_arg_str) {
				lcl_pString	= PyString_FromString( lcl_arg_str );// stuff it into the Python arg list
				PyTuple_SetItem(lcl_pArgs, idx_arg, lcl_pString);// add the string to the argument list
			} else {										// null string, can't process
				Py_DECREF(lcl_pArgs);						// dump the arg list
				lcl_pArgs = NULL;							// NULL to signify error, and abortof process
			}
		}
		
		va_end( args );										// close out argument iterating
	}// if(argc > 0)
	
	lcl_pValue = PyObject_CallObject(lcl_pFnc, lcl_pArgs);	// call routine
	
	if(lcl_pValue)
		cResult_str = PyString_AsString(lcl_pValue);		// if bytes value returned
	
	if(lcl_pArgs)
	Py_DECREF(lcl_pArgs);									// don't need python arg list anymore
	
	if(cResult_str == NULL)									// protective programming
		return strdup("");									// avoid
	
	return strdup(cResult_str);
}

/**************************************************************************
 * pycall__in_str__out_int
 *   multi argument call interface to python routines
 *   calls python routine that returns integer value
 *   with arguments, (none to many)
 *   the output
 * Inputs:
 *   arg_NameMod         module name to be tested
 *   arg_NameFnc         function name to be tested
 *   argc                interger count of arguments being sent to python
 *                       function
 *   ...                 argument list of strings, which will be packed
 *                       into tuple
 *   NOTE: argc and the number of arguments MUST BE EQUAL or this will crash
 *
 * Output:
 *   int                 the output from the python routine
 *                       default returned value is 0
 **************************************************************************/
int pycall__in_str__out_int(char* arg_NameMod, char* arg_NameFnc, int argc, ... ){
	va_list args;											// arguments
	char * lcl_arg_str = NULL;								// local argument string from argument list
	int idx_arg;											// argument list indexer
	PyObject *lcl_pMod = NULL;								// module name of function to be tested
	PyObject *lcl_pFnc = NULL;								// function to be tested
	PyObject *lcl_pArgs = NULL;								// python argument list
	PyObject *lcl_pString = NULL;							// local string for creating arg list
	PyObject *lcl_pValue = NULL;							// local returned value from the python routine
	int cResult_int = 0;									// C string result from py routine
	
	lcl_pMod	= pymodule_setup(arg_NameMod);				// point to module being tested
	lcl_pFnc	= pyfunc_setup(lcl_pMod, arg_NameFnc);		// point to function being tested
	
	if(argc > 0){											// build argument list only if
		// we ar sending the args
		// else lcl_pArgs is preloaded with NULL
		
		lcl_pArgs = PyTuple_New(argc);						// create python argument list
		
		va_start( args, argc );								// start argument iterating
		
		for(idx_arg = 0; idx_arg < argc; idx_arg++){
			lcl_arg_str = va_arg( args, char*);				// pull next value and
			if(lcl_arg_str) {
				lcl_pString	= PyString_FromString( lcl_arg_str );// stuff it into the Python arg list
				PyTuple_SetItem(lcl_pArgs, idx_arg, lcl_pString);// add the string to the argument list
			} else {										// null string, can't process
				Py_DECREF(lcl_pArgs);						// dump the arg list
				lcl_pArgs = NULL;							// NULL to signify error, and abortof process
			}
		}
		
		va_end( args );										// close out argument iterating
	}// if(argc > 0)
	
	lcl_pValue = PyObject_CallObject(lcl_pFnc, lcl_pArgs);	// call routine
	
	if(lcl_pValue)
		cResult_int = PyInt_AsLong(lcl_pValue);				// if bytes value returned
	
	if(lcl_pArgs)
		Py_DECREF(lcl_pArgs);								// don't need python arg list anymore
	
	return cResult_int;										// result
}

// MARK: romancalc_suite_test_pycall_io
/**************************************************************************
 * romancalc_suite_test_pycall_io
 * START_TEST (test_pycall__in_str__out_str)
 * START_TEST (test_pycall__in_str__out_int)
 * START_TEST (test_pycall__in_str__out_bool)
 *
 * NOTE: this tests the tester to ensure the python to C api does not get
 * borken
 **************************************************************************/
START_TEST (test_pycall__in_str__out_str)
{
	// setup python module and function interface for this test
	char *lcl_NameMod = "romancalc";
	char *lcl_NameFnc = "TestRoutineCheck__in_str__out_str";
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "SAME"), "SAME");
	ck_assert_str_ne(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "DIFFERENT"), "SAME");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "NOTDIFFERENT"), "NOTDIFFERENT");
}
END_TEST

START_TEST (test_pycall__in_str__out_int)
{
	// setup python module and function interface for this test
	char *lcl_NameMod = "romancalc";
	char *lcl_NameFnc = "TestRoutineCheck__in_str__out_int";
	
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "1"), 1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "2"), 2);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "3"), 3);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "4"), 4);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "5"), 5);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "6"), 6);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "7"), 7);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "32000"), 32000);
	
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "0"), 0);
	
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "-1"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "-2"), -2);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "-3"), -3);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "-4"), -4);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "-5"), -5);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "-6"), -6);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "-7"), -7);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "-32000"), -32000);

}
END_TEST

START_TEST (test_pycall__in_str__out_bool)
{
	// setup python module and function interface for this test
	char *lcl_NameMod = "romancalc";
	char *lcl_NameFnc = "TestRoutineCheck__in_str__out_bool";
	
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "TEST_STRING"), 1);
	ck_assert_int_ne(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "TEST_STRING"), 0);
	
	ck_assert_int_ne(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "NOT_THE_TEST_STRING"), 1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "NOT_THE_TEST_STRING"), 0);
}
END_TEST

Suite *
romancalc_suite_test_pycall_io(void)
{
	Suite *s = suite_create ("\nRoman Calc Suite Test the \"Test Suite\" Python interface");
	
	/*********** test python call string arg with string return *****************/
	TCase *tc_check_test_suite_str_str = tcase_create ("TestPythonCallIO__In_String__Out_String \n");
	tcase_add_test (tc_check_test_suite_str_str, test_pycall__in_str__out_str);
	suite_add_tcase (s, tc_check_test_suite_str_str);

	
	/*********** test python call string arg with int/bool return *****************/
	TCase *tc_check_test_suite_str_int = tcase_create ("TestPythonCallIO__In_String__Out_Int \n");
	tcase_add_test (tc_check_test_suite_str_int, test_pycall__in_str__out_int);
	suite_add_tcase (s, tc_check_test_suite_str_int);
	
	/*********** test python call string arg with bool/int return *****************/
	/* note bool and int return are the same return type but testing of python logic/math are same*/
	TCase *tc_check_test_suite_str_bool = tcase_create ("TestPythonCallIO__In_String__Out_Bool \n");
	tcase_add_test (tc_check_test_suite_str_bool, test_pycall__in_str__out_bool);
	suite_add_tcase (s, tc_check_test_suite_str_bool);
	
	return s;
}

// MARK: romancalc_suite_rn_numeral_validate_bool_digits_single
/**************************************************************************
 * romancalc_suite_rn_numeral_validate_bool_digits_single
 *  testing validation of roman numeral individual digits and whoe digits
 * START_TEST (test_rn_numeral_validate_bool_digits_single_upper_case)
 * START_TEST (test_rn_numeral_validate_bool_digits_single_lower_case)
 * START_TEST (test_rn_numeral_validate_bool_digits_single_error)
 *
 **************************************************************************/
START_TEST (test_rn_numeral_validate_bool_digits_single_upper_case)
{
	// setup python module and function interface for this test
	char *lcl_NameMod = "romancalc";
	char *lcl_NameFnc = "rn_numeral_validate_bool";
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "I"), 1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "V"), 1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "X"), 1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "L"), 1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "C"), 1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "D"), 1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "M"), 1);
}
END_TEST

START_TEST (test_rn_numeral_validate_bool_digits_single_lower_case)
{
	// setup python module and function interface for this test
	char *lcl_NameMod = "romancalc";
	char *lcl_NameFnc = "rn_numeral_validate_bool";
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "i"), 1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "v"), 1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "x"), 1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "l"), 1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "c"), 1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "d"), 1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "m"), 1);
}
END_TEST

START_TEST (test_rn_numeral_validate_bool_digits_single_error)
{
	// ensure that the roman numeral check throws out all invalid characters
	// setup python module and function interface for this test
	char *lcl_NameMod = "romancalc";
	char *lcl_NameFnc = "rn_numeral_validate_bool";
	char lcl_str_rnum[2] = " ";									// string to pass bad chars into check routine
	
	for(lcl_str_rnum[0] = ' '; lcl_str_rnum[0] < 'C'; lcl_str_rnum[0]++){
		ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, lcl_str_rnum), 0);}
	for(lcl_str_rnum[0] = 'E'; lcl_str_rnum[0] < 'I'; lcl_str_rnum[0]++){
		ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, lcl_str_rnum), 0);}
	for(lcl_str_rnum[0] = 'N'; lcl_str_rnum[0] < 'V'; lcl_str_rnum[0]++){
		ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, lcl_str_rnum), 0);}
	for(lcl_str_rnum[0] = 'W'; lcl_str_rnum[0] < 'W'; lcl_str_rnum[0]++){
		ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, lcl_str_rnum), 0);}
	for(lcl_str_rnum[0] = 'Y'; lcl_str_rnum[0] < 'c'; lcl_str_rnum[0]++){
		ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, lcl_str_rnum), 0);}
	for(lcl_str_rnum[0] = 'e'; lcl_str_rnum[0] < 'i'; lcl_str_rnum[0]++){
		ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, lcl_str_rnum), 0);}
	for(lcl_str_rnum[0] = 'n'; lcl_str_rnum[0] < 'v'; lcl_str_rnum[0]++){
		ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, lcl_str_rnum), 0);}
	for(lcl_str_rnum[0] = 'w'; lcl_str_rnum[0] < 'w'; lcl_str_rnum[0]++){
		ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, lcl_str_rnum), 0);}
	for(lcl_str_rnum[0] = 'y'; lcl_str_rnum[0] < 127; lcl_str_rnum[0]++){
		ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, lcl_str_rnum), 0);}
}
END_TEST

Suite *
romancalc_suite_rn_numeral_validate_bool_digits_single(void)
{
	Suite *s = suite_create ("\nRoman Calc Suite Test roman Numeral Validation");
	
	/*********** test python call string arg with string return *****************/
	TCase *tc_check_rn_numeral_validate_bool_single = tcase_create ("Test Roman Numeral Validation \n");
	tcase_add_test (tc_check_rn_numeral_validate_bool_single, test_rn_numeral_validate_bool_digits_single_upper_case);
	tcase_add_test (tc_check_rn_numeral_validate_bool_single, test_rn_numeral_validate_bool_digits_single_lower_case);
	tcase_add_test (tc_check_rn_numeral_validate_bool_single, test_rn_numeral_validate_bool_digits_single_error);
	suite_add_tcase (s, tc_check_rn_numeral_validate_bool_single);
	
	return s;
}


// MARK: romancalc_suite_rn_numeral_validate_bool_digits_multi
/**************************************************************************
 * romancalc_suite_rn_numeral_validate_bool_digits_multi
 *  testing validation of roman numeral multi digits
 * START_TEST (test_rn_numeral_validate_bool_digits_multi)
 * START_TEST (test_rn_numeral_validate_bool_digits_multi_error)
 *
 **************************************************************************/
START_TEST (test_rn_numeral_validate_bool_digits_multi)
{
	// setup python module and function interface for this test
	char *lcl_NameMod = "romancalc";
	char *lcl_NameFnc = "rn_numeral_validate_bool";
	char *str_thou[] = {"", "M", "MM", "MMM"};
	char *str_hund[] = {"", "C", "CC", "CCC", "CD", "D", "DC", "DCC", "DCCC", "CM"};
	char *str_tens[] = {"", "X", "XX", "XXX", "XL", "L", "LX", "LXX", "LXXX", "XC"};
	char *str_ones[] = {"", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX"};
	char lcl_str_rnum[20];
	int thou, hund, tens, ones;								// used to calcualte digits
	
	int cntr;												// counter
	int val;												// working value
	int mod;												// modulus
	int rem;												// remainder
	printf("\n test_rn_numeral_validate_bool_digits_multi \n");
	for(cntr = 1; cntr <= 3999; cntr++){
		if(cntr == 1){			printf("Testing    1 to 1000\n"); }
		else if(cntr == 1001){	printf("Testing 1001 to 2000\n"); }
		else if(cntr == 2001){	printf("Testing 2001 to 3000\n"); }
		else if(cntr == 3001){	printf("Testing 3001 to 3999\n"); }
		
		memset(lcl_str_rnum, 0, 20);		// clear output string
		thou = cntr/1000;
		hund = (cntr % 1000)/100;
		tens = (cntr %  100)/ 10;
		ones = (cntr %   10);

		strcat(lcl_str_rnum, str_thou[ thou ]);	// add in thousands
		strcat(lcl_str_rnum, str_hund[ hund ]);	// add in hundreds
		strcat(lcl_str_rnum, str_tens[ tens ]);	// add in tens
		strcat(lcl_str_rnum, str_ones[ ones ]);	// add in ones

		ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, lcl_str_rnum), 1);
	}
}
END_TEST

START_TEST (test_rn_numeral_validate_bool_digits_multi_error)
{
	// setup python module and function interface for this test
	char *lcl_NameMod = "romancalc";
	char *lcl_NameFnc = "rn_numeral_validate_bool";
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "IIII"), 0);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "VV"), 0);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "XXXX"), 0);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "LL"), 0);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "CCCC"), 0);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "DD"), 0);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "MCCCCV"), 0);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "MCMXCIX"), 1); // 1999 good value
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, "MCM XCIX"), 0); // invalid character
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 1, ""), 0);
}
END_TEST

Suite *
romancalc_suite_rn_numeral_validate_bool_digits_multi(void)
{
	Suite *s = suite_create ("\nRoman Calc Suite Test Multi-Digit Roman Numeral Validation");
	
	/*********** test python call string arg with string return *****************/
	TCase *tc_check_rn_numeral_validate_bool_multi = tcase_create ("Test Multi-Digit Roman Numeral Validation \n");
	tcase_add_test (tc_check_rn_numeral_validate_bool_multi, test_rn_numeral_validate_bool_digits_multi);
	tcase_add_test (tc_check_rn_numeral_validate_bool_multi, test_rn_numeral_validate_bool_digits_multi_error);
	suite_add_tcase (s, tc_check_rn_numeral_validate_bool_multi);
	
	return s;
}

// MARK: romancalc_suite_rn_numeral_digit_unmix
/**************************************************************************
 * romancalc_suite_rn_numeral_digit_unmix
 *  testing unmixing of roman numeral pairs into improper values for
 *  making addition and subtraction of roman numerals easier
 * START_TEST (test_rn_numeral_digit_unmix)
 *
 **************************************************************************/
START_TEST (test_rn_numeral_digit_unmix)
{
	// setup python module and function interface for this test
	char *lcl_NameMod = "romancalc";
	char *lcl_NameFnc = "rn_numeral_digit_unmix";
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "IV"), "IIII");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "IX"), "VIIII");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "XL"), "XXXX");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "XC"), "LXXXX");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "CD"), "CCCC");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "CM"), "DCCCC");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "MCMXCIX"), "MDCCCCLXXXXVIIII"); // 1999 good value
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "CMCDXCXLIXIV"),
					 "DCCCCCCCCLXXXXXXXXVIIIIIIII");
}
END_TEST

Suite *
romancalc_suite_rn_numeral_digit_unmix(void)
{
	Suite *s = suite_create ("\nRoman Calc Suite Test Digit Unmix");
	
	/*********** test unmixing digits *****************/
	/*
		mid process routine to unmix subractions from within the roman numerals
	 this take mixed digit pairs into improper values, making adding and subtracting
	 easier
	 i.e. IV -> IIII, IX -> VIIII, XL -> XXXX, XC -> LXXXX, CD -> CCCC, CM -> DCCCC
	 */
	TCase *tc_check_rn_numeral_digit_unmix = tcase_create ("Test unroll \n");
	tcase_add_test (tc_check_rn_numeral_digit_unmix, test_rn_numeral_digit_unmix);
	suite_add_tcase (s, tc_check_rn_numeral_digit_unmix);
	
	return s;
}


// MARK: romancalc_suite_rn_numeral_digit_sort
/**************************************************************************
 * romancalc_suite_rn_numeral_digit_sort
 *  ensure digits with roman numeral string are sorted in value order
 *  Since M = 1000, D = 500, C = 100, L = 50, X = 10, V = 5, I = 1
 *  then the order from left to right is MDCLVI within any numeral string
 * START_TEST (test_rn_numeral_digit_sort)
 *
 **************************************************************************/
START_TEST (test_rn_numeral_digit_sort)
{
	// setup python module and function interface for this test
	char *lcl_NameMod = "romancalc";
	char *lcl_NameFnc = "rn_numeral_digit_sort";
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "IVXLCDM"), "MDCLXVI");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "MDCLXVIMDCLXVIMDCLXVI"),
					 "MMMDDDCCCLLLXXXVVVIII");
}
END_TEST

Suite *
romancalc_suite_rn_numeral_digit_sort(void)
{
	Suite *s = suite_create ("\nRoman Calc Suite Test Digit Sort According to magnitude");
	
	TCase *tc_check_rn_numeral_digit_sort = tcase_create ("Test Digit Sort \n");
	tcase_add_test (tc_check_rn_numeral_digit_sort, test_rn_numeral_digit_sort);
	suite_add_tcase (s, tc_check_rn_numeral_digit_sort);
	
	return s;
}

// MARK: romancalc_suite_rn_numeral_digit_reduction
/**************************************************************************
 * romancalc_suite_rn_numeral_digit_reduction
 *  reduces multiple digits into the next higher value
 *  these larger number of digits either occur due to addition
 *  or possibly the result of borrowing during subtraction
 *  i.e. 
 * IIIII -> V,
 * VV -> X,
 * XXXXX -> L,
 * LL -> C,
 * CCCCC -> D,
 * DD -> M
 * START_TEST (test_rn_numeral_digit_reduction)
 *
 **************************************************************************/
START_TEST (test_rn_numeral_digit_reduction)
{
	// setup python module and function interface for this test
	char *lcl_NameMod = "romancalc";
	char *lcl_NameFnc = "rn_numeral_digit_reduction";
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "IIIII"), "V");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "VV"   ), "X");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "XXXXX"), "L");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "LL"   ), "C");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "CCCCC"), "D");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "DD"   ), "M");
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "VIIIII"    ), "X");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "IIIIIIIIII"), "X");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "VVVVVVVVVV"), "L");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "XVVVVVVVV"), "L");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "XXVVVVVV"), "L");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "XXXVVVV"), "L");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "LL"), "C");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "MLLLLLLVI"), "MCCCVI");
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "DDCCCCCLLXXXXXVVIIIIII"), "MDCLXVI");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "IIIIIIIIIIIIIIIIIIIIIIIIIIIIII"), "XXX");
}
END_TEST

Suite *
romancalc_suite_rn_numeral_digit_reduction(void)
{
	Suite *s = suite_create ("\nRoman Calc Suite Test Multi-digit numeral reduction");
	
	TCase *tc_check_rn_numeral_digit_reduction = tcase_create ("Test digit numeral reduction \n");
	tcase_add_test (tc_check_rn_numeral_digit_reduction, test_rn_numeral_digit_reduction);
	suite_add_tcase (s, tc_check_rn_numeral_digit_reduction);
	
	return s;
}

// MARK: romancalc_suite_rn_numeral_digit_remix
/**************************************************************************
 * romancalc_suite_rn_numeral_digit_remix
 *  testing remixing of roman numeral pairs into proper values from
 * improper values
 * START_TEST (test_rn_numeral_digit_remix)
 *
 **************************************************************************/
START_TEST (test_rn_numeral_digit_remix)
{
	// setup python module and function interface for this test
	char *lcl_NameMod = "romancalc";
	char *lcl_NameFnc = "rn_numeral_digit_remix";
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "IIII"), "IV");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "VIIII"), "IX");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "XXXX"), "XL");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "LXXXX"), "XC");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "CCCC"), "CD");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "DCCCC"), "CM");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1, "MDCCCCLXXXXVIIII"), "MCMXCIX"); // 1999
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 1,
					 "DCCCCCCCCLXXXXXXXXVIIIIIIII"), "CMCDXCXLIXIV");
}
END_TEST

Suite *
romancalc_suite_rn_numeral_digit_remix(void)
{
	Suite *s = suite_create ("\nRoman Calc Suite Numeral Digit Remix Improper Multi-Digit to Mixed Proper Digit Pair");
	
	/*********** test remixing digits *****************/
	/*
		mid process routine to remix subractions from within the roman numerals
	 this take mixed digit pairs into improper values, making adding and subtracting
	 easier i.e.
	 # DCCCC -> CM,
	 #  CCCC -> CD,
	 # LXXXX -> XC,
	 #  XXXX -> XL,
	 # VIIII -> IX,
	 #  IIII -> IV,
	 */
	TCase *tc_check_rn_numeral_digit_remix = tcase_create ("Test remix of digits \n");
	tcase_add_test (tc_check_rn_numeral_digit_remix, test_rn_numeral_digit_remix);
	suite_add_tcase (s, tc_check_rn_numeral_digit_remix);
	
	return s;
}

// MARK: romancalc_suite_rn_compare
/**************************************************************************
 * romancalc_suite_rn_compare
 * compares 2 roman numerals, A is first value, B is second
 * both A and B MUST be unmixed/unrolled i.e. NO "IV" must be "IIII"
 * A > B = 1
 * A == B = 0
 * A < B = -1
 * START_TEST (test_rn_compare_digit_single)
 * START_TEST (test_rn_compare_digit_multi)
 *
 **************************************************************************/
START_TEST (test_rn_compare_digit_single)
{
	// setup python module and function interface for this test
	char *lcl_NameMod = "romancalc";
	char *lcl_NameFnc = "rn_compare";
	
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "I", "I"),  0);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "I", "V"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "I", "X"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "I", "L"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "I", "C"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "I", "D"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "I", "M"), -1);
	
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "V", "I"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "V", "V"),  0);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "V", "X"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "V", "L"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "V", "C"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "V", "D"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "V", "M"), -1);
	
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "X", "I"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "X", "V"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "X", "X"),  0);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "X", "L"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "X", "C"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "X", "D"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "X", "M"), -1);
	
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "L", "I"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "L", "V"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "L", "X"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "L", "L"),  0);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "L", "C"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "L", "D"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "L", "M"), -1);
	
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "C", "I"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "C", "V"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "C", "X"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "C", "L"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "C", "C"),  0);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "C", "D"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "C", "M"), -1);
	
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "D", "I"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "D", "V"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "D", "X"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "D", "L"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "D", "C"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "D", "D"),  0);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "D", "M"), -1);
	
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "M", "I"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "M", "V"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "M", "X"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "M", "L"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "M", "C"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "M", "D"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "M", "M"),  0);
	
}
END_TEST

START_TEST (test_rn_compare_digit_multi)
{
	// setup python module and function interface for this test
	char *lcl_NameMod = "romancalc";
	char *lcl_NameFnc = "rn_compare";
	
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "II",  "I"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "VV",  "V"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "XX",  "X"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "LL",  "L"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "CC",  "C"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "DD",  "D"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "MM",  "M"),  1);
	
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2,  "I", "II"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2,  "V", "VV"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2,  "X", "XX"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2,  "L", "LL"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2,  "C", "CC"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2,  "D", "DD"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2,  "M", "MM"), -1);
	
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "II",  "I"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "VI",  "V"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "XI",  "X"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "LI",  "L"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "CI",  "C"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "DI",  "D"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "MI",  "M"),  1);
	
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2,  "I", "II"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2,  "V", "VI"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2,  "X", "XI"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2,  "L", "LI"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2,  "C", "CI"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2,  "D", "DI"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2,  "M", "MI"), -1);
	
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "II", "II"),  0);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "VV", "VV"),  0);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "XX", "XX"),  0);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "LL", "LL"),  0);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "CC", "CC"),  0);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "DD", "DD"),  0);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "MM", "MM"),  0);
	
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "VV", "II"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "XX", "VV"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "LL", "XX"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "CC", "LL"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "DD", "CC"),  1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "MM", "DD"),  1);
	
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "II", "VV"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "VV", "XX"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "XX", "LL"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "LL", "CC"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "CC", "DD"), -1);
	ck_assert_int_eq(pycall__in_str__out_int(lcl_NameMod, lcl_NameFnc, 2, "DD", "MM"), -1);
}
END_TEST

Suite *
romancalc_suite_rn_compare(void)
{
	Suite *s = suite_create ("\nRoman Calc Suite Test Unmixed Roman Numeral Comparison");
	
	/*********** compare unmixed strings ****************
	 * both A and B MUST be unmixed/unrolled i.e. NO "IV" must be "IIII"
	 * romancalc_suite_rn_compare
	 * compares 2 roman numerals, A is first value, B is second
	 * A > B = 1
	 * A == B = 0
	 * A < B = -1
	 */
	
	TCase *tc_check_rn_compare_single = tcase_create ("TestPython_Compare_Single_Digits\n");
	tcase_add_test (tc_check_rn_compare_single, test_rn_compare_digit_single);
	suite_add_tcase (s, tc_check_rn_compare_single);
	
	TCase *tc_check_rn_compare_multi = tcase_create ("TestPython_Compare_Multi_Digits\n");
	tcase_add_test (tc_check_rn_compare_multi, test_rn_compare_digit_multi);
	suite_add_tcase (s, tc_check_rn_compare_multi);
	
	return s;
}

// MARK: romancalc_suite_rn_unmixed_subt_LG_SML
/**************************************************************************
 * romancalc_suite_rn_unmixed_subt_LG_SML
 *   mid process method test, this python routine is not final value
 *   test subtraction, routine performs A - B,
 *   where
 *   (1) always A > B
 *   (2) equality check to ensure A>B ALWAYS done before this routine
 *   and (3) A and B are both unmixed digits, by rn_numeral_digit_unmix
 *
 * ensuring that A>B occurs outside of this python routine
 * ensuriong that A and B are unmixed occurrs outside of this python routine
 * RESULTS:
 *   result will always be positive 
 *   result will NEVER BE ZERO
 *   result will be unmixed romannumeral
 # START_TEST (test_rn_unmixed_subt_LG_SML_single_noBorrow)
 * START_TEST (test_rn_unmixed_subt_LG_SML_multi)
 *
 **************************************************************************/
START_TEST (test_rn_unmixed_subt_LG_SML_single_noBorrow)
{
	// setup python module and function interface for this test
	char *lcl_NameMod = "romancalc";
	char *lcl_NameFnc = "rn_unmixed_subt_LG_SML";
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "II",  "I"), "I");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "VI",  "I"), "V");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "XI",  "I"), "X");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "LI",  "I"), "L");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "CI",  "I"), "C");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "DI",  "I"), "D");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MI",  "I"), "M");
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "VV",  "V"), "V");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "XV",  "V"), "X");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "LV",  "V"), "L");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "CV",  "V"), "C");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "DV",  "V"), "D");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MV",  "V"), "M");
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "XX",  "X"), "X");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "LX",  "X"), "L");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "CX",  "X"), "C");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "DX",  "X"), "D");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MX",  "X"), "M");
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "LL",  "L"), "L");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "CL",  "L"), "C");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "DL",  "L"), "D");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "ML",  "L"), "M");
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "CC",  "C"), "C");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "DC",  "C"), "D");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MC",  "C"), "M");
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MM",  "M"), "M");

	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MDCLXVI", "I"), "MDCLXV");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MDCLXVI", "V"), "MDCLXI");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MDCLXVI", "X"), "MDCLVI");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MDCLXVI", "L"), "MDCXVI");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MDCLXVI", "C"), "MDLXVI");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MDCLXVI", "D"), "MCLXVI");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MDCLXVI", "M"), "DCLXVI");

	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MMDDCCLLXXVVII", "I"), "MMDDCCLLXXVVI");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MMDDCCLLXXVVII", "V"), "MMDDCCLLXXVII");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MMDDCCLLXXVVII", "X"), "MMDDCCLLXVVII");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MMDDCCLLXXVVII", "L"), "MMDDCCLXXVVII");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MMDDCCLLXXVVII", "C"), "MMDDCLLXXVVII");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MMDDCCLLXXVVII", "D"), "MMDCCLLXXVVII");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MMDDCCLLXXVVII", "M"), "MDDCCLLXXVVII");

	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MMMDDDCCCLLLXXXVVVIII", "I"),
					 "MMMDDDCCCLLLXXXVVVII");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MMMDDDCCCLLLXXXVVVIII", "V"),
					 "MMMDDDCCCLLLXXXVVIII");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MMMDDDCCCLLLXXXVVVIII", "X"),
					 "MMMDDDCCCLLLXXVVVIII");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MMMDDDCCCLLLXXXVVVIII", "L"),
					 "MMMDDDCCCLLXXXVVVIII");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MMMDDDCCCLLLXXXVVVIII", "C"),
					 "MMMDDDCCLLLXXXVVVIII");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MMMDDDCCCLLLXXXVVVIII", "D"),
					 "MMMDDCCCLLLXXXVVVIII");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "MMMDDDCCCLLLXXXVVVIII", "M"),
					 "MMDDDCCCLLLXXXVVVIII");

}
END_TEST

Suite *
romancalc_suite_rn_unmixed_subt_LG_SML(void)
{
	Suite *s = suite_create ("\nRoman Calc Suite Test Unmixed Subtraction Larger Minus SMaller");
	
	/**************************************************************************
	 * romancalc_suite_rn_unmixed_subt_LG_SML
	 *   mid process method test, this python routine is not final value
	 *   test subtraction, routine performs A - B,
	 *   where
	 *   (1) always A > B
	 *   (2) equality check to ensure A>B ALWAYS done before this routine
	 *   and (3) A and B are both unmixed digits, by rn_numeral_digit_unmix
	 *
	 * ensuring that A>B occurs outside of this python routine
	 * ensuriong that A and B are unmixed occurrs outside of this python routine
	 * RESULTS:
	 *   result will always be positive
	 *   result will NEVER BE ZERO
	 *   result will be unmixed romannumeral
	 * START_TEST (test_rn_unmixed_subt_LG_SML_single)
	 * START_TEST (test_rn_unmixed_subt_LG_SML_multi)
	 *
	 **************************************************************************/
	
	TCase *tc_check_rn_unmixed_subt_LG_SML_single = tcase_create ("TestPython_Subtraction_single\n");
	tcase_add_test (tc_check_rn_unmixed_subt_LG_SML_single, test_rn_unmixed_subt_LG_SML_single_noBorrow);
	suite_add_tcase (s, tc_check_rn_unmixed_subt_LG_SML_single);
	
	return s;
}

// MARK: @micah
// MARK: romancalc_suite_rn_unmixed_borrow
/**************************************************************************
 * romancalc_suite_rn_unmixed_borrow
 *   mid process calculate roman numeral borrwed value,
 *   where that A can be subtracted from
 *   (1) always A > B
 *   (2) equality check to ensure A>B ALWAYS done before this routine
 *   and (3) A and B are both unmixed digits, by rn_numeral_digit_unmix
 *
 * START_TEST (test_rn_unmixed_borrow_Simple)
 *
 **************************************************************************/
START_TEST (test_rn_unmixed_borrow_Simple)
{
	// setup python module and function interface for this test
	char *lcl_NameMod = "romancalc";
	char *lcl_NameFnc = "rn_unmixed_borrow";
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "V",  "I"), "IIIII");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "X",  "V"), "VV"   );
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "L",  "X"), "XXXXX");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "C",  "L"), "LL");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "D",  "C"), "CCCCC");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "M",  "D"), "DD");
	
}
END_TEST

START_TEST (test_rn_unmixed_borrow_Complex_single_single)
{
	// setup python module and function interface for this test
	char *lcl_NameMod = "romancalc";
	char *lcl_NameFnc = "rn_unmixed_borrow";
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "X",  "I"), "VIIIII");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "L",  "I"), "XXXXVIIIII");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "C",  "I"), "LXXXXVIIIII");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "D",  "I"), "CCCCLXXXXVIIIII");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "M",  "I"), "DCCCCLXXXXVIIIII");
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "L",  "V"), "XXXXVV");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "C",  "V"), "LXXXXVV");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "D",  "V"), "CCCCLXXXXVV");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "M",  "V"), "DCCCCLXXXXVV");
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "C",  "X"), "LXXXXX");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "D",  "X"), "CCCCLXXXXX");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "M",  "X"), "DCCCCLXXXXX");
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "D",  "L"), "CCCCLL");
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "M",  "L"), "DCCCCLL");
	
	ck_assert_str_eq(pycall__in_str__out_str(lcl_NameMod, lcl_NameFnc, 2, "M",  "C"), "DCCCCC");
}
END_TEST

Suite *
romancalc_suite_rn_unmixed_borrow(void)
{
	Suite *s = suite_create ("\nRoman Calc Suite Test Unmixed Subtraction Larger Minus SMaller");
	
	/**************************************************************************
	 * romancalc_suite_rn_unmixed_borrow
	 *   mid process calculate roman numeral borrwed value,
	 *   where
	 *   (1) always A > B
	 *   (2) equality check to ensure A>B ALWAYS done before this routine
	 *   and (3) A and B are both unmixed digits, by rn_numeral_digit_unmix
	 *
	 * START_TEST (test_rn_unmixed_borrow_Simple)
	 *
	 **************************************************************************/
	
	TCase *tc_check_rn_unmixed_borrow = tcase_create ("TestPython_borow calculation\n");
	tcase_add_test (tc_check_rn_unmixed_borrow, test_rn_unmixed_borrow_Simple);
	tcase_add_test (tc_check_rn_unmixed_borrow, test_rn_unmixed_borrow_Complex_single_single);
	suite_add_tcase (s, tc_check_rn_unmixed_borrow);
	
	return s;
}


// MARK: Main routine
int
main (void)
{
	int number_failed;
	number_failed = 1;										// start assumption no tests passed

	pyenv_setup("../src", NULL, NULL);						// set up test env and point to py src

	SRunner *sr = srunner_create (romancalc_suite_test_pycall_io ());	// validate test routines
																		// test the tester
	srunner_add_suite(sr, romancalc_suite_rn_numeral_validate_bool_digits_single());	// test validate roman numerals
	srunner_add_suite(sr, romancalc_suite_rn_numeral_validate_bool_digits_multi());	// test validate roman numerals
	srunner_add_suite(sr, romancalc_suite_rn_numeral_digit_unmix());	//unmix mid-numeral digits
	srunner_add_suite(sr, romancalc_suite_rn_numeral_digit_sort());	// digit magnitude sort
	srunner_add_suite(sr, romancalc_suite_rn_numeral_digit_reduction());	// digit reduction
	srunner_add_suite(sr, romancalc_suite_rn_numeral_digit_remix());	//remix mid-numeral digits
	srunner_add_suite(sr, romancalc_suite_rn_compare());	// roman numeral comparison
	srunner_add_suite(sr, romancalc_suite_rn_unmixed_subt_LG_SML());	// subtraction unmixed larger - smaller
	srunner_add_suite(sr, romancalc_suite_rn_unmixed_borrow());	// check to see if can figure out borrow

	srunner_run_all (sr, CK_VERBOSE);						// perform the tests
	
	number_failed = srunner_ntests_failed (sr);

	srunner_free (sr);
	pyenv_teardown();										// shut down python interprater
	
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
