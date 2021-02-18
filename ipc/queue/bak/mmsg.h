#ifndef __MMSG_H__
#define __MMSG_H__

#define MSG_KEY		1010
#define MSG_TYPE	10
#define MSG_MAX_LEN	300	
#define MSG_MAX_NUM	200

struct msgbuf{
	long mtype;
	char mtext[MSG_MAX_LEN];
};

int msg_queue_init(void);
int msg_queue_receive(int msgid, struct msgbuf *msg);
int msg_queue_send(int msgid, char *str);

#endif
