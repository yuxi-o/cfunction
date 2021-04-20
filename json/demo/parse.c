#include <stdio.h>
#include "cJSON.h"

char *create_monitor(void)
{
	const unsigned int resolution_numbers[3][2] = {
		{1280, 720},
		{1920, 1080},
		{3840, 2160}
	};

	char *string = NULL;
	cJSON *monitor = NULL;
	cJSON *name = NULL, *resolutions = NULL, *resolution = NULL;
	cJSON *width = NULL, *height = NULL;


	monitor = cJSON_CreateObject();
	if(monitor == NULL){
		return NULL;
	}
/*
	name = cJSON_CreateString("Awesome 4K");
	if(name == NULL){
		goto end;
	}
	cJSON_AddItemToObject(monitor, "name", name);
*/
	if(cJSON_AddStringToObject(monitor, "name", "Awesome 4K") == NULL){
		goto end;
	}
/*
	resolutions = cJSON_CreateArray();
	if(resolutions == NULL){
		goto end;
	}
	cJSON_AddItemToObject(monitor, "resolutions", resolutions);
*/
	resolutions = cJSON_AddArrayToObject(monitor, "resolutions");
	if(resolutions == NULL){
		goto end;
	}

	for(int index = 0; index < (sizeof(resolution_numbers)/sizeof(resolution_numbers[0])); index++){
		resolution = cJSON_CreateObject();
		if(resolution == NULL){
			goto end;
		}
		cJSON_AddItemToArray(resolutions, resolution);
/*
		width = cJSON_CreateNumber(resolution_numbers[index][0]);
		if(width == NULL){
			goto end;
		}
		cJSON_AddItemToObject(resolution, "width", width);

		height = cJSON_CreateNumber(resolution_numbers[index][1]);
		if(height == NULL){
			goto end;
		}
		cJSON_AddItemToObject(resolution, "height", height);
*/
		if(cJSON_AddNumberToObject(resolution, "width", resolution_numbers[index][0])== NULL){
			goto end;
		}
		if(cJSON_AddNumberToObject(resolution, "height", resolution_numbers[index][1]) == NULL){
			goto end;
		}
	}

//	string = cJSON_PrintUnformatted(monitor);
	string = cJSON_Print(monitor);
	if(string == NULL){
		printf("Failed to print monitor\n");
	}

end:
	cJSON_Delete(monitor);
	return string;
}

int supports_full_hd(const char * const monitor)
{
	const cJSON *resolution = NULL;
	const cJSON *resolutions = NULL;
	cJSON *monitor_json = NULL;
	const cJSON *name = NULL;
	int status = 0;

	monitor_json = cJSON_Parse(monitor);
	if(monitor_json == NULL){
		const char *error_ptr = cJSON_GetErrorPtr();
		if(error_ptr != NULL){
			printf("Error before: %s\n", error_ptr);
		}
		status = 0;
		goto end;
	}

	name = cJSON_GetObjectItemCaseSensitive(monitor_json, "name");
	if(cJSON_IsString(name) && (name->valuestring != NULL)){
		printf("checking monitor \"%s\"\n", name->valuestring);
	}

	resolutions = cJSON_GetObjectItemCaseSensitive(monitor_json, "resolutions");
	cJSON_ArrayForEach(resolution, resolutions){
		cJSON *width  = cJSON_GetObjectItemCaseSensitive(resolution, "width");
		cJSON *height = cJSON_GetObjectItemCaseSensitive(resolution, "height");
		if(!cJSON_IsNumber(width) || !cJSON_IsNumber(height)){
			status = 0;
			goto end;
		}

		if((width->valueint == 1920 && height->valueint == 1080)){
			status = 1;
			goto end;
		}
	}

end:
	cJSON_Delete(monitor_json);
	return status;
}


int main(int argc, char *argv[])
{
	char *show = NULL;

	if((show = create_monitor()) != NULL){
		printf("create JSON:\n%s\n", show);

		if(supports_full_hd(show)){
			printf("device support hd\n");
		}
		cJSON_free(show);
	}

	return 0;
}

