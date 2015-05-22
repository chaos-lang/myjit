#define JIT_REGISTER_TEST
#include "tests.h"

DEFINE_TEST(test10)
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

DEFINE_TEST(test11)
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

DEFINE_TEST(test12)
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

DEFINE_TEST(test20)
{
	plfv f1;
	static char x[] = { -2, -3, -4, -5, -6, -7, -8, -9, -10 };
	static char y[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };

	jit_prolog(p, &f1);
	jit_movi(p, R(0), x + 2);
	jit_movi(p, R(1), y + 1);
	jit_movi(p, R(2), 3);

	jit_op *op = jit_transferr(p, R(0), R(1), R(2), 2);
	jit_transfer_cpy(p, op);

	jit_retr(p, R(2));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ(3, f1());

	ASSERT_EQ(-2, x[0]);
	ASSERT_EQ(-3, x[1]);
	ASSERT_EQ(20, x[2]);
	ASSERT_EQ(30, x[3]);
	ASSERT_EQ(40, x[4]);
	ASSERT_EQ(50, x[5]);
	ASSERT_EQ(60, x[6]);
	ASSERT_EQ(70, x[7]);
	ASSERT_EQ(-10, x[8]);

	return 0;
}

DEFINE_TEST(test21)
{
	plfv f1;
	static int x[] = { -2, -3, -4, -5, -6, -7, -8, -9, -10 };
	static int y[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };

	jit_prolog(p, &f1);
	jit_movi(p, R(0), x + 2);
	jit_movi(p, R(1), y + 1);
	jit_movi(p, R(2), 3);

	jit_op *op = jit_transferr(p, R(0), R(1), R(2), 8);
	jit_transfer_cpy(p, op);

	jit_retr(p, R(2));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ(3, f1());

	ASSERT_EQ(-2, x[0]);
	ASSERT_EQ(-3, x[1]);
	ASSERT_EQ(20, x[2]);
	ASSERT_EQ(30, x[3]);
	ASSERT_EQ(40, x[4]);
	ASSERT_EQ(50, x[5]);
	ASSERT_EQ(60, x[6]);
	ASSERT_EQ(70, x[7]);
	ASSERT_EQ(-10, x[8]);

	return 0;
}

DEFINE_TEST(test30)
{
	plfv f1;
	static int x[] = { -2, -3, -4, -5, -6, -7, -8, -9, -10 };
	static int y[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };

	jit_prolog(p, &f1);
	jit_movi(p, R(0), x + 2);
	jit_movi(p, R(1), y + 1);
	jit_movi(p, R(2), 6);
	jit_movi(p, R(3), 1);
	jit_op *op = jit_transferr(p, R(0), R(1), R(2), 4);
	jit_transfer_orr(p, op, R(3));

	jit_retr(p, R(2));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ(6, f1());

	ASSERT_EQ(-2, x[0]);
	ASSERT_EQ(-3, x[1]);
	ASSERT_EQ(21, x[2]);
	ASSERT_EQ(31, x[3]);
	ASSERT_EQ(41, x[4]);
	ASSERT_EQ(51, x[5]);
	ASSERT_EQ(61, x[6]);
	ASSERT_EQ(71, x[7]);
	ASSERT_EQ(-10, x[8]);

	return 0;
}

DEFINE_TEST(test31)
{
	plfv f1;
	static int x[] = { -2, -3, -4, -5, -6, -7, -8, -9, -10 };
	static int y[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };

	jit_prolog(p, &f1);
	jit_movi(p, R(0), x + 2);
	jit_movi(p, R(1), y + 1);
	jit_movi(p, R(2), 6);
	jit_movi(p, R(3), 7);
	jit_movi(p, R(4), ~7);
	jit_op *op = jit_transferr(p, R(0), R(1), R(2), 4);
	jit_transfer_addr(p, NULL, R(3));
	jit_transfer_andr(p, op, R(4));

	jit_retr(p, R(2));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ(6, f1());

	ASSERT_EQ(-2, x[0]);
	ASSERT_EQ(-3, x[1]);
	ASSERT_EQ(24, x[2]);
	ASSERT_EQ(32, x[3]);
	ASSERT_EQ(40, x[4]);
	ASSERT_EQ(56, x[5]);
	ASSERT_EQ(64, x[6]);
	ASSERT_EQ(72, x[7]);
	ASSERT_EQ(-10, x[8]);

	return 0;
}

DEFINE_TEST(test32)
{
	plfv f1;
	static int x[] = { -2, -3, -4, -5, -6, -7, -8, -9, -10 };
	static int y[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };

	jit_prolog(p, &f1);
	jit_movi(p, R(0), x + 2);
	jit_movi(p, R(1), y + 1);
	jit_movi(p, R(2), 6);
	jit_movi(p, R(3), 7);
	jit_movi(p, R(4), ~7);
	
	jit_force_spill(p, R(3));
	jit_op *op = jit_transferr(p, R(0), R(1), R(2), 4);
	jit_transfer_addr(p, NULL, R(3));
	jit_transfer_andr(p, op, R(4));

	jit_retr(p, R(2));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ(6, f1());

	ASSERT_EQ(-2, x[0]);
	ASSERT_EQ(-3, x[1]);
	ASSERT_EQ(24, x[2]);
	ASSERT_EQ(32, x[3]);
	ASSERT_EQ(40, x[4]);
	ASSERT_EQ(56, x[5]);
	ASSERT_EQ(64, x[6]);
	ASSERT_EQ(72, x[7]);
	ASSERT_EQ(-10, x[8]);

	return 0;
}

DEFINE_TEST(test33)
{
	plfv f1;
	static int x[] = { -2, -3, -4, -5, -6, -7, -8, -9, -10 };
	static int y[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };

	jit_prolog(p, &f1);
	jit_movi(p, R(0), x + 2);
	jit_movi(p, R(1), y + 1);
	jit_movi(p, R(2), 6);
	jit_movi(p, R(10), 1);
	jit_movi(p, R(11), 2);
	jit_movi(p, R(12), 3);
	jit_movi(p, R(13), 4);
	jit_movi(p, R(14), 5);
	
	jit_force_spill(p, R(3));
	jit_op *op = jit_transferr(p, R(0), R(1), R(2), 4);
	jit_transfer_addr(p, NULL, R(10));
	jit_transfer_addr(p, NULL, R(11));
	jit_transfer_addr(p, NULL, R(12));
	jit_transfer_addr(p, NULL, R(13));
	jit_transfer_addr(p, NULL, R(2));
	jit_transfer_addr(p, op, R(14));

	jit_retr(p, R(2));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ(6, f1());

	ASSERT_EQ(-2, x[0]);
	ASSERT_EQ(-3, x[1]);
	ASSERT_EQ(41, x[2]);
	ASSERT_EQ(51, x[3]);
	ASSERT_EQ(61, x[4]);
	ASSERT_EQ(71, x[5]);
	ASSERT_EQ(81, x[6]);
	ASSERT_EQ(91, x[7]);
	ASSERT_EQ(-10, x[8]);

	return 0;
}

void test_setup()
{
	test_filename = __FILE__;
	SETUP_TEST(test10);
	SETUP_TEST(test11);
	SETUP_TEST(test12);

	SETUP_TEST(test20);
	SETUP_TEST(test21);

	SETUP_TEST(test30);
	SETUP_TEST(test31);
	SETUP_TEST(test32);
	SETUP_TEST(test33);
}
