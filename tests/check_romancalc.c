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
 * pycall__in_str__out_in
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
		
		lcl_pArgs = PyTuple_New(argc);							// create python argument list
		
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

// MARK: Main routine
int
main (void)
{
	int number_failed;
	number_failed = 1;										// start assumption no tests passed

	pyenv_setup("../src", NULL, NULL);						// set up test env and point to py src

	SRunner *sr = srunner_create (romancalc_suite_test_pycall_io ());// build first test

	srunner_run_all (sr, CK_VERBOSE);						// perform the tests
	
	number_failed = srunner_ntests_failed (sr);

	srunner_free (sr);
	pyenv_teardown();										// shut down python interprater
	
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
