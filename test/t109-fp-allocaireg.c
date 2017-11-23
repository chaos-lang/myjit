#define JIT_REGISTER_TEST
#include "tests.h"
#include "simple-buffer.c"

#define BLOCK_SIZE	(16)

int correct_result;

static int foobar(double a1, double a2, double a3, double a4, double a5, double a6, double a7, double a8, double a9, double a10)
{
        correct_result = 1;
        if (!equals(a1, -1, 0.0001)) correct_result = 0;
        if (!equals(a2, 1, 0.0001)) correct_result = 0;
        if (!equals(a3, 2, 0.0001)) correct_result = 0;
        if (!equals(a4, 4, 0.0001)) correct_result = 0;
        if (!equals(a5, 8, 0.0001)) correct_result = 0;
        if (!equals(a6, 16, 0.0001)) correct_result = 0;
        if (!equals(a7, 32, 0.0001)) correct_result = 0;
        if (!equals(a8, 64, 0.0001)) correct_result = 0;
        if (!equals(a9, 128, 0.0001)) correct_result = 0;
        if (!equals(a10, 256, 0.0001)) correct_result = 0;
	return 666;
}

static int ifd30(int a1, float a2, double a3, int a4, float a5, double a6,
                 int a7, float a8, double a9, int a10, float a11, double a12,
                 int a13, float a14, double a15, int a16, float a17, double a18,
                 int a19, float a20, double a21, int a22, float a23, double a24,
                 int a25, float a26, double a27, int a28, float a29, double a30)
{
        correct_result = 1;
        if (!equals(a1, 1, 0.0001)) correct_result = 0;
        if (!equals(a2, 2, 0.0001)) correct_result = 0;
        if (!equals(a3, 3, 0.0001)) correct_result = 0;
        if (!equals(a4, 4, 0.0001)) correct_result = 0;
        if (!equals(a5, 5, 0.0001)) correct_result = 0;
        if (!equals(a6, 6, 0.0001)) correct_result = 0;
        if (!equals(a7, 7, 0.0001)) correct_result = 0;
        if (!equals(a8, 8, 0.0001)) correct_result = 0;
        if (!equals(a9, 9, 0.0001)) correct_result = 0;
        if (!equals(a10, 10, 0.0001)) correct_result = 0;
        if (!equals(a11, 11, 0.0001)) correct_result = 0;
        if (!equals(a12, 12, 0.0001)) correct_result = 0;
        if (!equals(a13, 13, 0.0001)) correct_result = 0;
        if (!equals(a14, 14, 0.0001)) correct_result = 0;
        if (!equals(a15, 15, 0.0001)) correct_result = 0;
        if (!equals(a16, 16, 0.0001)) correct_result = 0;
        if (!equals(a17, 17, 0.0001)) correct_result = 0;
        if (!equals(a18, 18, 0.0001)) correct_result = 0;
        if (!equals(a19, 19, 0.0001)) correct_result = 0;
        if (!equals(a20, 20, 0.0001)) correct_result = 0;
        if (!equals(a21, 21, 0.0001)) correct_result = 0;
        if (!equals(a22, 22, 0.0001)) correct_result = 0;
        if (!equals(a23, 23, 0.0001)) correct_result = 0;
        if (!equals(a24, 24, 0.0001)) correct_result = 0;
        if (!equals(a25, 25, 0.0001)) correct_result = 0;
        if (!equals(a26, 26, 0.0001)) correct_result = 0;
        if (!equals(a27, 27, 0.0001)) correct_result = 0;
        if (!equals(a28, 28, 0.0001)) correct_result = 0;
        if (!equals(a29, 29, 0.0001)) correct_result = 0;
        if (!equals(a30, 30, 0.0001)) correct_result = 0;

	return 666;
}

static int iff30(int a1, float a2, float a3, int a4, float a5, float a6,
                 int a7, float a8, float a9, int a10, float a11, float a12,
                 int a13, float a14, float a15, int a16, float a17, float a18,
                 int a19, float a20, float a21, int a22, float a23, float a24,
                 int a25, float a26, float a27, int a28, float a29, float a30)
{
        correct_result = 1;
        if (!equals(a1, 1, 0.0001)) correct_result = 0;
        if (!equals(a2, 2, 0.0001)) correct_result = 0;
        if (!equals(a3, 3, 0.0001)) correct_result = 0;
        if (!equals(a4, 4, 0.0001)) correct_result = 0;
        if (!equals(a5, 5, 0.0001)) correct_result = 0;
        if (!equals(a6, 6, 0.0001)) correct_result = 0;
        if (!equals(a7, 7, 0.0001)) correct_result = 0;
        if (!equals(a8, 8, 0.0001)) correct_result = 0;
        if (!equals(a9, 9, 0.0001)) correct_result = 0;
        if (!equals(a10, 10, 0.0001)) correct_result = 0;
        if (!equals(a11, 11, 0.0001)) correct_result = 0;
        if (!equals(a12, 12, 0.0001)) correct_result = 0;
        if (!equals(a13, 13, 0.0001)) correct_result = 0;
        if (!equals(a14, 14, 0.0001)) correct_result = 0;
        if (!equals(a15, 15, 0.0001)) correct_result = 0;
        if (!equals(a16, 16, 0.0001)) correct_result = 0;
        if (!equals(a17, 17, 0.0001)) correct_result = 0;
        if (!equals(a18, 18, 0.0001)) correct_result = 0;
        if (!equals(a19, 19, 0.0001)) correct_result = 0;
        if (!equals(a20, 20, 0.0001)) correct_result = 0;
        if (!equals(a21, 21, 0.0001)) correct_result = 0;
        if (!equals(a22, 22, 0.0001)) correct_result = 0;
        if (!equals(a23, 23, 0.0001)) correct_result = 0;
        if (!equals(a24, 24, 0.0001)) correct_result = 0;
        if (!equals(a25, 25, 0.0001)) correct_result = 0;
        if (!equals(a26, 26, 0.0001)) correct_result = 0;
        if (!equals(a27, 27, 0.0001)) correct_result = 0;
        if (!equals(a28, 28, 0.0001)) correct_result = 0;
        if (!equals(a29, 29, 0.0001)) correct_result = 0;
        if (!equals(a30, 30, 0.0001)) correct_result = 0;

	return 666;
}


DEFINE_TEST(test1)
{
	correct_result = 2;
	static char * msg2 = "%u ";
	simple_buffer(BUFFER_CLEAR, NULL);

	plfv foo;

	jit_prolog(p, &foo);
	int i = jit_allocai(p, BLOCK_SIZE);

	// loads into the allocated memory multiples of 3
	jit_movi(p, R(0), 0);	// index
	jit_addi(p, R(1), R_FP, i); // pointer to the allocated memory
	jit_label * lab = jit_get_label(p);
	jit_muli(p, R(2), R(0), 3);
	jit_stxr(p, R(1), R(0), R(2), 1);
	jit_addi(p, R(0), R(0), 1);
	jit_blti(p, lab, R(0), BLOCK_SIZE);

	// does something with registers
	jit_fmovi(p, FR(0), -1);
	jit_fmovi(p, FR(1), 1);
	jit_fmovi(p, FR(2), 2);
	jit_fmovi(p, FR(3), 4);
	jit_fmovi(p, FR(4), 8);
	jit_fmovi(p, FR(5), 16);
	jit_fmovi(p, FR(6), 32);
	jit_fmovi(p, FR(7), 64);
	jit_fmovi(p, FR(8), 128);
	jit_fmovi(p, FR(9), 256);

	jit_prepare(p);
	jit_fputargr(p, FR(0), sizeof(double));
	jit_fputargr(p, FR(1), sizeof(double));
	jit_fputargr(p, FR(2), sizeof(double));
	jit_fputargr(p, FR(3), sizeof(double));
	jit_fputargr(p, FR(4), sizeof(double));
	jit_fputargr(p, FR(5), sizeof(double));
	jit_fputargr(p, FR(6), sizeof(double));
	jit_fputargr(p, FR(7), sizeof(double));
	jit_fputargr(p, FR(8), sizeof(double));
	jit_fputargr(p, FR(9), sizeof(double));
	jit_call(p, foobar);

	// print outs allocated memory

	jit_movi(p, R(0), 0);	// index
	jit_addi(p, R(1), R_FP, i); // pointer to the allocated memory
	jit_label * lab2 = jit_get_label(p);
	
	jit_ldxr(p, R(2), R(1), R(0), 1);

	jit_prepare(p);
	jit_putargi(p, BUFFER_PUT);
	jit_putargi(p, msg2);
	jit_putargr(p, R(2));
	jit_call(p, simple_buffer);

	jit_addi(p, R(0), R(0), 1);
	jit_blti(p, lab2, R(0), BLOCK_SIZE);

	jit_retr(p, R(0));


	JIT_GENERATE_CODE(p);
	ASSERT_EQ(16, foo());
	ASSERT_EQ(1, correct_result);
	ASSERT_EQ_STR("0 3 6 9 12 15 18 21 24 27 30 33 36 39 42 45 ", simple_buffer(BUFFER_GET, NULL));
	return 0;
}

DEFINE_TEST(test2)
{
	correct_result = 2;
	static char *msg2 = "%u ";
	printf("ptr:%zx\n", msg2);
	simple_buffer(BUFFER_CLEAR, NULL);

	plfv foo;

	jit_prolog(p, &foo);
	int i = jit_allocai(p, BLOCK_SIZE);

	// loads into the allocated memory multiples of 3
	jit_movi(p, R(0), 0);	// index
	jit_addi(p, R(1), R_FP, i); // pointer to the allocated memory
	jit_label * lab = jit_get_label(p);
	jit_muli(p, R(2), R(0), 3);
	jit_stxr(p, R(1), R(0), R(2), 1);
	jit_addi(p, R(0), R(0), 1);
	jit_blti(p, lab, R(0), BLOCK_SIZE);

	// does something with registers

	jit_movi(p, R(1), 1);
	jit_fmovi(p, FR(2), 2.0);
	jit_fmovi(p, FR(3), 3.0);
	jit_movi(p, R(4), 4);
	jit_fmovi(p, FR(5), 5.0);
	jit_fmovi(p, FR(6), 6.0);
	jit_movi(p, R(7), 7);
	jit_fmovi(p, FR(8), 8.0);
	jit_fmovi(p, FR(9), 9.0);
	jit_movi(p, R(10), 10);
	jit_movi(p, R(22), 22);
	jit_fmovi(p, FR(23), 23.0);
	jit_fmovi(p, FR(24), 24.0);
	jit_movi(p, R(25), 25);
	jit_fmovi(p, FR(26), 26.0);
	jit_fmovi(p, FR(27), 27.0);


	jit_prepare(p);

	jit_putargr(p, R(1));
	jit_fputargr(p, FR(2), sizeof(float));
	jit_fputargr(p, FR(3), sizeof(double));
	jit_putargr(p, R(4));
	jit_fputargr(p, FR(5), sizeof(float));
	jit_fputargr(p, FR(6), sizeof(double));
	jit_putargr(p, R(7));
	jit_fputargr(p, FR(8), sizeof(float));
	jit_fputargr(p, FR(9), sizeof(double));
	jit_putargr(p, R(10));

	jit_fputargi(p, 11.0, sizeof(float));
	jit_fputargi(p, 12.0, sizeof(double));
	jit_putargi(p, 13);
	jit_fputargi(p, 14.0, sizeof(float));
	jit_fputargi(p, 15.0, sizeof(double));
	jit_putargi(p, 16);
	jit_fputargi(p, 17.0, sizeof(float));
	jit_fputargi(p, 18.0, sizeof(double));
	jit_putargi(p, 19);
	jit_fputargi(p, 20.0, sizeof(float));

	jit_fputargi(p, 21.0, sizeof(double));
	jit_putargr(p, R(22));
	jit_fputargr(p, FR(23), sizeof(float));
	jit_fputargr(p, FR(24), sizeof(double));
	jit_putargr(p, R(25));
	jit_fputargr(p, FR(26), sizeof(float));
	jit_fputargr(p, FR(27), sizeof(double));
	jit_putargi(p, 28);
	jit_fputargi(p, 29.0, sizeof(float));
	jit_fputargi(p, 30.0, sizeof(double));

	jit_call(p, ifd30);

	// print outs allocated memory

	jit_comment(p, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
	jit_movi(p, R(0), 0);	// index
	jit_addi(p, R(1), R_FP, i); // pointer to the allocated memory
	jit_label * lab2 = jit_get_label(p);
	
	jit_ldxr(p, R(2), R(1), R(0), 1);

	jit_prepare(p);
	jit_putargi(p, BUFFER_PUT);
	jit_putargi(p, msg2);
	jit_putargr(p, R(2));
	jit_call(p, simple_buffer);

	jit_addi(p, R(0), R(0), 1);
	jit_blti(p, lab2, R(0), BLOCK_SIZE);

	jit_retr(p, R(0));


	JIT_GENERATE_CODE(p);
	ASSERT_EQ(16, foo());
	ASSERT_EQ(1, correct_result);
	ASSERT_EQ_STR("0 3 6 9 12 15 18 21 24 27 30 33 36 39 42 45 ", simple_buffer(BUFFER_GET, NULL));
	return 0;
}

DEFINE_TEST(test3)
{
	correct_result = 2;
	static char *msg2 = "%u ";
	printf("ptr:%zx\n", msg2);
	simple_buffer(BUFFER_CLEAR, NULL);

	plfv foo;

	jit_prolog(p, &foo);
	int i = jit_allocai(p, BLOCK_SIZE);

	// loads into the allocated memory multiples of 3
	jit_movi(p, R(0), 0);	// index
	jit_addi(p, R(1), R_FP, i); // pointer to the allocated memory
	jit_label * lab = jit_get_label(p);
	jit_muli(p, R(2), R(0), 3);
	jit_stxr(p, R(1), R(0), R(2), 1);
	jit_addi(p, R(0), R(0), 1);
	jit_blti(p, lab, R(0), BLOCK_SIZE);

	// does something with registers

	jit_movi(p, R(1), 1);
	jit_fmovi(p, FR(2), 2.0);
	jit_fmovi(p, FR(3), 3.0);
	jit_movi(p, R(4), 4);
	jit_fmovi(p, FR(5), 5.0);
	jit_fmovi(p, FR(6), 6.0);
	jit_movi(p, R(7), 7);
	jit_fmovi(p, FR(8), 8.0);
	jit_fmovi(p, FR(9), 9.0);
	jit_movi(p, R(10), 10);
	jit_movi(p, R(22), 22);
	jit_fmovi(p, FR(23), 23.0);
	jit_fmovi(p, FR(24), 24.0);
	jit_movi(p, R(25), 25);
	jit_fmovi(p, FR(26), 26.0);
	jit_fmovi(p, FR(27), 27.0);


	jit_prepare(p);

	jit_putargr(p, R(1));
	jit_fputargr(p, FR(2), sizeof(float));
	jit_fputargr(p, FR(3), sizeof(float));
	jit_putargr(p, R(4));
	jit_fputargr(p, FR(5), sizeof(float));
	jit_fputargr(p, FR(6), sizeof(float));
	jit_putargr(p, R(7));
	jit_fputargr(p, FR(8), sizeof(float));
	jit_fputargr(p, FR(9), sizeof(float));
	jit_putargr(p, R(10));

	jit_fputargi(p, 11.0, sizeof(float));
	jit_fputargi(p, 12.0, sizeof(float));
	jit_putargi(p, 13);
	jit_fputargi(p, 14.0, sizeof(float));
	jit_fputargi(p, 15.0, sizeof(float));
	jit_putargi(p, 16);
	jit_fputargi(p, 17.0, sizeof(float));
	jit_fputargi(p, 18.0, sizeof(float));
	jit_putargi(p, 19);
	jit_fputargi(p, 20.0, sizeof(float));

	jit_fputargi(p, 21.0, sizeof(float));
	jit_putargr(p, R(22));
	jit_fputargr(p, FR(23), sizeof(float));
	jit_fputargr(p, FR(24), sizeof(float));
	jit_putargr(p, R(25));
	jit_fputargr(p, FR(26), sizeof(float));
	jit_fputargr(p, FR(27), sizeof(float));
	jit_putargi(p, 28);
	jit_fputargi(p, 29.0, sizeof(float));
	jit_fputargi(p, 30.0, sizeof(float));

	jit_call(p, iff30);

	// print outs allocated memory

	jit_comment(p, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
	jit_movi(p, R(0), 0);	// index
	jit_addi(p, R(1), R_FP, i); // pointer to the allocated memory
	jit_label * lab2 = jit_get_label(p);
	
	jit_ldxr(p, R(2), R(1), R(0), 1);

	jit_prepare(p);
	jit_putargi(p, BUFFER_PUT);
	jit_putargi(p, msg2);
	jit_putargr(p, R(2));
	jit_call(p, simple_buffer);

	jit_addi(p, R(0), R(0), 1);
	jit_blti(p, lab2, R(0), BLOCK_SIZE);

	jit_retr(p, R(0));


	JIT_GENERATE_CODE(p);
	ASSERT_EQ(16, foo());
	ASSERT_EQ(1, correct_result);
	ASSERT_EQ_STR("0 3 6 9 12 15 18 21 24 27 30 33 36 39 42 45 ", simple_buffer(BUFFER_GET, NULL));
	return 0;
}




void test_setup()
{
	test_filename = __FILE__;
	SETUP_TEST(test1);
	SETUP_TEST(test2);
	SETUP_TEST(test3);
}
