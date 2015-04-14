#include "tests.h"

#define LOOP_CNT	(10000000)
int value1 = 666;
int value2 = 777;

int cond_value = 10;

// implements ternary operator (old-fashioned way); end tests efficiency
DEFINE_TEST(test1)
{
	plfl f1;
	jit_prolog(p, &f1);
	jit_declare_arg(p, JIT_SIGNED_NUM, sizeof(long));

	jit_getarg(p, R(0), 0);
	jit_movi(p, R(1), LOOP_CNT);

	jit_label * loop = jit_get_label(p);

	jit_op * eq = jit_beqi(p, JIT_FORWARD, R(0), cond_value);
	jit_movi(p, R(2), value2);
	jit_op * end = jit_jmpi(p, JIT_FORWARD);
	jit_patch(p, eq);
	jit_movi(p, R(2), value1);
	jit_patch(p, end);

	jit_subi(p, R(1), R(1), 1);
	jit_bgti(p, loop, R(1), 0);

	jit_retr(p, R(2));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ(666, f1(10));
	ASSERT_EQ(777, f1(20));
	return 0;
}

// implements ternary operator (tricky); end tests efficiency
DEFINE_TEST(test2)
{
	plfl f1;
	jit_prolog(p, &f1);
	jit_declare_arg(p, JIT_SIGNED_NUM, sizeof(long));

	jit_getarg(p, R(0), 0);
	jit_movi(p, R(1), LOOP_CNT);

	jit_label * loop = jit_get_label(p);

	jit_eqi(p, R(2), R(0), cond_value);

	jit_subi(p, R(2), R(2), 1);
	jit_andi(p, R(2), R(2), value2 - value1);
	jit_addi(p, R(2), R(2), value1);


	jit_subi(p, R(1), R(1), 1);
	jit_bgti(p, loop, R(1), 0);

	jit_retr(p, R(2));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ(666, f1(10));
	ASSERT_EQ(777, f1(20));

	return 0;
}

#ifdef JIT_ARCH_COMMON86
DEFINE_TEST(test10)
{
	plfl f1;
	jit_prolog(p, &f1);
	jit_declare_arg(p, JIT_SIGNED_NUM, sizeof(long));

	jit_getarg(p, R(0), 0);
	jit_movi(p, R(1), 123);
	
	jit_force_spill(p, R(0));
	jit_force_spill(p, R(1));

	jit_force_assoc(p, R(1), 0, 0); // AX
	jit_force_assoc(p, R(0), 0, 2); // CX

	jit_lshr(p, R(0), R(1), R(0));

	jit_retr(p, R(0));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ(123 * 4, f1(2));

	return 0;
}

DEFINE_TEST(test11)
{
	plfl f1;
	jit_prolog(p, &f1);
	jit_declare_arg(p, JIT_SIGNED_NUM, sizeof(long));

	jit_getarg(p, R(0), 0);
	jit_movi(p, R(1), 123);
	
	jit_force_spill(p, R(0));
	jit_force_spill(p, R(1));

	jit_force_assoc(p, R(1), 0, 2); // CX
	jit_force_assoc(p, R(0), 0, 0); // AX

	jit_lshr(p, R(2), R(1), R(0));

	jit_retr(p, R(2));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ(123 * 4, f1(2));

	return 0;
}

DEFINE_TEST(test12)
{
	plfl f1;
	jit_prolog(p, &f1);
	jit_declare_arg(p, JIT_SIGNED_NUM, sizeof(long));

	jit_getarg(p, R(0), 0);
	jit_movi(p, R(1), 123);
	jit_movi(p, R(2), 111);
	
	jit_force_spill(p, R(0));
	jit_force_spill(p, R(1));

	jit_force_assoc(p, R(0), 0, 0); // AX
	jit_force_assoc(p, R(1), 0, 2); // CX
	jit_force_assoc(p, R(2), 0, 4); // SI

	jit_gtr(p, R(2), R(0), R(1));

	jit_addr(p, R(2), R(2), R(0));

	jit_retr(p, R(2));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ(2, f1(2));
	ASSERT_EQ(201, f1(200));

	return 0;
}
	
#endif

DEFINE_TEST(test20)
{
	plfv f1;
	jit_prolog(p, &f1);
	jit_movi(p, R(0), 0);
	for (int i = 0; i < 10000; i++)
		jit_addi(p, R(0), R(0), 1);
	jit_retr(p, R(0));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ(10000, f1());
	return 0;
}


DEFINE_TEST(test30)
{
	plfv f1;
	static char x[] = { -2, -3, -4, -5, -6, -7, -8, -9, -10 };
	static char y[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };

	jit_prolog(p, &f1);
	jit_movi(p, R(0), x + 2);
	jit_movi(p, R(1), y + 1);

	jit_memcpyi(p, R(0), R(1), 5);

	jit_reti(p, 0);
	JIT_GENERATE_CODE(p);

	ASSERT_EQ(0, f1());

	ASSERT_EQ(-2, x[0]);
	ASSERT_EQ(-3, x[1]);
	ASSERT_EQ(20, x[2]);
	ASSERT_EQ(30, x[3]);
	ASSERT_EQ(40, x[4]);
	ASSERT_EQ(50, x[5]);
	ASSERT_EQ(60, x[6]);
	ASSERT_EQ(-9, x[7]);
	ASSERT_EQ(-10, x[8]);

	return 0;
}

DEFINE_TEST(test31)
{
	plfv f1;
	static char x[] = { -2, -3, -4, -5, -6, -7, -8, -9, -10 };
	static char y[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };

	jit_prolog(p, &f1);
	jit_movi(p, R(0), x + 2);
	jit_movi(p, R(1), y + 1);
	jit_movi(p, R(2), 5);

	jit_memcpyr(p, R(0), R(1), R(2));

	jit_reti(p, 0);
	JIT_GENERATE_CODE(p);

	ASSERT_EQ(0, f1());

	ASSERT_EQ(-2, x[0]);
	ASSERT_EQ(-3, x[1]);
	ASSERT_EQ(20, x[2]);
	ASSERT_EQ(30, x[3]);
	ASSERT_EQ(40, x[4]);
	ASSERT_EQ(50, x[5]);
	ASSERT_EQ(60, x[6]);
	ASSERT_EQ(-9, x[7]);
	ASSERT_EQ(-10, x[8]);

	return 0;
}

DEFINE_TEST(test32)
{
	plfv f1;
	static char x[] = { -2, -3, -4, -5, -6, -7, -8, -9, -10 };
	static char y[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };

	jit_prolog(p, &f1);
	jit_movi(p, R(0), x + 2);
	jit_movi(p, R(1), y + 1);
	jit_movi(p, R(2), 5);

	jit_memcpyr(p, R(0), R(1), R(2));

	jit_retr(p, R(2));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ(5, f1());

	ASSERT_EQ(-2, x[0]);
	ASSERT_EQ(-3, x[1]);
	ASSERT_EQ(20, x[2]);
	ASSERT_EQ(30, x[3]);
	ASSERT_EQ(40, x[4]);
	ASSERT_EQ(50, x[5]);
	ASSERT_EQ(60, x[6]);
	ASSERT_EQ(-9, x[7]);
	ASSERT_EQ(-10, x[8]);

	return 0;
}

void test_setup()
{
	test_filename = __FILE__;
	SETUP_TEST(test1);
	SETUP_TEST(test2);
#ifdef JIT_ARCH_COMMON86
	SETUP_TEST(test10);
	SETUP_TEST(test11);
	SETUP_TEST(test12);
#endif
	SETUP_TEST(test20);
	SETUP_TEST(test30);
	SETUP_TEST(test31);
	SETUP_TEST(test32);
}
