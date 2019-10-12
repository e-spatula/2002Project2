#include "unit_test.h"

int assert_equals(int arg1, int arg2) {
	if(arg1 == arg2) {
		printf(GREEN "Test passed\n" RESET);
		return(0);
	} else {
		printf(RED "Test failed\n" RESET);
		return(1);
	} 
}

void print_results(int *failed, int *total) {
	int passed = *total - *failed;
	printf("Total tests : %i\n", *total);
	printf(GREEN "Passed : %i\n" RESET, passed);
	printf(RED "Failed : %i\n" RESET, *failed);
	if(*failed == 0) {
		printf(GREEN "All tests passed\n"RESET);
	} else {
		printf(RED "%i tests failed\n" RESET, *failed);
	}
	printf(RESET"\n");
}
