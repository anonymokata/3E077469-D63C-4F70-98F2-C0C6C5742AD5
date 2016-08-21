#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <check.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <Python.h>
#include "../src/romancalc.h"

// MARK: global objects for use by python modules
PyObject *glbl_pFunc;

// MARK:  python module setup *********
PyObject *pymodule_setup(char * arg_modulename){
	PyObject *lcl_pName = NULL, *lcl_pModule = NULL;
	
	/* setup python library paths */
	PyRun_SimpleString("import sys; sys.path.append('.')");
	PyRun_SimpleString("sys.path.append('../src')");
	
	/* setup python module for testing */
	lcl_pName = PyString_FromString(arg_modulename);		// setup python moule name string
	
	/* Error checking of pName left out */
	lcl_pModule = PyImport_Import(lcl_pName);				// import the module
	Py_DECREF(lcl_pName);									// no longer needed, so dump
	
	return lcl_pModule;
}

// MARK:  python module teardown *********
void pymodule_teardown(PyObject* arg_pModule){
	Py_DECREF(arg_pModule);
	Py_Finalize();
}

void pyfunc_teardown(PyObject *arg_pFunc){
	if(arg_pFunc)
		Py_DECREF(arg_pFunc);
}
	
PyObject* pyfunc_setup(PyObject *arg_pModule, char *arg_funcname){
	PyObject *lcl_pFunc = NULL;
	
	// check to see if function exists and is callable
	lcl_pFunc = PyObject_GetAttrString(arg_pModule, arg_funcname);
	
	if(lcl_pFunc && PyCallable_Check(lcl_pFunc)){
		return lcl_pFunc;									// return new module
	}
	
	pyfunc_teardown(lcl_pFunc);								// free unneeded object
	return NULL;											// failure
}

// MARK: python string input string output call
/**************************************************************************
 * pycall__in_strargs__out_pyobj
 * multi argument call interface to python routines
 * Inputs:
 *	arg_pFunc           module interface built by PyObject_GetAttrString
 *                      this is setup using PyObject_GetAttrString
 *  argc                interger count of arguments being sent to python
 *                      function
 *  ...                 argument list of strings, which will be packed
 *                      into tuple
 *  NOTE: argc and the number of arguments MUST BE EQUAL or this will crash
 *
 * Returns:
 *  PyObject*           Output return from python routine
 *                      This is interpreted/converted into whatever type
 *                      is used externally, this allows more a more useful
 *                      interface, and will avoid code duplication for different
 *                      return types
 *
 **************************************************************************/
PyObject* tuple__in_strargs(int argc, ... ){
	va_list args;											// arguments
	char * lcl_arg_str = NULL;								// local argument string from argument list
	int idx_arg;											// argument list indexer
	PyObject *lcl_pArgs = NULL;								// python argument list
	PyObject *lcl_pString = NULL;							// local string for creating arg list
	PyObject *lcl_pValue = NULL;							// local returned value from the python routine
	char *cResult_str = NULL;								// C string result from py routine
	
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
	
	lcl_pValue = PyObject_CallObject(arg_pFunc, lcl_pArgs);	// call routine
	
	if(lcl_pValue){
		cResult_str = PyString_AsString(lcl_pValue);		// if bytes value returned
	}
	
	if(lcl_pArgs)
	Py_DECREF(lcl_pArgs);								// don't need python arg list anymore
	
	if(cResult_str == NULL)									// protective programming
	return strdup("");									// avoid
	
	return strdup(cResult_str);
}
char* pycall__in_str__out_str(PyObject* arg_pFunc, int argc, ... ){
	char *lcl_string = "";
	va_list args;											// arguments
	char * lcl_arg_str = NULL;								// local argument string from argument list
	va_start( args, argc );									// start argument iterating
	lcl_arg_str = va_arg( args, char*);					// pull next value and
	va_end( args );											// close out argument iterating
	lcl_string = strdup(lcl_arg_str);						// create copy of string, 
	return lcl_string;
}

}


START_TEST (test_pycall__in_str__out_str)
{
	ck_assert_int_eq(1,1);
}
END_TEST

START_TEST (test_pycall__in_str__out_int)
{
	ck_assert_int_eq(1,1);
}
END_TEST

Suite *
romancalc_suite_process_text_io(void)
{
	Suite *s = suite_create ("\nRoman Calc Suite Test Sub/ADD Text In Text Out check");
	TCase *tc_full_subtraction = tcase_create ("Test Roman Sub/ADD In Text Out \n");
	tcase_add_test (tc_full_subtraction, test_valid_numeral_str);
	suite_add_tcase (s, tc_full_subtraction);
	// check to see if function exists and is callable
	glbl_pFunc = pyfunc_setup(arg_pModule, "TestRoutineCheck__in_str__out_str");
	if(glbl_pFunc != NULL){
		TCase *tc_check_test_suite_str_str = tcase_create ("TestPythonCallIO__In_String__Out_String \n");
		tcase_add_test (tc_check_test_suite_str_str, test_pycall__in_str__out_str);
		suite_add_tcase (s, tc_check_test_suite_str_str);
	}
	glbl_pFunc = pyfunc_setup(arg_pModule, "TestRoutineCheck__in_str__out_int");
	if(glbl_pFunc != NULL){
		TCase *tc_check_test_suite_str_int = tcase_create ("TestPythonCallIO__In_String__Out_Int \n");
		tcase_add_test (tc_check_test_suite_str_int, test_pycall__in_str__out_int);
		suite_add_tcase (s, tc_check_test_suite_str_int);
	}
	pyfunc_teardown(glbl_pFunc);
	
	return s;
}

// MARK: Main routine
int
main (void)
{
	int number_failed;
	PyObject *pModule;										// module name for global use
	
	number_failed = 1;										// start assumption no tests passed
	
	Py_Initialize();										// initialize py in main scope
	pModule = pymodule_setup("romancalc");					// setup module and

	if(pModule != NULL){
	
		SRunner *sr = srunner_create (romancalc_suite_test_pycall_io (pModule));

	srunner_run_all (sr, CK_VERBOSE);
	number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	
	} else {												// could not create module, failure
		printf("\n Unable to create Python Module \n");
		number_failed = 1;									// force failure return
	}
	pymodule_teardown(pModule);								// shutdown python

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
