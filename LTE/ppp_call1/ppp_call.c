#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define DNS_FILE	"/etc/resolv.conf"
#define LINK_PATH	"/etc/ppp"
#define PPP_PATH	"/usr/local/etc/ppp"
#define PPP_LINK_FILE	"/tmp/link.ppp0"
#define PPP_DNS_FILE	"/etc/ppp/resolv.conf"

#define PPP_OPTIONS_FILE	"/usr/local/etc/ppp/gosuncn_options"
#define PPP_DIALER_FILE		"/usr/local/etc/ppp/gosuncn_ppp_dialer"
#define ECM_DIALER_FILE		"/usr/local/etc/ppp/gosuncn_ecm_dialer"

//#define PPP_DIALER_CMD	"pppd file %s connect \"chat -v -f %s\""   
#define PPP_CHAT_CMD	"/usr/bin/chat -f %s"   
#define PPP_KILL_CMD	"killall pppd"

#define PPP_START_TIMEOUT	120

#define PING_COUNT	5
#define PING_CMD	"ping -q -w 2 -c 1 114.114.114.114 >/dev/null 2>&1"

enum {
	PPPD_STATUS_STOP = 0x1,
	PPPD_STATUS_START = 0x2,
	PPPD_STATUS_ONLINE = 0x4,
};

static pid_t pppd_id = 0;
static int pppd_link_num = 0;
volatile static int pppd_status = 0;
static char pppd_ping_on = 1;

int pppd_exec(pid_t *pid)
{
	int ret = 0;
	char cbuf[64] = {0};
	char *cmd_argv[6] = {0};

	cmd_argv[0] = "/usr/bin/pppd";
	cmd_argv[1] = "file";
	cmd_argv[2] = PPP_OPTIONS_FILE;
	cmd_argv[3] = "connect";
	sprintf(cbuf, PPP_CHAT_CMD, PPP_DIALER_FILE);
	cmd_argv[4] = cbuf; 
	cmd_argv[5] = NULL;

	*pid = fork();
	if(*pid == 0){
		ret = execvp(cmd_argv[0], cmd_argv);
		if(ret < 0){
			printf("exec pppd failed: %s\n", strerror(errno));
			exit(-1);
		}
	} else if(*pid < 0){
		printf("pppd fork error: %s\n", strerror(errno));
		return -1;
	}
	
	return 0;
}

int pppd_prepare()
{
	int ret = 0;

	unlink(LINK_PATH);
	ret = symlink(PPP_PATH, LINK_PATH);
	if(ret){
		printf("symlink error: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

int pppd_stop()
{
	system("killall ip-up");
	system("killall ip-down");
	sleep(1);

	system("killall -SIGHUP pppd");
	system("killall pppd");

	unlink(DNS_FILE);
	unlink(PPP_LINK_FILE);

	return 0;
}

int pppd_start()
{
	int fd = 0, ret = 0;
	int timeout = PPP_START_TIMEOUT;
	char cbuf[8];

	system("killall pppd");
	sleep(1);

	pppd_id = 0;
	
	ret = pppd_exec(&pppd_id);
	if(ret < 0){
		return -1;
	}

	while(timeout > 0){
		if(access(PPP_LINK_FILE, F_OK) == 0){
			break;
		}
		timeout--;
		sleep(1);
		if(pppd_status & PPPD_STATUS_STOP){
			return -1;
		}
	}

	if(timeout <= 0){
		printf("%s not exist within %d\n", PPP_LINK_FILE, PPP_START_TIMEOUT);
		return -1;
	}

	if(++pppd_link_num >= 10000){
		pppd_link_num = 0;
	}

	sprintf(cbuf, "%d\n", pppd_link_num);
	
	fd = open(PPP_LINK_FILE, O_WRONLY);
	if(fd < 0){
		printf("open %s error: %s\n", PPP_LINK_FILE, strerror(errno));
		return -1;
	}
	write(fd, cbuf, strlen(cbuf));
	close(fd);

	printf("pppd (%d) start successfully\n", pppd_id);
	return 0;
}

//---------------------------------------------------------------------
static int fatal_signals[] = {
	SIGQUIT,
	SIGILL,
	SIGABRT,
	SIGFPE,
	SIGPIPE,
	SIGBUS,
};

void fatal_signal_handler(int sig)
{
	char *msg = NULL;
	
	switch(sig){
		case SIGQUIT: msg = "QUIT"; break;
		case SIGILL: msg = "Illegal instructiona"; break;
		case SIGABRT: msg = "Abort"; break;
		case SIGFPE: msg = "Float point exception"; break;
		case SIGPIPE: msg = "Broken pipe"; break;
		case SIGBUS: msg = "Bud error"; break;
	}

	if (msg){
		printf("catch signal(%d): %s\n", sig, msg);
	} else {
		printf("catch signal(%d)\n", sig);
	}
}

void child_signal_handler(int sig)
{
	pid_t pid;
	int status;

	while((pid = waitpid(-1, &status, WNOHANG)) > 0){
		pppd_status = PPPD_STATUS_STOP;
		printf("Child %d receive signal %d", pid, sig);
		if(WIFEXITED(status)){
			printf(" and exit with %d\n", WEXITSTATUS(status));
		} else if (WIFSIGNALED(status)){
			printf(" and killed with %d\n", WTERMSIG(status));
		}
	} 
}

void init_signal(void)
{
	int i;
	
	for(i = 0; i < sizeof(fatal_signals)/sizeof(fatal_signals[0]); i++){
		signal(fatal_signals[i], fatal_signal_handler);
	}

	signal(SIGCHLD, child_signal_handler);
}
//---------------------------------------------------------------------

void reset_module()
{
	system("echo \"0\" > /sys/class/leds/gpio1/brightness");
	sleep(10);
	system("echo \"1\" > /sys/class/leds/gpio1/brightness");
	sleep(30);
}

int ping()
{
	return system(PING_CMD);
}

int main(int argc, char *argv[])
{
	int error_num = 0;

	init_signal();
	pppd_prepare();

	pppd_status = PPPD_STATUS_START;
	while(1){
		if(pppd_status & PPPD_STATUS_STOP){
			pppd_stop();
			pppd_status = PPPD_STATUS_START;
			sleep(1);
			reset_module();
		}
		if(pppd_status & PPPD_STATUS_START){
			if(pppd_start()){
				pppd_status = PPPD_STATUS_STOP;
			} else {
				pppd_status &= ~PPPD_STATUS_START;
				pppd_status |= PPPD_STATUS_ONLINE;
			}
		}

		if(pppd_status == PPPD_STATUS_ONLINE){
			if(pppd_ping_on && ping()){
				error_num++;
				if(error_num > PING_COUNT){
					printf("pppd ping(%s) error %d times, and reset 4G network\n",
							PING_CMD, PING_COUNT);
					pppd_status = PPPD_STATUS_STOP;
					error_num = 0;
				}
			} else {
				error_num = 0;
			}
		}

//		printf("pppd status : 0x%x\n", pppd_status);
		sleep(2);
	}
	
//	system("route del default");
//	system("route add -net 0.0.0.0 ppp0");

	return 0;
}

