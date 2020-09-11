#include <stdio.h>
#include <unistd.h>
#include "msg_queue.h"

int main()
{
    int msqid;
    int ret = 0;
    struct buf{
        long type;
        int op;
    } buf;
    msqid = msq_init(100);
    buf.type = 100;
    buf.op = 111;
    ret = msq_send(msqid, (void *)&buf, sizeof(int));
    if(ret < 0) {
        printf("send error\n");
        return -1;
    }
    ret = msq_recv_timeout(msqid, &buf, sizeof(int), 100, 1);
    if(ret < 0) {
        printf("recv error\n");
        return -1;
    }
    printf("%d\n", buf.op);
    return 0;
}

