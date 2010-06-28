#include <limits.h>
#include "../myjit/jitlib.h"
#include "tests.h"

#define CREATE_TEST_SIGNED_REG(testname, __data_type, __ji_op, __operator) \
void testname(int id, int samereg, int equal, int negative, int greater)\
{\
	long r;\
	struct jit * p = jit_init(16);\
	plfv f1;\
	jit_prolog(p, &f1);\
\
	__data_type firstval = 42;\
	__data_type secval = 28;\
	if (equal) secval = firstval;\
	if (greater) secval = 60;\
	if (negative) secval *= -1;\
\
	jit_movi(p, R(1), firstval);\
	jit_movi(p, R(2), secval);\
\
	if (samereg) {\
		__ji_op(p, R(2), R(1), R(2));\
		jit_retr(p, R(2));\
	} else {\
		__ji_op(p, R(3), R(1), R(2));\
		jit_retr(p, R(3));\
	}\
\
	jit_generate_code(p);\
	r = f1();\
	int testid = id * 10000 + samereg * 1000 + equal * 100 + negative * 10 + greater;\
	if ((r == 0) && !(firstval __operator secval)) SUCCESS(testid);\
	else if ((r == 1) && (firstval __operator secval)) SUCCESS(testid);\
	else FAIL(testid);\
	jit_free(p);\
}

#define CREATE_TEST_SIGNED_IMM(testname, __data_type, __ji_op, __operator) \
void testname(int id, int samereg, int equal, int negative, int greater)\
{\
	long r;\
	struct jit * p = jit_init(16);\
	plfv f1;\
	jit_prolog(p, &f1);\
\
	__data_type firstval = 42;\
	__data_type secval = 28;\
	if (equal) secval = firstval;\
	if (greater) secval = 60;\
	if (negative) secval *= -1;\
\
	jit_movi(p, R(1), firstval);\
\
	if (samereg) {\
		__ji_op(p, R(2), R(1), secval);\
		jit_retr(p, R(2));\
	} else {\
		__ji_op(p, R(3), R(1), secval);\
		jit_retr(p, R(3));\
	}\
\
	jit_generate_code(p);\
	r = f1();\
	int testid = id * 10000 + samereg * 1000 + equal * 100 + negative * 10 + greater;\
	if ((r == 0) && !(firstval __operator secval)) SUCCESS(testid);\
	else if ((r == 1) && (firstval __operator secval)) SUCCESS(testid);\
	else FAIL(testid);\
	jit_free(p);\
}

#define TEST_SET(testname, id) \
	testname(id, 0, 0, 0, 0);\
	testname(id, 0, 0, 0, 1);\
	testname(id, 0, 0, 1, 0);\
	testname(id, 0, 0, 1, 1);\
	testname(id, 0, 1, 0, 0);\
	testname(id, 0, 1, 0, 1);\
	testname(id, 0, 1, 1, 0);\
	testname(id, 0, 1, 1, 1);\
	testname(id, 1, 0, 0, 0);\
	testname(id, 1, 0, 0, 1);\
	testname(id, 1, 0, 1, 0);\
	testname(id, 1, 0, 1, 1);\
	testname(id, 1, 1, 0, 0);\
	testname(id, 1, 1, 0, 1);\
	testname(id, 1, 1, 1, 0);\
	testname(id, 1, 1, 1, 1);\


CREATE_TEST_SIGNED_REG(test01reg, long, jit_ltr, <)
CREATE_TEST_SIGNED_REG(test02reg, long, jit_ler, <=)
CREATE_TEST_SIGNED_REG(test03reg, long, jit_ger, >=)
CREATE_TEST_SIGNED_REG(test04reg, long, jit_gtr, >)
CREATE_TEST_SIGNED_REG(test05reg, long, jit_eqr, ==)
CREATE_TEST_SIGNED_REG(test06reg, long, jit_ner, !=)

CREATE_TEST_SIGNED_REG(test11reg, unsigned long, jit_ltr_u, <)
CREATE_TEST_SIGNED_REG(test12reg, unsigned long, jit_ler_u, <=)
CREATE_TEST_SIGNED_REG(test13reg, unsigned long, jit_ger_u, >=)
CREATE_TEST_SIGNED_REG(test14reg, unsigned long, jit_gtr_u, >)
CREATE_TEST_SIGNED_REG(test15reg, unsigned long, jit_eqr, ==)
CREATE_TEST_SIGNED_REG(test16reg, unsigned long, jit_ner, !=)

CREATE_TEST_SIGNED_IMM(test21imm, long, jit_lti, <)
CREATE_TEST_SIGNED_IMM(test22imm, long, jit_lei, <=)
CREATE_TEST_SIGNED_IMM(test23imm, long, jit_gei, >=)
CREATE_TEST_SIGNED_IMM(test24imm, long, jit_gti, >)
CREATE_TEST_SIGNED_IMM(test25imm, long, jit_eqi, ==)
CREATE_TEST_SIGNED_IMM(test26imm, long, jit_nei, !=)

CREATE_TEST_SIGNED_IMM(test31imm, unsigned long, jit_lti_u, <)
CREATE_TEST_SIGNED_IMM(test32imm, unsigned long, jit_lei_u, <=)
CREATE_TEST_SIGNED_IMM(test33imm, unsigned long, jit_gei_u, >=)
CREATE_TEST_SIGNED_IMM(test34imm, unsigned long, jit_gti_u, >)
CREATE_TEST_SIGNED_IMM(test35imm, unsigned long, jit_eqi, ==)
CREATE_TEST_SIGNED_IMM(test36imm, unsigned long, jit_nei, !=)


int main()
{
	TEST_SET(test01reg, 1);
	TEST_SET(test02reg, 2);
	TEST_SET(test03reg, 3);
	TEST_SET(test04reg, 4);
	TEST_SET(test05reg, 5);
	TEST_SET(test06reg, 6);

	TEST_SET(test11reg, 11);
	TEST_SET(test12reg, 12);
	TEST_SET(test13reg, 13);
	TEST_SET(test14reg, 14);
	TEST_SET(test15reg, 15);
	TEST_SET(test16reg, 16);

	TEST_SET(test21imm, 21);
	TEST_SET(test22imm, 22);
	TEST_SET(test23imm, 23);
	TEST_SET(test24imm, 24);
	TEST_SET(test25imm, 25);
	TEST_SET(test26imm, 26);

	TEST_SET(test31imm, 31);
	TEST_SET(test32imm, 32);
	TEST_SET(test33imm, 33);
	TEST_SET(test34imm, 34);
	TEST_SET(test35imm, 35);
	TEST_SET(test36imm, 36);
}
