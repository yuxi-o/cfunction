#include <stdio.h>
#include <stdint.h>
#include "s2j.h"
#include "s2jdef.h"

#ifdef DEBUGS2J
extern int s2j_test(void);
extern int s2j_test2(void);
#endif// DEBUGS2J

typedef struct {
    char name[16];
} Hometown;

typedef struct {
    uint8_t id;
    double weight;
    uint8_t score[8];
    char name[10];
    Hometown hometown;
    char homeaddr[10][16];
} Student;

/**
 * Student JSON object to structure object
 *
 * @param json_obj JSON object
 *
 * @return structure object
 */
static void *json_to_struct(cJSON* json_obj) {
    /* create Student structure object */
    s2j_create_struct_obj(struct_student, Student);

    /* deserialize data to Student structure object. */
    s2j_struct_get_basic_element(struct_student, json_obj, int, id);
#if 1
    s2j_struct_get_array_element(struct_student, json_obj, int, score);
    s2j_struct_get_basic_element(struct_student, json_obj, string, name);
    s2j_struct_get_basic_element(struct_student, json_obj, double, weight);
#else // another xxx_ex api, add default value and more secure
    s2j_struct_get_array_element_ex(struct_student, json_obj, int, score, 8, 0);
    s2j_struct_get_basic_element_ex(struct_student, json_obj, string, name, "John");
    s2j_struct_get_basic_element_ex(struct_student, json_obj, double, weight, 0);
#endif

    /* deserialize data to Student.Hometown structure object. */
    s2j_struct_get_struct_element(struct_hometown, struct_student, json_hometown, json_obj, Hometown, hometown);
    s2j_struct_get_basic_element(struct_hometown, json_hometown, string, name);
    s2j_struct_get_array_element(struct_student, json_obj, string, homeaddr);

    /* return Student structure object pointer */
    return struct_student;
}

/**
 * Student structure object to JSON object
 *
 * @param struct_obj structure object
 *
 * @param JSON object
 */
static cJSON *struct_to_json(void* struct_obj) {
    Student *struct_student = (Student *)struct_obj;

    /* create Student JSON object */
    s2j_create_json_obj(json_student);

    /* serialize data to Student JSON object. */
    s2j_json_set_basic_element(json_student, struct_student, int, id);
    s2j_json_set_basic_element(json_student, struct_student, double, weight);
    s2j_json_set_array_element(json_student, struct_student, int, score, 8);
    s2j_json_set_basic_element(json_student, struct_student, string, name);

    /* serialize data to Student.Hometown JSON object. */
    s2j_json_set_struct_element(json_hometown, json_student, struct_hometown, struct_student, Hometown, hometown);
    s2j_json_set_basic_element(json_hometown, struct_hometown, string, name);

    s2j_json_set_array_element(json_student, struct_student, string, homeaddr, sizeof(struct_student->homeaddr)/sizeof(struct_student->homeaddr[0]));

    /* return Student JSON object pointer */
    return json_student;
}

int main(void) {
    static Student orignal_student_obj = {
            .id = 24,
            .weight = 71.2,
            .score = {1, 2, 3, 4, 5, 6, 7, 8},
            .name = "armink",
            .hometown.name = "China",
            .homeaddr = {"192.168.3.111", "192.168.3.222"},
    };

    /* serialize Student structure object */
    cJSON *json_student = struct_to_json(&orignal_student_obj);
    /* deserialize Student structure object */
    Student *converted_student_obj = json_to_struct(json_student);

    /* compare Student structure object */
    if(memcmp(&orignal_student_obj, converted_student_obj, sizeof(Student))) {
        printf("Converted failed!\n");
    } else {
        printf("Converted OK!\n");
		// char *string = cJSON_Print(json_student);
        char *string = cJSON_PrintUnformatted(json_student);
		if(string){
			printf("%s\n", string);
			cJSON_free(string);
		}
    }

    s2j_delete_json_obj(json_student);
    s2j_delete_struct_obj(converted_student_obj);

#ifdef DEBUGS2J
    s2j_test();
    s2j_test2();
#endif// DEBUGS2J

    return 0;
}
