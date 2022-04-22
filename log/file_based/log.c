#include <stdio.h>  
#include <stdarg.h>  
#include <time.h>
#include <pthread.h>

#include "log.h"

FILE *g_log_fp = NULL;
int g_log_level = 0;

static pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;


int log_init(const char *file, int level)
{
	g_log_fp = fopen(file, "a+");
	if(g_log_fp == NULL){
		printf("log %s open failed\n", file);
		return -1;
	}

	g_log_level = level;
	MYLOG_INFO("log init level %d...", level);

	return 0;
}

void log_close()
{
    fflush(g_log_fp);
	fclose(g_log_fp);
}

void my_log(char* loglvl, char* file, int line, char* function, const char *format, ...)
{
    va_list arg;  

    pthread_mutex_lock(&fileMutex);
    va_start(arg, format);  
  
    time_t time_log = time(NULL);  
    struct tm* tm_log = localtime(&time_log);  
    fprintf(g_log_fp, "[%04d-%02d-%02d %02d:%02d:%02d]%s%s(%d)-%s(): ", tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday, 
        tm_log->tm_hour, tm_log->tm_min, tm_log->tm_sec, loglvl, file, line, function);  
  
    vfprintf(g_log_fp, format, arg); 
    fprintf(g_log_fp, "\n"); 
    va_end(arg);  

    fflush(g_log_fp);
    pthread_mutex_unlock(&fileMutex);  
    return;      
}


void print_byte(char* p, int num)
{
    int i = 0;
    for(; i<num; i++)
    {
        fprintf(g_log_fp, "%02X", p[i]);
    }
    fprintf(g_log_fp, "\n");

    return;
}
