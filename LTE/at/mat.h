#ifndef __MAT_H__
#define __MAT_H__

#include "muart.h" 

enum {
	SIM_ZERO,
	SIM_INIT,
	SIM_READY,
	SIM_SIGNAL,
	SIM_REGISTER,
};

typedef struct msim_struct{
	int		state;
	int		signal;
	char	operator[16]; 
} msim_t;

typedef struct mat_uart_struct{
	muart_t muart;
	msim_t	msim;
} mat_uart_t;


int mat_uart_init(mat_uart_t *mat, char *dev);
int mat_uart_deinit(mat_uart_t *mat);
void mat_uart_show_sim(mat_uart_t *mat);
//int mat_uart_check_sim_all(mat_uart_t *mat);
int mat_uart_get_signal(mat_uart_t *mat);
int mat_uart_get_operator(mat_uart_t *mat, char *operator, int operator_size);
int mat_uart_get_sim(char *dev, int *signal, char *operator, int operator_size);

#endif

