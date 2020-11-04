#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>

//---------------------------log process start--------------------------------
#define PRODUCT_NAME	"work"
#define MODULE_NAME		"log"
enum {
	DDUMMY		= 0,
	DRELEASE	= 1,
	DDEBUG		= 2,
};
int g_log_level = DDEBUG;

void msyslog(int priority, const char *format, ...)
{
	va_list ap;
	char buf[1024];
	int offset;

	offset = snprintf(buf, sizeof(buf), "##[%s][%s][%s]##", PRODUCT_NAME, MODULE_NAME, __FILE__);
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
	if(level <= g_log_level){
		if(level == DDUMMY){
			return;
		}

		va_start(ap, format);
		printf("##[%s][%s][%s]##", PRODUCT_NAME, MODULE_NAME, __FILE__);
		vprintf(format, ap);
		if(err_num > 0){
			//printf(":%s\n", err_msg[err_num]);
			printf(":%d\n", err_num);
		}
		va_end(ap);
	}
}

void msyslog_printf_error(int err_num, const char *format, ...)
{
	va_list ap;
	char buf[1024];
	int offset;

	offset = snprintf(buf, sizeof(buf), "##[%s][%s][%s]##", PRODUCT_NAME, MODULE_NAME, __FILE__);
	openlog(NULL, 0, LOG_USER);
	va_start(ap, format);
	offset = vsprintf(buf+offset, format, ap);
	va_end(ap);
#if 0
	if(err_num > 0){
		//printf(":%s\n", err_msg[err_num]);
		if(offset < sizeof(buf)){
			strncat(buf+offset, err_msg[err_num], sizeof(buf)-offset)
		}
	}
#endif
	syslog(LOG_ERR, "%s", buf);
	closelog();
}
//---------------------------log process end--------------------------------

int main(void)
{
	msyslog_printf_error(0, "Open file %s failed!\n", __FILE__);
	mprintf(DRELEASE, 0, "Cmd exec error: %s\n", strerror(1));
	msyslog(LOG_WARNING, "Cmd return erro: %s", strerror(2));
	return 0;
}
