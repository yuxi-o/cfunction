#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cJSON.h"

typedef struct mkey{
	char Product[20];
	char ProductSn[20];
	char ProductKey[20];
	char DeviceName[20];
	char DeviceSecret[64];
} mkey_t;

char *marshal_mkey(mkey_t *m)
{
	if(m == NULL){
		return NULL;
	}

	char *string = NULL;
	cJSON *jkey = NULL;

	jkey = cJSON_CreateObject();
	if(jkey == NULL){
		return NULL;
	}

	if(cJSON_AddStringToObject(jkey, "Product", m->Product) == NULL){
		goto end;
	}
	if(cJSON_AddStringToObject(jkey, "ProductSn", m->ProductSn) == NULL){
		goto end;
	}
	if(cJSON_AddStringToObject(jkey, "ProductKey", m->ProductKey) == NULL){
		goto end;
	}
	if(cJSON_AddStringToObject(jkey, "DeviceName", m->DeviceName) == NULL){
		goto end;
	}
	if(cJSON_AddStringToObject(jkey, "DeviceSecret", m->DeviceSecret) == NULL){
		goto end;
	}

	string = cJSON_PrintUnformatted(jkey);
	if(string == NULL){
		goto end;
	}

end:
	cJSON_Delete(jkey);
	return string;
}

int unmarshal_mkey(const char *mstr, mkey_t *m)
{
	if(!m){
		return 0;
	}

	int status = 0;
	cJSON *jkey = NULL;
	cJSON *strkey = NULL;

	jkey = cJSON_Parse(mstr);
	if(jkey == NULL){
		const char *error_ptr = cJSON_GetErrorPtr();
		if(error_ptr != NULL){
			fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		status = 0;
		goto end;
	}

	strkey = cJSON_GetObjectItemCaseSensitive(jkey, "Product");
	if(cJSON_IsString(strkey) && (strkey->valuestring != NULL)){
		strncpy(m->Product, strkey->valuestring, sizeof(m->Product));
		status++;
	}

	strkey = cJSON_GetObjectItemCaseSensitive(jkey, "ProductSn");
	if(cJSON_IsString(strkey) && (strkey->valuestring != NULL)){
		strncpy(m->ProductSn, strkey->valuestring, sizeof(m->ProductSn));
		status++;
	}

	strkey = cJSON_GetObjectItemCaseSensitive(jkey, "ProductKey");
	if(cJSON_IsString(strkey) && (strkey->valuestring != NULL)){
		strncpy(m->ProductKey, strkey->valuestring, sizeof(m->ProductKey));
		status++;
	}

	strkey = cJSON_GetObjectItemCaseSensitive(jkey, "DeviceName");
	if(cJSON_IsString(strkey) && (strkey->valuestring != NULL)){
		strncpy(m->DeviceName, strkey->valuestring, sizeof(m->DeviceName));
		status++;
	}

	strkey = cJSON_GetObjectItemCaseSensitive(jkey, "DeviceSecret");
	if(cJSON_IsString(strkey) && (strkey->valuestring != NULL)){
		strncpy(m->DeviceSecret, strkey->valuestring, sizeof(m->DeviceSecret));
		status++;
	}

	if(status != 5){
		status = 0; // parse error
		memset(m, 0, sizeof(*m));
	}

end:
	cJSON_Delete(jkey);
	return status;
}

int main(int argc, char *argv[])
{

	mkey_t mid = {
		.Product = "BoxBox",
		.ProductSn = "WB0123456789",
		.ProductKey = "aap0d4WfhqJ",
		.DeviceName = "abcdefgh",
		.DeviceSecret = "dcda624f0e8be108931df60910c68354"
	};
	mkey_t mid2;
	char *json_str = NULL;
	int ret = 0;

	json_str = marshal_mkey(&mid);
	if(json_str != NULL){
		printf("%s\n", json_str);
		ret = unmarshal_mkey(json_str, &mid2);
		if(ret){
			printf("parse successfully\n");
			printf("%s-%s-%s\n%s-%s\n", mid2.Product, mid2.ProductSn, mid2.ProductKey, 
				mid2.DeviceName, mid2.DeviceSecret);
		} else {
			printf("parse error\n");
		}
		cJSON_free(json_str);
	}

	return 0;
}

