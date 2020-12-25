#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <mosquitto.h>
#include <time.h>
#include <signal.h>

char peerid[] = "peerid_wang";
//char host[] = "192.168.3.122";
//int port = 1883;
char host[] = "server";
int port = 8883;
char cafile[] = "/tmp/cert/ca.crt";
char client_cert[] = "/tmp/cert/client.crt";
char client_key[] = "/tmp/cert/client.key";
int keepalive = 10;
int send_qos = 1;
bool clean_session = true; // need to subscribe per connect if clean session flag is true
struct mosquitto *mosq = NULL;
pthread_t pmosid = 0;
static int mid_send = -1;
volatile char flag_stop = 0;
volatile char flag_connect = 0;

void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    if(message->payloadlen){
        printf("====>recv:[%s](%d)%s\n", message->topic, message->mid, (char *)message->payload);
    }else{
        printf("====>recv:[%s](%d)(null)\n", message->topic, message->mid);
    }
}

// Be called when the message has been sent to the broker successfully
void my_publish_callback(struct mosquitto *mosq, void *userdata, int mid)
{
	printf("[%ld]publish callback mid %d\n", time(NULL), mid);
}

void my_disconnect_callback(struct mosquitto *mosq, void *userdata, int rc)
{
	printf("[%ld]disconnect callback rc %d\n", time(NULL), rc);
	flag_connect = 0;
}

void my_connect_callback(struct mosquitto *mosq, void *userdata, int rc)
{
	printf("[%ld]connect callback rc %d\n", time(NULL), rc);
	if(!rc){
		// 订阅发布的topic，这样消息发送后，会从broker再发送过来
		// 适用于消息机制，订阅机制，可准确知道是否发送成功
		mosquitto_subscribe(mosq, NULL, "wang/publish", 1);
		flag_connect = 1;
	}
}

void mos_init()
{
    mosquitto_lib_init();
    mosq = mosquitto_new(peerid, clean_session, NULL);
    if(!mosq){
        fprintf(stderr, "Error: Out of memory.\n");
        exit(-1);
    }

    mosquitto_message_callback_set(mosq, my_message_callback);
	mosquitto_publish_callback_set(mosq, my_publish_callback);
	mosquitto_connect_callback_set(mosq, my_connect_callback);
	mosquitto_disconnect_callback_set(mosq, my_disconnect_callback);
    mosquitto_will_set(mosq,"wang/will", sizeof("livewill"), "livewill", 2, false);
	// Used to tell the library that your application is using threads, 
	// but not using mosquitto_loop_start. 
	//mosquitto_threaded_set(mosq, 1);
	mosquitto_username_pw_set(mosq, "wang", "wangqh");
	mosquitto_tls_set(mosq, cafile, NULL, client_cert, client_key, NULL);
}

void cfinish(int sig)
{
	signal(SIGINT, NULL);
	flag_stop = 1;
}

int main(int argc, char *argv[])
{
    int ret = 0;
	int toserver = -1;
    int timeout = 0;
	char buf[16];
    
	signal(SIGINT, cfinish);
    mos_init();
    while(toserver){
        toserver = mosquitto_connect(mosq, host, port, keepalive);
        if(toserver){
            timeout++;
            fprintf(stderr, "Unable to connect server [%d] times.\n", timeout);
            if(timeout > 3){
                fprintf(stderr, "Unable to connect server, exit.\n" );
				exit(-1);
            }
            sleep(10);
        }
    }

//	mosquitto_subscribe(mosq, NULL, "wang/publish", 1);
	ret = mosquitto_loop_start(mosq);
	if(ret != MOSQ_ERR_SUCCESS){
		printf("loop start error: %s\n", mosquitto_strerror(ret));
		goto clean;
	}

    while(!flag_stop){
		ret = 0;
		sprintf(buf, "%ld", time(NULL));
		strcat(buf, "love");
		if(flag_connect){
			ret = mosquitto_publish(mosq, &mid_send, "wang/publish", strlen(buf), buf, send_qos, false);
			printf("[%ld]push [%s] mid %d:%s(%d)\n", time(NULL), buf, mid_send, mosquitto_strerror(ret), ret);
		}

		if(!flag_connect || ret){
			printf("Message publish failed, need to store!\n");
		}
		
        sleep(5);
    }

	mosquitto_disconnect(mosq);
	mosquitto_loop_stop(mosq, true);
	printf("mosquitto finished\n");
clean:
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}

