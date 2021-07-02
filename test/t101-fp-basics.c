#include "tests.h"

DEFINE_TEST(test1)
{
	pdfv f1;
	jit_prolog(p, &f1);
	jit_freti(p, 123, sizeof(double));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(123.0, f1());
	return 0;
}

DEFINE_TEST(test1f)
{
	pffv f1;
	jit_prolog(p, &f1);
	jit_freti(p, 123, sizeof(float));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(123.0, f1());
	return 0;
}

DEFINE_TEST(test2)
{
	pdfv f1;
	jit_prolog(p, &f1);
	jit_fmovi(p, FR(0), 100);
	jit_faddi(p, FR(0), FR(0), 200);
	jit_faddi(p, FR(1), FR(0), 200);
	jit_fretr(p, FR(1), sizeof(double));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(500.0, f1());
	return 0;
}

DEFINE_TEST(test2f)
{
	pffv f1;
	jit_prolog(p, &f1);
	jit_fmovi(p, FR(0), 100);
	jit_faddi(p, FR(0), FR(0), 200);
	jit_faddi(p, FR(1), FR(0), 200);
	jit_fretr(p, FR(1), sizeof(float));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(500.0, f1());
	return 0;
}

DEFINE_TEST(test3)
{
	pdfv f1;
	jit_prolog(p, &f1);
	jit_fmovi(p, FR(0), 100);
	jit_faddr(p, FR(0), FR(0), FR(0));
	jit_fsubi(p, FR(1), FR(0), 50);
	jit_fsubr(p, FR(2), FR(1), FR(0));
	jit_fretr(p, FR(2), sizeof(double));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(-50.0, f1());
	return 0;
}

DEFINE_TEST(test3f)
{
	pffv f1;
	jit_prolog(p, &f1);
	jit_fmovi(p, FR(0), 100);
	jit_faddr(p, FR(0), FR(0), FR(0));
	jit_fsubi(p, FR(1), FR(0), 50);
	jit_fsubr(p, FR(2), FR(1), FR(0));
	jit_fretr(p, FR(2), sizeof(float));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(-50.0, f1());
	return 0;
}

DEFINE_TEST(test4)
{
	pdfv f1;
	jit_prolog(p, &f1);
	jit_movi(p, R(0), 100);
	jit_movi(p, R(1), 50);

	jit_extr(p, FR(0), R(0));
	jit_extr(p, FR(1), R(1));

	jit_faddr(p, FR(0), FR(0), FR(0));
	jit_fsubr(p, FR(1), FR(0), FR(1));
	jit_fsubr(p, FR(2), FR(1), FR(0));
	jit_fretr(p, FR(2), sizeof(double));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(-50.0, f1());
	return 0;
}

DEFINE_TEST(test4f)
{
	pffv f1;
	jit_prolog(p, &f1);
	jit_movi(p, R(0), 100);
	jit_movi(p, R(1), 50);
	jit_msgr(p, "Reg R1: %lld\n", R(1));

	jit_extr(p, FR(0), R(0));
	jit_extr(p, FR(1), R(1));

	jit_faddr(p, FR(0), FR(0), FR(0));
	jit_fsubr(p, FR(1), FR(0), FR(1));
	jit_fsubr(p, FR(2), FR(1), FR(0));
	jit_fretr(p, FR(2), sizeof(float));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(-50.0, f1());
	return 0;
}

DEFINE_TEST(test5)
{
	pdfv f1;
	jit_prolog(p, &f1);
	jit_fmovi(p, FR(0), 100.0);
	jit_fmovr(p, FR(1), FR(0));
	jit_faddi(p, FR(0), FR(0), 20);

	jit_faddr(p, FR(2), FR(0), FR(1));
	jit_fretr(p, FR(2), sizeof(double));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(220.0, f1());
	return 0;
}

DEFINE_TEST(test5f)
{
	pffv f1;
	jit_prolog(p, &f1);
	jit_fmovi(p, FR(0), 100.0);
	jit_fmsgr(p, "Reg FR0: %lf\n", FR(0));
	jit_fmovr(p, FR(1), FR(0));
	jit_faddi(p, FR(0), FR(0), 20);

	jit_faddr(p, FR(2), FR(0), FR(1));
	jit_fretr(p, FR(2), sizeof(float));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(220.0, f1());
	return 0;
}

DEFINE_TEST(test10)
{
	double x = 123.4;
	pdfv f1;
	jit_prolog(p, &f1);
	jit_movi(p, R(0), &x);
	jit_fldr(p, FR(0), R(0), sizeof(double));
	jit_fretr(p, FR(0), sizeof(double));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(123.4, f1());
	return 0;
}

DEFINE_TEST(test10f)
{
	float x = 123.4;
	pffv f1;
	jit_prolog(p, &f1);
	jit_movi(p, R(0), &x);
	jit_fldr(p, FR(0), R(0), sizeof(float));
	jit_fretr(p, FR(0), sizeof(float));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(123.4, f1());
	return 0;
}

DEFINE_TEST(test11)
{
	float x = 123.4;
	pdfv f1;
	jit_prolog(p, &f1);
	jit_movi(p, R(0), &x);
	jit_fldr(p, FR(0), R(0), sizeof(float));
	jit_fretr(p, FR(0), sizeof(double));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(123.4, f1());
	return 0;
}

DEFINE_TEST(test12)
{
	double x = 123.4;
	pdfv f1;
	jit_prolog(p, &f1);
	jit_fldi(p, FR(0), &x, sizeof(double));
	jit_fretr(p, FR(0), sizeof(double));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(123.4, f1());
	return 0;
}


DEFINE_TEST(test13)
{
	float x = 123.4;
	pdfv f1;
	jit_prolog(p, &f1);
	jit_fldi(p, FR(0), &x, sizeof(float));
	jit_fretr(p, FR(0), sizeof(double));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(123.4, f1());
	return 0;
}

DEFINE_TEST(test14)
{
	double x[] = { 1.1, 2.2, 3.3 };
	pdfv f1;
	jit_prolog(p, &f1);
	jit_movi(p, R(0), &x);
	jit_movi(p, R(1), sizeof(double));
	jit_fldxr(p, FR(0), R(0), R(1), sizeof(double));
	jit_fretr(p, FR(0), sizeof(double));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(2.2, f1());
	return 0;
}

DEFINE_TEST(test14f)
{
	float x[] = { 1.1, 2.2, 3.3 };
	pffv f1;
	jit_prolog(p, &f1);
	jit_movi(p, R(0), &x);
	jit_movi(p, R(1), sizeof(float));
	jit_fldxr(p, FR(0), R(0), R(1), sizeof(float));
	jit_fretr(p, FR(0), sizeof(float));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(2.2, f1());
	return 0;
}

DEFINE_TEST(test20)
{
	double x[] = { 1.0, 2.0, 3.0, 4.0, 5.0 };
	pdfv f1;
	jit_prolog(p, &f1);

	jit_fmovi(p, FR(0), 1.1);
	jit_fsti(p, &x, FR(0), sizeof(double));	


	jit_movi(p, R(0), &x);
	jit_addi(p, R(1), R(0), sizeof(double));
	jit_fmovi(p, FR(0), 2.2);
	jit_fstr(p, R(1), FR(0), sizeof(double));	


	jit_fmovi(p, FR(0), 3.3);
	jit_fstxi(p, sizeof(double) * 2, R(0), FR(0), sizeof(double));	


	jit_fmovi(p, FR(0), 4.4);
	jit_movi(p, R(1), sizeof(double) * 3);
	jit_fstxr(p, R(1), R(0), FR(0), sizeof(double));	

	jit_freti(p, 10.10, sizeof(double));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(10.10, f1());
	ASSERT_EQ_DOUBLE(1.1, x[0]);
	ASSERT_EQ_DOUBLE(2.2, x[1]);
	ASSERT_EQ_DOUBLE(3.3, x[2]);
	ASSERT_EQ_DOUBLE(4.4, x[3]);
	ASSERT_EQ_DOUBLE(5.0, x[4]);
	return 0;
}

DEFINE_TEST(test21)
{
	float x[] = { 1.0, 2.0, 3.0, 4.0, 5.0 };
	pffv f1;
	jit_prolog(p, &f1);

	jit_fmovi(p, FR(0), 1.1);
	jit_fsti(p, &x, FR(0), sizeof(float));	


	jit_movi(p, R(0), &x);
	jit_addi(p, R(1), R(0), sizeof(float));
	jit_fmovi(p, FR(0), 2.2);
	jit_fstr(p, R(1), FR(0), sizeof(float));	


	jit_fmovi(p, FR(0), 3.3);
	jit_fstxi(p, sizeof(float) * 2, R(0), FR(0), sizeof(float));	


	jit_fmovi(p, FR(0), 4.4);
	jit_movi(p, R(1), sizeof(float) * 3);
	jit_fstxr(p, R(1), R(0), FR(0), sizeof(float));	

	jit_freti(p, 10.10, sizeof(float));
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(10.10, f1());
	ASSERT_EQ_DOUBLE(1.1, x[0]);
	ASSERT_EQ_DOUBLE(2.2, x[1]);
	ASSERT_EQ_DOUBLE(3.3, x[2]);
	ASSERT_EQ_DOUBLE(4.4, x[3]);
	ASSERT_EQ_DOUBLE(5.0, x[4]);
	return 0;
}

DEFINE_TEST(test30)
{
	static float x = 1.1;
	static double y = 2.3;

	plfv f1;
	jit_prolog(p, &f1);
	jit_fldi(p, FR(0), &x, sizeof(float));
	jit_fldi(p, FR(1), &y, sizeof(double));

	jit_faddi(p, FR(0), FR(0), 10.10);
	jit_faddi(p, FR(1), FR(1), 20.20);
	
	jit_fsti(p, &x, FR(0), sizeof(float));
	jit_fsti(p, &y, FR(1), sizeof(double));
	
	jit_reti(p, 666);
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(666, f1());
	ASSERT_EQ_DOUBLE(11.20, x);
	ASSERT_EQ_DOUBLE(22.50, y);
	return 0;
}

DEFINE_TEST(test31)
{
	static double x[] = {1.1, 2.2, 3.3, 4.4};

	plfv f1;
	jit_prolog(p, &f1);
	jit_movi(p, R(0), 8);
	jit_fldxi(p, FR(0), R(0), &x, sizeof(double));

	jit_faddi(p, FR(0), FR(0), 10.10);
	
	jit_fstxi(p, &x, R(0), FR(0), sizeof(double));
	
	jit_reti(p, 666);
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(666, f1());
	ASSERT_EQ_DOUBLE(1.1, x[0]);
	ASSERT_EQ_DOUBLE(12.3, x[1]);
	ASSERT_EQ_DOUBLE(3.3, x[2]);
	ASSERT_EQ_DOUBLE(4.4, x[3]);
	return 0;
}

DEFINE_TEST(test32)
{
	static float x[] = {1.1, 2.2, 3.3, 4.4};

	plfv f1;
	jit_prolog(p, &f1);
	jit_movi(p, R(0), 4);
	jit_fldxi(p, FR(0), R(0), &x, sizeof(float));

	jit_faddi(p, FR(0), FR(0), 10.10);
	
	jit_fstxi(p, &x, R(0), FR(0), sizeof(float));
	
	jit_reti(p, 666);
	JIT_GENERATE_CODE(p);

	ASSERT_EQ_DOUBLE(666, f1());
	ASSERT_EQ_DOUBLE(1.1, x[0]);
	ASSERT_EQ_DOUBLE(12.3, x[1]);
	ASSERT_EQ_DOUBLE(3.3, x[2]);
	ASSERT_EQ_DOUBLE(4.4, x[3]);
	return 0;
}


void test_setup() 
{
	test_filename = __FILE__;
	SETUP_TEST(test1);
	SETUP_TEST(test2);
	SETUP_TEST(test3);
	SETUP_TEST(test4);
	SETUP_TEST(test5);

	SETUP_TEST(test1f);
	SETUP_TEST(test2f);
	SETUP_TEST(test3f);
	SETUP_TEST(test4f);
	SETUP_TEST(test5f);

	SETUP_TEST(test10);
	SETUP_TEST(test10f);
	SETUP_TEST(test11);
	SETUP_TEST(test12);
	SETUP_TEST(test13);
	SETUP_TEST(test14);
	SETUP_TEST(test14f);

	SETUP_TEST(test20);
	SETUP_TEST(test21);

	SETUP_TEST(test30);
	SETUP_TEST(test31);
	SETUP_TEST(test32);
}
