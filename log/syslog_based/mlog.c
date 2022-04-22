#include <stdio.h>
#include <syslog.h>
#include <stdarg.h>
#include <string.h>

#include "config.h"
#include "merror.h"

void msyslog(int level, int priority, const char *format, ...)
{
	va_list ap;
	char buf[1024];
	int offset;

	if(level > MODULE_LOG_LEVEL){
		return;
	}

	offset = snprintf(buf, sizeof(buf), "##[%s][%s]##", PRODUCT_NAME, MODULE_NAME);
	openlog(NULL, 0, LOG_USER);
	va_start(ap, format);
	offset = vsprintf(buf+offset, format, ap);
	va_end(ap);
	syslog(priority, "%s", buf);
	closelog();
}

void mprintf(int level, int err_num, const char *format, ...)
{
	va_list ap;
	if(level > MODULE_LOG_LEVEL){
		return;
	}

	va_start(ap, format);
	printf("##[%s][%s]##", PRODUCT_NAME, MODULE_NAME);
	vprintf(format, ap);
	if(err_num > 0){
		printf(":%s", merror_msg[err_num]);
	}
	printf("\n");
	va_end(ap);

}

void msyslog_printf_error(int level, int err_num, const char *format, ...)
{
	va_list ap;
	char buf[1024];
	int offset, left, len;

	if(level > MODULE_LOG_LEVEL){
		return;
	}

	offset = sprintf(buf, "##[%s][%s]##", PRODUCT_NAME, MODULE_NAME);
	openlog(NULL, 0, LOG_USER);
	va_start(ap, format);
	offset = vsprintf(buf+offset, format, ap);
	va_end(ap);

	if(err_num > 0){
		left = sizeof(buf) - offset;
		if(left > 1){
			strcat(buf+offset, ":");
			len = strlen(merror_msg[err_num]);
			strncat(buf+offset+1, merror_msg[err_num], len<left?len:left);
		}
	}

	syslog(LOG_ERR, "%s", buf);
	closelog();
	printf("%s\n", buf);
}


