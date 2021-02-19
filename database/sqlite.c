#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>

#define SQLITE_BUSY_TIMEOUT		10
#define SQLITE_QUERY_MAX_NUM	10
#define SENSOR_DB_PATH			"/usr/local/db/sensor.db"

static int busy_handler(void *arg, int repeat)
{
	if(repeat < (int)arg){
		printf("sqlite busy-handler %d times\n", repeat);
		sleep(1);
		return 1;
	}
	return 0;
}

int create_database(sqlite3 **db, const char *path, int (*create_table)(sqlite3 *))
{
	int ret = 0;
	
	ret = sqlite3_open(path, db);
	if(ret != SQLITE_OK){
		printf("open db %s error:%s\n", path, sqlite3_errmsg(*db));
		return -1;
	}

	ret = sqlite3_exec(*db, "PRAGMA cache_size = 2000", NULL, NULL, NULL);
	if(ret != SQLITE_OK){
		printf("PRAGMA cache_size = 2000 error\n");
//		return -1;
	}

	ret = sqlite3_exec(*db, "PRAGMA auto_vacuum = 1", NULL, NULL, NULL);
	if(ret != SQLITE_OK){
		printf("PRAGMA auto_vacuum = 1 error\n");
//		return -1;
	}

	ret = sqlite3_exec(*db, "PRAGMA synchronous = FULL", NULL, NULL, NULL);
	if(ret != SQLITE_OK){
		printf("PRAGMA synchronous = FULL error\n");
//		return -1;
	}

	ret = sqlite3_exec(*db, "PRAGMA temp_store = MEMORY", NULL, NULL, NULL);
	if(ret != SQLITE_OK){
		printf("PRAGMA temp_store = MEMORY error\n");
//		return -1;
	}

	sqlite3_busy_handler(*db, busy_handler, (void *)SQLITE_BUSY_TIMEOUT);

	if(create_table){
		(create_table)(*db);
		/* 
		 * no need to check return, because database need open more times(such as reboot).
		 * no nedd to resolve the error that table alread exist. 
		ret = (create_table)(*db);
		if(ret < 0){
			sqlite3_close(*db);
			return -1;
		}
		*/
	}

	return 0;
}

int create_backup_table(sqlite3 *db)
{
	//int ret = 0;
	//char *err = NULL;

	sqlite3_exec(db, "CREATE TABLE backup_table (id integer primary key, data text, flag integer);", NULL, NULL, NULL);
	//sqlite3_exec(db, "CREATE UNIQUE INDEX idx_bak on backup_table(data);", NULL, NULL, NULL);

	/*
	 * no need to check return, because database need open more times(such as reboot).
	 * no nedd to resolve the error that table alread exist. 
	ret = sqlite3_exec(db, "CREATE TABLE backup_table (id integer primary key, data text, flag integer);", NULL, NULL, &err);
	if(ret != SQLITE_OK){
		printf("create backup_table error(%d): %s", ret, err);
		return -1;
	}
	*/

	return 0;
}

int modify_sql(sqlite3 *db, const char *sql)
{
	int ret = 0;
	char *err = NULL;

	ret = sqlite3_exec(db, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, &err);
	if(ret != SQLITE_OK){
		printf("BEGIN TRANSACTION error: %s", err);
		sqlite3_free(err);
		return -1;
	}

	ret = sqlite3_exec(db, sql, NULL, NULL, &err);
	if(ret != SQLITE_OK){
		printf("exec sql(%s) error: %s", sql, err);
		sqlite3_free(err);
		return -1;
	}

	ret = sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &err);
	if(ret != SQLITE_OK){
		printf("COMMIT TRANSACTION error: %s", err);
		sqlite3_free(err);
		return -1;
	}

	return 0;
}

int result_callback(void *in, int argc, char *argv[], char *argvv[])
{
	int i = 0;
	int *arr = (int *)in;

	for(;i < argc; i++){
		printf("%s=%s ", argvv[i], argv[i]?argv[i]:"NULL");
		if(arr && (!strcmp(argvv[i], "id"))){
			arr[arr[SQLITE_QUERY_MAX_NUM]] = atoi(argv[i]);
			arr[SQLITE_QUERY_MAX_NUM]++;
		}
	}
	printf("\n");
	
	return 0;
}

int select_sql(sqlite3 *db, const char *sql, sqlite3_callback callback, int arr[])
{
	int ret = 0;
	char *err = NULL;

	ret = sqlite3_exec(db, sql, callback, arr, &err);
	if(ret != SQLITE_OK){
		printf("exec select sql(%s) error: %s", sql, err);
		sqlite3_free(err);
		return -1;
	}

	return 0;
}

char *itoa(int num, char *str, int radix)
{
	char index[]="0123456789ABCDEF";
	unsigned unum;
	int i=0, j, k;
	char tmp;

	if(radix == 10 && num < 0){
		unum = (unsigned)-num;
		str[i++]='-';
	} else {
		unum = (unsigned)num;
	}

	do{
		str[i++]=index[unum%(unsigned)radix];
		unum /= radix;
	} while(unum);
	str[i] = '\0';

	if(str[0] == '-'){
		k = 1;
	} else {
		k = 0;
	}

	for(j = k; j <= (i-1)/2; j++){
		tmp = str[j];
		str[j] = str[i-1+k-j];
		str[i-1+k-j] = tmp;
	}

	return str;
}

#if 1
int main(int argc, char *argv[])
{
	int i = SQLITE_QUERY_MAX_NUM + 1;//10; 
	int ret = 0;
	sqlite3 *db;
	char sql[512], cbuf[16];
	int id_arr[SQLITE_QUERY_MAX_NUM+1] = {0};

	ret = create_database(&db, SENSOR_DB_PATH, create_backup_table);
	if(ret){
		return -1;
	}

	// insert
	while(i-- > 0){
		memset(sql, 0, 512);
		sprintf(sql, "insert into backup_table(data, flag) values('%ld', %d);", 
				time(NULL), i);
		if(modify_sql(db, sql)){
			break;
		}
	}

	// select
	memset(id_arr, 0, sizeof(id_arr));
	memset(sql, 0, 512);
	sprintf(sql, "select * from backup_table limit %d;", SQLITE_QUERY_MAX_NUM);
	select_sql(db, sql, result_callback, id_arr);
	if(id_arr[SQLITE_QUERY_MAX_NUM]){
		// delete
		memset(sql, 0, 512);
		sprintf(sql, "delete from backup_table where id in (0");
		for(i = 0; i < id_arr[SQLITE_QUERY_MAX_NUM]; i++){
			strcat(sql, ",");
			strcat(sql, itoa(id_arr[i], cbuf, 10));
		}
		strcat(sql, ");");
		modify_sql(db, sql);
	}

	// select
	printf("Current data----------\n");
	memset(id_arr, 0, sizeof(id_arr));
	sprintf(sql, "select * from backup_table;");
	select_sql(db, sql, result_callback, id_arr);

	sqlite3_close(db);

	return 0;
}
#endif

