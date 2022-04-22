#ifndef __MLOG_H__
#define __MLOG_H__

void msyslog(int priority, const char *format, ...);
void mprintf(int level, int err_num, const char *format, ...);
void msyslog_printf_error(int level, int err_num, const char *format, ...);

#endif
