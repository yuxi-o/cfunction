#include <stdio.h>

char *itoa(int num, char *str, int radix)
{
	char index[]="0123456789ABCDEF";
	unsigned unum;
	int i=0, j, k;
	char tmp;

	if(radix == 10 && num < 0){
		unum = (unsigned)-num;
		str[i++]='-';
	} else {
		unum = (unsigned)num;
	}

	do{
		str[i++]=index[unum%(unsigned)radix];
		unum /= radix;
	} while(unum);
	str[i] = '\0';

	if(str[0] == '-'){
		k = 1;
	} else {
		k = 0;
	}

	for(j = k; j <= (i-1)/2; j++){
		tmp = str[j];
		str[j] = str[i-1+k-j];
		str[i-1+k-j] = tmp;
	}

	return str;
}

int main(int argc, char *argv[])
{
	char cbuf[16] = {0};

	printf("decimal 10009856 is %s\n", itoa(10009856, cbuf, 10));
	printf("decimal -10009856 is %s\n", itoa(-10009856, cbuf, 10));
	printf("hex 0xFFFF0000 is %s\n", itoa(0xFFFF0000, cbuf, 16));

	return 0;
}

