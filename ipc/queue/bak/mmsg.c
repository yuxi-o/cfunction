#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#include "mmsg.h"

static void msg_queue_stat(int msgid, struct msqid_ds *stat)
{
	if(msgctl(msgid, IPC_STAT, stat)){
		printf("msgctl stat error(%d): %s", errno, strerror(errno));
		return;
	}
	printf("msg queue status: %ld bytes, %ld messages(max bytes: %ld)", 
			stat->__msg_cbytes, stat->msg_qnum, stat->msg_qbytes);
}

int msg_queue_init()
{
	int msgid;
	struct msqid_ds stat;

	msgid = msgget(MSG_KEY, IPC_CREAT| 0666);
	if(msgid < 0){
		printf("msgget error(%d): %s", errno, strerror(errno));
		return -1;
	}

	msg_queue_stat(msgid, &stat);
	stat.msg_qbytes = MSG_MAX_NUM * MSG_MAX_LEN;
	if(msgctl(msgid, IPC_SET, &stat)){
		printf("msgctl stat error(%d): %s", errno, strerror(errno));
		return -1;
	}
	return msgid;
}

int msg_queue_receive(int msgid, struct msgbuf *msg)
{
	struct msqid_ds stat;

	while(msgrcv(msgid, (void *)msg, sizeof(msg->mtext), MSG_TYPE, MSG_NOERROR) < 0){
		if(errno == EINTR){
			continue;
		}
		msg_queue_stat(msgid, &stat);
		printf("msgctl recv error(%d): %s, and should exit", errno, strerror(errno));
		return -1;
	}

	return 0;
}

int msg_queue_send(int msgid, char *str)
{
	struct msgbuf msg;
	struct msqid_ds stat;
	int len;

	len = strlen(str);
	if(len >= MSG_MAX_LEN){
		printf("msg length (%d) is more then max length(%d)", len, MSG_MAX_LEN);
		len = MSG_MAX_LEN - 1;
	}
	
	msg.mtype = MSG_TYPE;
	strncpy(msg.mtext, str, len);
	msg.mtext[len] = 0;
	while(msgsnd(msgid, (void *)&msg, MSG_MAX_LEN, IPC_NOWAIT) < 0){
		if(errno == EINTR){
			continue;
		}

		msg_queue_stat(msgid, &stat);
		printf("msgsen error(%d): %s", errno, strerror(errno));
		return -1;
	}

	return 0;
}

#if 1
int main(int argc, char *argv[])
{
	int msgid;
	struct msgbuf msg;

	msgid = msg_queue_init();
	if(msgid < 0){
		return -1;
	}

	msg_queue_send(msgid, "Hello world");

	if(msg_queue_receive(msgid, &msg) <0){
		return -1;
	}

	printf("receive msg: %s\n", msg.mtext);
	return 0;
}

#endif
