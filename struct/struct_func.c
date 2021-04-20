// struct中函数占用字节数
// 32位主机占用4个字节，64位主机占用8个字节
//

#include <stdio.h>

struct student_st {
	char name[20];
	int no;
	void (*show)(struct student_st);
	void (*show1)(struct student_st);
};

struct func_st{
	void (*print)();
};

void show(struct student_st st)
{
	printf("name: %s, no: %d\n", st.name, st.no);
}

int main(int argc, char *argv[])
{
	printf("student struct size = %d\n", sizeof(struct student_st));
	printf("func struct size = %d\n", sizeof(struct func_st));

	struct student_st st = {"wang", 1, show};
	
	st.show(st);

	return 0;
}
