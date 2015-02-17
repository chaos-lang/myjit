#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../myjit/jitlib.h"

typedef jit_value (*plfv)(void);
typedef jit_value (*plfl)(jit_value);
typedef jit_value (*plfll)(jit_value, jit_value);
typedef double (*pdfv)(void);
typedef double (*pdfd)(double);
typedef jit_value (*plfd)(double);
typedef void * (*ppfl)(jit_value);

#define DUMP_CODE       0x01
#define DUMP_COMPILABLE 0x02
#define DUMP_ASSOC      0x04
#define DUMP_LIVENESS   0x08
#define OPT_LIST        0x10
#define OPT_ALL         0x20


#define TOLERANCE	0.00001
#define MAX_NUMBER_OF_TEST      (1024)

typedef int(*test_case_fn)(struct jit *, char *, int);
test_case_fn test_cases[MAX_NUMBER_OF_TEST];
char * test_names[MAX_NUMBER_OF_TEST];
int test_cnt = 0;
char *test_filename;

#define DEFINE_TEST(_name) \
	int _name(struct jit *p, char *test_name, int test_flags)

#define SETUP_TEST(name) \
	test_cases[test_cnt] = name;\
	test_names[test_cnt] = "" #name "";\
	test_cnt++;

#define JIT_GENERATE_CODE(p)  { \
	if (test_flags & DUMP_COMPILABLE) jit_dump_ops(p, JIT_DEBUG_COMPILABLE); \
	jit_generate_code(p); \
	if (test_flags & (DUMP_ASSOC | DUMP_LIVENESS)) \
	jit_dump_ops(p, (test_flags & DUMP_ASSOC ? JIT_DEBUG_ASSOC : 0) | (test_flags & DUMP_LIVENESS ? JIT_DEBUG_LIVENESS : 0)); \
	if (test_flags & DUMP_CODE) jit_dump_code(p, 0); \
}


#define ASSERT_EQ(_expected, _actual) { \
	if (_expected != _actual)  {\
		fprintf(stderr, "%s: %s (expected: %li, actual: %li)\n", test_filename, test_name, (jit_value)_expected, (jit_value)_actual); \
		return 1; \
	} \
}

#define ASSERT_EQ_DOUBLE(_expected, _actual) { \
	if (!equals((_expected), (_actual),  TOLERANCE)) {\
		fprintf(stderr, "%s: %s (expected: %f, actual: %f)\n", test_filename, test_name, (double)_expected, (double)_actual); \
		return 1; \
	} \
}

#define ASSERT_EQ_STR(_expected, _actual) { \
	if (strcmp(_expected, _actual))  {\
		fprintf(stderr, "%s: %s (expected: %s, actual: %s)\n", test_filename, test_name, (char *)_expected, (char *)_actual); \
		return 1; \
	} \
}

static inline int equals(double x, double y, double tolerance)
{
	return fabs(x - y) < tolerance;
}

void test_setup();

int run_test(int id, int options)
{       
	struct jit *p = jit_init();
	int result = test_cases[id](p, test_names[id], options);
	jit_free(p);
	return result;
}

void report_results_and_quit(int successful, int total)
{       
	int ok = (total == successful);
	if (ok) printf(" \033[1;32mOK\033[0m ");
	else printf(" \033[1;31m!!\033[0m ");
	printf("%-25s %i/%i\n", test_filename, successful, total);
	exit(ok ? 0 : 1);
}

int main(int argc, char **argv)
{       
	int options = 0; 
	int successful = 0;
	if (argc == 1) options |= OPT_ALL;

	for (int i = 1; i < argc; i++) {
		if (!strcmp("--code", argv[i])) options |= DUMP_CODE;
		if (!strcmp("--compilable", argv[i])) options |= DUMP_COMPILABLE;
		if (!strcmp("--assoc", argv[i])) options |= DUMP_ASSOC;
		if (!strcmp("--liveness", argv[i])) options |= DUMP_LIVENESS;
		if (!strcmp("-l", argv[i])) options |= OPT_LIST;
		if (!strcmp("--all", argv[i])) options |= OPT_ALL;
	}

	test_setup();

	if (options & OPT_LIST) {
		for (int i = 0; i < test_cnt; i++)
			printf("%s\n", test_names[i]);
		return 0;
	}

	if (options & OPT_ALL) {
		for (int i = 0; i < test_cnt; i++) {
			if (run_test(i, options) == 0) successful++;
		}
		report_results_and_quit(successful, test_cnt);
	}

	int performed = 0;
	for (int i = 1; i < argc; i++) {
		char *name = argv[i];
		if (name[0] == '-') continue;

		int found = 0;
		for (int j = 0; j < test_cnt; j++) {
			if (!strcmp(test_names[j], name)) {
				performed++;
				if (run_test(j, options) == 0) successful++;
				found = 1;
				break;
			}
		}
		if (!found) fprintf(stderr, "%s: %s: test not found!\n", test_filename, name);
	}
	report_results_and_quit(successful, performed);

}




// <obsolete>
#define SUCCESS(x) printf("%s:\ttest%i\thas succeeded\n", __FILE__, x)
#define FAIL(x) printf("%s:\ttest%i\thas failed\n", __FILE__, x)
#define FAILX(x, r, expt) printf("%s:\ttest%i\thas failed. Return value is `%li' but expected `%li':%i\n", __FILE__, x, r, (long)(expt), r == expt)
#define FAILD(x, r, expt) printf("%s:\ttest%i\thas failed. Return value is `%f' but expected `%f':%i\n", __FILE__, x, r, expt, r == expt)


// </OBSOLETE>
