#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "cJSON.h"
#include "s2j.h"

typedef struct mqtt_info{
	char Url[32];
	int Port;
	bool IsLive;
} mqtt_info_t;


typedef struct mkey{
	char Product[20];
	char ProductSn[20];
	char ProductKey[20];
	char DeviceName[20];
	char DeviceSecret[64];
	mqtt_info_t MQTTInfo;
	char Addr[10][16];
} mkey_t;

char *marshal_mkey(mkey_t *m)
{
	if(m == NULL){
		return NULL;
	}

	char *string = NULL;
	cJSON *jkey = NULL;
	cJSON *jmqtt = NULL;
	cJSON *jaddr = NULL;
	cJSON *jaddr_item = NULL;
	int index = 0;

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
	jmqtt = cJSON_CreateObject();
	if(jmqtt == NULL){
		goto end;
	}
	if(!cJSON_AddItemToObject(jkey, "MqttInfo", jmqtt)){
		goto end1;
	}
	if(cJSON_AddStringToObject(jmqtt, "Url", m->MQTTInfo.Url) == NULL){
		goto end1;
	}
	if(cJSON_AddIntToObject(jmqtt, "Port", m->MQTTInfo.Port) == NULL){
		goto end1;
	}
	if(cJSON_AddBoolToObject(jmqtt, "IsLive", m->MQTTInfo.IsLive) == NULL){
		goto end1;
	}

	if((jaddr = cJSON_AddArrayToObject(jkey, "Address")) == NULL){
		goto end1;
	}
	for(index=0; index < (sizeof(m->Addr)/sizeof(m->Addr[0])); index++){
		if(m->Addr[index][0] == 0){
			break;
		}
		jaddr_item = cJSON_CreateString(m->Addr[index]);
		if(jaddr_item == NULL){
			goto end1;
		}
		if(!cJSON_AddItemToArray(jaddr, jaddr_item)){
			goto end1;
		}
	}

	string = cJSON_PrintUnformatted(jkey);
	if(string == NULL){
		goto end1;
	}

end1:
	//cJSON_Delete(jmqtt);

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
	cJSON *jmqtt = NULL;
	cJSON *intkey = NULL;
	cJSON *boolkey = NULL;
	cJSON *jaddr = NULL;
	cJSON *jaddr_item = NULL;

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

	jmqtt = cJSON_GetObjectItemCaseSensitive(jkey, "MqttInfo");
	if(cJSON_IsObject(jmqtt)){
		strkey = cJSON_GetObjectItemCaseSensitive(jmqtt, "Url");
		if(cJSON_IsString(strkey) && (strkey->valuestring != NULL)){
			strncpy(m->MQTTInfo.Url, strkey->valuestring, sizeof(m->MQTTInfo.Url));
			status++;
		}

		intkey = cJSON_GetObjectItemCaseSensitive(jmqtt, "Port");
		if(cJSON_IsInt(intkey)){
			m->MQTTInfo.Port = intkey->valueint;
			status++;
		}

		boolkey = cJSON_GetObjectItemCaseSensitive(jmqtt, "IsLive"); 
		if(cJSON_IsBool(boolkey)){
			m->MQTTInfo.IsLive = cJSON_IsTrue(boolkey)?true:false;
			status++;
		}
	}

	jaddr = cJSON_GetObjectItemCaseSensitive(jkey, "Address");
	if(cJSON_IsArray(jaddr)){
		int num = cJSON_GetArraySize(jaddr);
		int i = 0;
		for(i=0; i < num; i++){
			jaddr_item = cJSON_GetArrayItem(jaddr, i);
			if(cJSON_IsString(strkey) && (strkey->valuestring != NULL)){
				strncpy(m->Addr[i], jaddr_item->valuestring, sizeof(m->Addr[i])); 
			}
		}
	}

	if(status != 8){
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
		.DeviceSecret = "dcda624f0e8be108931df60910c68354",
		.MQTTInfo = {
			.Url = "www.iotcloud.com",
			.Port = 1881,
			.IsLive = false,
		},
		.Addr = {
			"192.168.3.111",
			"192.168.3.222",
		},
	};
	mkey_t mid2;
	char *json_str = NULL;
	int ret = 0;

	memset(&mid2, 0, sizeof(mkey_t));

	json_str = marshal_mkey(&mid);
	if(json_str != NULL){
		printf("%s\n", json_str);
		/* {"Product":"BoxBox","ProductSn":"WB0123456789","ProductKey":"aap0d4WfhqJ",
			"DeviceName":"abcdefgh","DeviceSecret":"dcda624f0e8be108931df60910c68354",
			"MqttInfo":{"Url":"www.iotcloud.com","Port":1881,"IsLive":false},
			"Address":["192.168.3.111","192.168.3.222"]}
		*/
		ret = unmarshal_mkey(json_str, &mid2);
		if(ret){
			printf("parse successfully\n");
			printf("%s-%s-%s\n%s-%s-%s-%s\n", mid2.Product, mid2.ProductSn, mid2.ProductKey, 
				mid2.DeviceName, mid2.DeviceSecret, mid2.Addr[0], mid2.Addr[1]);
		} else {
			printf("parse error\n");
		}
		cJSON_free(json_str);
	}

	return 0;
}

