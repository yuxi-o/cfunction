#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(2) //设置2字节对齐
struct student 
{
	unsigned char anticollision_front:1;
	unsigned char anticollision_back:1;
	unsigned char charge_detect:1;
	unsigned char sw_a:1;
	unsigned char sw_b:1;
	short no;
	int id;
	char name[];
};

union endian
{
	int id;
	char cc[4];
}end;

int main()
{
	printf("sizeof int is %ld\n", sizeof(int));
	printf("sizeof struct student is %ld\n", sizeof(struct student));
/*
	struct student st;
	st.no = 1;
	st.id = 1;
	st.name =(char *)malloc(6);
	if(st.name == NULL){
		printf("malloc error!\n");
		exit(-1);
	}
	strcpy(st.name, "WORLD");
	printf("st.no[%d], st.id[%d], st.name[%s]\n", st.no, st.id, st.name);
*/
	struct student *pst;
	int ssize = sizeof(struct student) + 6*sizeof(char);
//	pst = (struct student*)malloc(sizeof(struct student) + 6*sizeof(char));
	pst = (struct student*)malloc(ssize);
	if(pst == NULL){
		printf("malloc error!\n");
		exit(-1);
	}
	pst->no = 1;
	pst->id = 1;
	pst->anticollision_front=0;
	pst->anticollision_back=1;
	pst->charge_detect = 1;
	pst->sw_a = 0;
	pst->sw_b = 1;
	strcpy(pst->name, "WORLD");
	printf("pst->no[%d], pst->id[%d], pst->name[%s]\n", pst->no, pst->id, pst->name);
	printf("pst->anticollision_front[%u], pst->anticollision_back[%u], pst->charge_detect[%u], pst->sw_a[%u], pst->sw_b[%u]\n", pst->anticollision_front, pst->anticollision_back, pst->charge_detect, pst->sw_a, pst->sw_b);
	printf("sizeof struct student is %ld\n", sizeof(struct student));
	
	int i;
	char *pc = (char *)pst;
	for(i = 0; i <ssize;){
		printf("%.2X ", (char)pc[i]&0xFF);
		i++;
		if(i % 8 == 0){
			printf("\n");
		}
	}
	printf("\n");

	struct student *pst1 = (struct student*)malloc(ssize);
	if(pst1 == NULL){
		printf("malloc error!\n");
		exit(-1);
	}
	memcpy(pst1, pst, ssize);
	pc = (char *)pst1;
	for(i = 0; i <ssize;){
		printf("%.2X ", (char)pc[i]&0xFF);
		i++;
		if(i % 8 == 0){
			printf("\n");
		}
	}
	printf("\n");

	end.id = 1;
	printf("cc[0123]=1 ==> [%X %X %X %X]\n", end.cc[0], end.cc[1], end.cc[2], end.cc[3]);
	free(pst);
	return 0;
}

