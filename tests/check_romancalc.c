#include <stdlib.h>
#include <time.h>
#include <check.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../src/romancalc.h"

void
setup (void)
{
//	ptr_tst_pair = rnum_pair_create();
}

void
teardown (void)
{
//	rnum_str_free (ptr_tst_pair);
}

Suite *
romancalc_suite_core (void)
{
	Suite *s = suite_create ("\nRoman Calc Suite");
	
	/* Core test case */
	TCase *tc_core = tcase_create ("Core\n");
	tcase_add_checked_fixture (tc_core, setup, teardown);
	suite_add_tcase (s, tc_core);
	return s;
}

START_TEST (test_valid_numeral_str)
{// ADD to create subtracted values
	ck_assert_int_eq(rnum_valid_numeral_str("MDCLXVI"), RNUM_ERR_NONE);
}
END_TEST

Suite *
romancalc_suite_process_text_io(void)
{
	Suite *s = suite_create ("\nRoman Calc Suite Test Sub/ADD Text In Text Out check");
	TCase *tc_full_subtraction = tcase_create ("Test Roman Sub/ADD In Text Out \n");
	tcase_add_test (tc_full_subtraction, test_valid_numeral_str);
	suite_add_tcase (s, tc_full_subtraction);
	return s;
}

// MARK: Main routine
int
main (void)
{
  int number_failed;
	
  SRunner *sr = srunner_create (romancalc_suite_core ());

  srunner_run_all (sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
