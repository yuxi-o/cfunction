#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "mat.h"

#define AT_MAX_LEN	64

char *msim_state[]={
	"SIM_ZERO",
	"SIM_INIT[AT]",
	"SIM_READY[AT+CPIN?]",
	"SIM_SIGNAL[AT+CSQ]",
	"SIM_REGISTER[AT+CREG?,AT+COPS?]"
};

int mat_uart_init(mat_uart_t *mat, char *dev)
{
	if(!mat){
		return -1;
	}

	memset(mat, 0, sizeof(mat_uart_t));
	if(muart_open(&(mat->muart), dev) < 0){
		printf("AT: open uart %s error\n", dev);
		return -1;
	} 
	if(muart_setup(&mat->muart, 115200, 8, 1, 'N') < 0){
		muart_close(&mat->muart);
		printf("AT: setup uart %s(115200, 8N1) error\n", dev);
		return -1;
	}

	mat->msim.state = SIM_ZERO;

	return 0;
}

int mat_uart_deinit(mat_uart_t *mat)
{
	muart_close(&mat->muart);
	mat->msim.state = SIM_ZERO;
	return 0;
}

static int send_at_cmd(muart_t *m, char *at_cmd, char *expect, char *msg,
		int msg_size, int timeout){

	int nbytes = 0;
	char buf[AT_MAX_LEN] = {0};

	if(at_cmd == NULL || expect == NULL){
		return -1;
	}

	if(muart_write(m, at_cmd, strlen(at_cmd)) < 0){
		printf("AT: write AT(%s) error: %s\n", at_cmd, strerror(errno));
		return -1;
	}

//	if(muart_read_timeout(m, buf, sizeof(buf), timeout) <= 0){
	if(muart_read_nbytes(m, buf, sizeof(buf), timeout) <= 0){
		printf("AT: read AT(%s) reply error: %s\n", at_cmd, strerror(errno));
		return -1;
	}

//	printf("AT: recv {{%s}}\n", buf);
	if(strstr(buf, expect) == NULL){
		return -1;
	}

	if(msg){
		nbytes = sizeof(buf);
		buf[nbytes-1] = 0;

		if(msg_size > 0){
			nbytes = (msg_size>nbytes)? (nbytes) : (msg_size);
			strncpy(msg, buf, nbytes);
		}
	}

	return 0;
}

static int check_at_ok(muart_t *m)
{
	int ret = 0;

	ret = send_at_cmd(m, "AT\r", "OK", NULL, 0, 1);
	if(ret != 0){
//		printf("AT: send AT no reply\n");
		return -1;
	}

	return 0;
}

static int check_sim_ready(muart_t *m)
{
	int ret = 0;
	
	ret = send_at_cmd(m, "AT+CPIN?\r", "READY", NULL, 0, 1);
	if(ret != 0){
//		printf("AT: No SIM detected\n");
		return ret;
	}

	return 0;
}

static int check_sim_signal(muart_t *m, msim_t *ms)
{
	int ret = 0, i = 0;
	char buf[AT_MAX_LEN] = {0};
	int signal = 0, tmp = 0;

	ret = send_at_cmd(m, "AT+CSQ\r", "+CSQ", buf, sizeof(buf), 1);
	if(ret < 0){
//		printf("AT: Can't check signal\n");
		return -1;
	}
	
	buf[63] = 0;
	for(i = 0; i < strlen(buf); i++){
		if(buf[i] == ','){
			tmp = buf[i-1] - '0';
			if((tmp >= 0) && (tmp <= 9)){
				signal = tmp;
			}
			tmp = buf[i-2] - '0';
			if((tmp >= 0) && (tmp <= 9)){
				signal += tmp * 10;
			}
			break;
		}
	}

	if(signal < 7 || signal == 99){
		printf("AT: signal(%d) is too low\n", signal);
		return -1;
	}
	
	ms->signal = signal;
	return 0;
}

static int check_sim_register(muart_t *m)
{
	int ret = 0;
	char buf[AT_MAX_LEN] = {0};

	ret = send_at_cmd(m, "AT+CREG?\r", "OK", buf, sizeof(buf), 1);
	if(ret < 0){
//		printf("AT: SIM is not registered\n");
		return -1;
	}

	buf[63] = 0;
	if((strstr(buf, "0,1") == NULL) && (strstr(buf, "0,5") == NULL)){
		printf("AT: SIM register error:%s\n", buf);
		return -2;
	}

	return 0;
}

static int check_sim_operator(muart_t *m, msim_t *ms)
{
	int ret = 0;
	char buf[AT_MAX_LEN] = {0};
	char *pstart = NULL, *pend = NULL;

	ret = send_at_cmd(m, "AT+COPS?\r", "OK", buf, sizeof(buf), 1);
	if(ret < 0){
//		printf("AT: Can't check operator\n");
		return -1;
	}

	buf[63] = 0;
	pstart = strstr(buf, "\"");
	if(pstart == NULL){
		printf("AT: get operator error:%s\n", buf);
		return -1;
	}

	pstart++;
	pend = strstr(pstart, "\"");
	*pend = 0;

	strncpy(ms->operator, pstart, pend - pstart + 1);
	return 0;
}

static int mat_uart_check_sim_all(mat_uart_t *mat)
{
	if(!mat){
		return -1;
	}

	muart_t *m = &mat->muart;
	msim_t *ms = &mat->msim;

	if(ms->state < SIM_READY){
		if(check_at_ok(m) < 0){
			goto msim_reset;
		}

		if(check_sim_ready(m) < 0){
			goto msim_reset;
		}

		if(check_sim_register(m) < 0){
			goto msim_reset;
		}

		if(check_sim_operator(m, ms) < 0){
			goto msim_reset;
		}
	}

	if(check_sim_signal(m, ms) < 0){
		goto msim_reset;
	}
#if 0	
	if(check_sim_register(m) < 0){
		goto msim_reset;
	}
	if(check_sim_operator(m, ms) < 0){
		goto msim_reset;
	}
#endif
	ms->state = SIM_REGISTER;

	return 0;

msim_reset:
	printf("AT: get next state(%s) error!\n", msim_state[ms->state]);
	ms->state = SIM_ZERO;
	return -1;
}

void mat_uart_show_sim(mat_uart_t *mat)
{
	if(!mat){
		return;
	}

	printf("AT: state:[%s], signal:%d, operator:%s\n", 
		msim_state[mat->msim.state], mat->msim.signal, mat->msim.operator);
}

int mat_uart_get_signal(mat_uart_t *mat)
{
	if(!mat){
		return -1;
	}

	mat_uart_check_sim_all(mat);

	return mat->msim.signal;
}

int mat_uart_get_operator(mat_uart_t *mat, char *operator, int operator_size)
{
	if(!mat){
		return -1;
	}

	int len = 0;

	if(!mat->msim.operator[0]){
		operator[0] = 0;
		return -2;
	}

	len = strlen(mat->msim.operator) + 1;
	len = (operator_size > len)? len:operator_size;
	strncpy(operator, mat->msim.operator, len);
	return 0;
}

int mat_uart_get_sim(char *dev, int *signal, char *operator, int operator_size)
{
	int ret = 0, len = 0;
	mat_uart_t mat;

	ret = mat_uart_init(&mat, dev);
	if(ret < 0){
		return -1;
	}

	ret = mat_uart_check_sim_all(&mat);
	if(ret < 0){
		mat_uart_deinit(&mat);
		return -1;
	}

	*signal = mat.msim.signal;

	len = strlen(mat.msim.operator) + 1;
	len = (operator_size > len)?len:operator_size;
	strncpy(operator, mat.msim.operator, len);

	mat_uart_deinit(&mat);
	return 0;
}

