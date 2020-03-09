#include <stdio.h>
#include <unistd.h>

int main(void)
{
	float ff = 123456.0;
	unsigned short aa[4] = {0};

	aa[0] = (unsigned short)ff;
//	aa[1] = (unsigned short)(ff >> 16);

	printf("%f: %X, %X\n", ff, (unsigned int)ff, aa[0]);


	union{int i; float f; unsigned char c[4];} conv;
	conv.f = ff;
	aa[0] = (unsigned short)conv.i;
	aa[1] = (unsigned short)(conv.i >> 16);
	printf("%f: %X, %X, %X\n", ff, conv.i, aa[0], aa[1]);

	conv.f = 100.0;
	conv.c[0] = 0x64; 
	conv.c[1] = 0x00; 
	conv.c[2] = 0xc8; 
	conv.c[3] = 0x42; 
	printf("%f: %X, %X, %X, %X, %X\n", conv.f, conv.i, conv.c[0], conv.c[1], conv.c[2], conv.c[3]);

	return 0;
}
