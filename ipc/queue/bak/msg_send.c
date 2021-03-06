#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <errno.h>

#define MSG_KEY		1010
#define MSG_TYPE	10 
#define MSG_MAX_LEN	256
#define MSG_MAX_NUM	200

struct msgbuf {
	long mtype;
	char mtext[MSG_MAX_LEN];
};

int main(int argc, char *argv[])
{
	struct msgbuf msg;
	struct msqid_ds stat;
	int msgid;

	msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
	if(msgid < 0){
		printf("msgget error(%d): %s\n", errno, strerror(errno));
		exit(-1);
	}
	
	if( msgctl(msgid, IPC_STAT, &stat) == 0){
		printf("msg queue status: [%ld] bytes, [%ld] messages (Max bytes: [%ld]).\n", stat.__msg_cbytes, stat.msg_qnum, stat.msg_qbytes);
	} else {
		printf("msgctl stat error(%d): %s\n", errno, strerror(errno));
	}

	while(1){
		msg.mtype = MSG_TYPE;
		snprintf(msg.mtext, MSG_MAX_LEN, "ts:%ld", time(NULL));
		if(msgsnd(msgid, (void *)&msg, MSG_MAX_LEN, IPC_NOWAIT) < 0){
			printf("msgsnd error: %s\n", strerror(errno));
			if( msgctl(msgid, IPC_STAT, &stat) == 0){
				printf("msg queue status: [%ld] bytes, [%ld] messages (Max bytes: [%ld]).\n", stat.__msg_cbytes, stat.msg_qnum, stat.msg_qbytes);
			} else {
				printf("msgctl stat error(%d): %s\n", errno, strerror(errno));
			}
		}
		sleep(1);
	}

	return 0;
}

