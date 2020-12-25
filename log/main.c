#include <string.h>
#include <syslog.h>

#include "config.h"
#include "mlog.h"

int main(void)
{
	msyslog_printf_error(DRELEASE, 0, "Open file %s failed!", __FILE__);
	mprintf(DRELEASE, 0, "Cmd exec error: %s", strerror(1));
	msyslog(LOG_WARNING, "Cmd return erro: %s", strerror(2));
	return 0;
}
