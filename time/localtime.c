#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char date_mday[]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
void utc2btc(struct tm *mtm)
{
	int year;

	mtm->tm_hour += 8;
	if(mtm->tm_hour >= 24){
		mtm->tm_hour -=24;
		mtm->tm_mday +=1;
		if(mtm->tm_mon == 1){
			year = mtm->tm_year + 1900;
			if((year%400 == 0) || ((year%4 == 0) && (year%100 != 0))){
				date_mday[1] = 29;
			}
		}
		if(mtm->tm_mday > date_mday[mtm->tm_mon]){
			mtm->tm_mday = 1;
			mtm->tm_mon += 1;
			if(mtm->tm_mon > 11){
				mtm->tm_mon = 0;
				mtm->tm_year += 1;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	struct tm *mtm;
	time_t tt;

	printf("Usage: %s timestamp\n", argv[0]);
	printf("ex. \t1582908645\t1614531045\t1609433445\n");

	if(argc > 1){
		tt = atol(argv[1]);
	} else {
		tt = time(NULL);
	}
	
	mtm = gmtime(&tt);
	printf("UTC: \t%04d.%02d.%02d %02d:%02d:%02d\n", mtm->tm_year + 1900, mtm->tm_mon+1, mtm->tm_mday, 
			mtm->tm_hour, mtm->tm_min, mtm->tm_sec);

	utc2btc(mtm);
	printf("BTC: \t%04d.%02d.%02d %02d:%02d:%02d\n", mtm->tm_year + 1900, mtm->tm_mon+1, mtm->tm_mday, 
			mtm->tm_hour, mtm->tm_min, mtm->tm_sec);

	mtm = localtime(&tt);
	printf("localtime: %04d.%02d.%02d %02d:%02d:%02d\n", mtm->tm_year + 1900, mtm->tm_mon+1, mtm->tm_mday, 
			mtm->tm_hour, mtm->tm_min, mtm->tm_sec);

	return 0;
}
