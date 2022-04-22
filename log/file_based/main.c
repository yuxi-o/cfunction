#include <stdio.h>
#include <stdlib.h>
#include "log.h"

#define LOG_FILE_PATH	"/home/wang/app/"
#define LOG_FILE		"exe.log"

int main(int argc, char *argv[])
{
	int ret = 0;

	if(argc != 2){
		printf("Usage: %s level\n", argv[0]);
		printf("----log file: "LOG_FILE_PATH LOG_FILE);
		return -1;
	}

	ret = log_init(LOG_FILE_PATH LOG_FILE, atoi(argv[1]));
	if(ret < 0){
		return -1;
	}

	MYLOG_DEBUG("[l%s] running %s", argv[1], argv[0]);
	MYLOG_INFO("[l%s] running %s", argv[1], argv[0]);
	MYLOG_ERROR("[l%s] running %s", argv[1], argv[0]);

	log_close();

	return 0;
}

