// gcc sqlite.c -Wall -lsqlite3
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>

#define SQLITE_BUSY_TIMEOUT		10
#define SQLITE_QUERY_MAX_NUM	10
#define SQLITE_QUERY_ITEM_LEN	12
#define SQLITE_SQL_MAX_LEN		512
//#define SENSOR_DB_PATH			"/usr/local/db/sensor.db"
#define SENSOR_DB_PATH			"/tmp/sensor.db"


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

	sqlite3_exec(db, "CREATE TABLE rttable(ID integer primary key, polld text, time integer, value real, extend text default '0');", NULL, NULL, NULL);
	sqlite3_exec(db, "CREATE UNIQUE INDEX idx_rtd on rttable(polld);", NULL, NULL, NULL);

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
	char *arr = (char *)in;
	int count = arr?*(arr + SQLITE_QUERY_MAX_NUM*SQLITE_QUERY_ITEM_LEN):0;

	for(;i < argc; i++){
		printf("%s=%s ", argvv[i], argv[i]?argv[i]:"NULL");
		if(arr && !strcmp(argvv[i], "id")){
			strcpy(arr + count*SQLITE_QUERY_ITEM_LEN, argv[i]);
			*(arr + SQLITE_QUERY_MAX_NUM*SQLITE_QUERY_ITEM_LEN) = count + 1;
		}
	}
	printf("\n");
	
	return 0;
}

int update_callback(void *in, int argc, char *argv[], char *argvv[])
{
	int i = 0, index_id = 0, index_data = 0;
	char sql[SQLITE_SQL_MAX_LEN] = {0};

	for(; i < argc; i++){
		if(!strcmp(argvv[i], "data")){
			index_data = i;
		} else if(!strcmp(argvv[i], "id")){
			index_id = i;
		}
	}

	sprintf(sql, "update backup_table set flag=0 where id=%s;", argv[index_id]);
	printf("update sql: %s\n", sql);
	if(modify_sql((sqlite3 *)in, sql)){
		return -1;
	}

	return 0;
}

int delete_callback(void *in, int argc, char *argv[], char *argvv[])
{
	int i = 0, index_id = 0, index_data = 0;
	char sql[SQLITE_SQL_MAX_LEN] = {0};

	for(; i < argc; i++){
		if(!strcmp(argvv[i], "data")){
			index_data = i;
		} else if(!strcmp(argvv[i], "id")){
			index_id = i;
		}
	}

	sprintf(sql, "delete from backup_table where id=%s;", argv[index_id]);
	printf("delete sql: %s\n", sql);
	if(modify_sql((sqlite3 *)in, sql)){
		return -1;
	}

	return 0;
}

int select_sql(sqlite3 *db, const char *sql, sqlite3_callback callback, void *arr)
{
	int ret = 0;
	char *err = NULL;

	ret = sqlite3_exec(db, sql, callback, (void *)arr, &err);
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
	int i = 5; //SQLITE_QUERY_MAX_NUM + 1;//10; 
	int ret = 0;
	sqlite3 *db;
	char sql[512], cbuf[16];
//	int id_arr[SQLITE_QUERY_MAX_NUM+1] = {0};
	char id_arr[SQLITE_QUERY_MAX_NUM+1][SQLITE_QUERY_ITEM_LEN] = {0};

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

	i = 5;
	while(i-- > 0){
		memset(sql, 0, 512);
		sprintf(sql, "replace into rttable(polld, time, value, extend) values('%s%d', %ld, %f, '%s');", 
				"water", i, time(NULL), 30.5, "NO");   // 替换原来数据，每个polld只保留一份数据
		if(modify_sql(db, sql)){
			break;
		}
		sleep(1);
	}

	// select
	memset(sql, 0, 512);
	id_arr[SQLITE_QUERY_MAX_NUM][0] = 0;
	sprintf(sql, "select * from backup_table limit %d;", SQLITE_QUERY_MAX_NUM);
//	select_sql(db, sql, delete_callback, db);
#if 0
	select_sql(db, sql, result_callback, (void *)id_arr);
	if(id_arr[SQLITE_QUERY_MAX_NUM][0]){
		// delete
		memset(sql, 0, 512);
		sprintf(sql, "delete from backup_table where id in (0");
		for(i = 0; i < id_arr[SQLITE_QUERY_MAX_NUM][0]; i++){
			strcat(sql, ",");
//			strcat(sql, itoa(id_arr[i], cbuf, 10));
			strcat(sql, id_arr[i]);
		}
		strcat(sql, ");");
		printf("delete sql: %s\n", sql);
		modify_sql(db, sql);
	}
#endif

	// select
	printf("Current data----------\n");
	sprintf(sql, "select * from backup_table;");
	select_sql(db, sql, result_callback, NULL);

	printf("Current data----------\n");
	sprintf(sql, "select * from rttable;");
	select_sql(db, sql, result_callback, NULL);

	sqlite3_close(db);

	return 0;
}
#endif

