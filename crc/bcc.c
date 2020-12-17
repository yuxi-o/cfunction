#include <stdio.h>
#include <string.h>

// 异或校验  xor
char bcc(char *buf, int len)
{
	int i;
	char crc = 0;

	for(i = 0; i < len; i++){
		crc ^= buf[i];
	}

	return crc;
}

int main(int argc, char *argv[])
{
	if(argc != 2){
		printf("Usage: %s string", argv[0]);
		printf("ex:\nGNGGA,031211.104,,,,,0,0,,,M,,M,,\t\t53\n");
		printf("GNRMC,031211.104,V,,,,,0.00,0.00,060180,,,N\t\t59\n");
		printf("GNVTG,0.00,T,,M,0.00,N,0.00,K,N\t\t2C\n");
		return -1;
	}

	int len = strlen(argv[1]);

	printf("crc=[0x%x]\n", bcc(argv[1], len));
	
	return 0;
}
