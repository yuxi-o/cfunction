#include <stdio.h>
#include <stdint.h>
#include <float.h>

int main()
{
	printf("UINT64_MAX:[%ld][0x%lx]\n", UINT64_MAX, UINT64_MAX);
	printf("UINT32_MAX:[%d][0x%x]\n", UINT32_MAX, UINT32_MAX);
	printf("UINT16_MAX:[%d][0x%x]\n", UINT16_MAX, UINT16_MAX);
	printf("UINT8_MAX:[%d][0x%x]\n", UINT8_MAX, UINT8_MAX);

	printf("INT64_MIN:[%ld][0x%lx]\n", INT64_MIN, INT64_MIN);
	printf("INT32_MIN:[%d][0x%x]\n", INT32_MIN, INT32_MIN);
	printf("INT16_MIN:[%d][0x%x]\n", INT16_MIN,INT16_MIN);
	printf("INT8_MIN:[%d][0x%x]\n", INT8_MIN, INT8_MIN);

	printf("float32 MAX:[%f]\n", FLT_MAX);
	printf("double 64 MAX:[%f]\n", DBL_MAX);

	return 0;
}
