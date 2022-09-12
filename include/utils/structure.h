#ifndef _STR_H_
#define _STR_H_

#include "common.h"

char* string_init(const char *str);
char* string_join(const char *base, const char *add);

uint32_t** uint32_list_init();
char* uint32_list_dump(uint32_t **int_list);
void uint32_list_free(uint32_t **uint32_list);
uint32_t uint32_list_len(uint32_t **int_list);
uint32_t** uint32_list_append(uint32_t **int_list, uint32_t number);

char** string_list_init();
void string_list_free(char **string_list);
char* string_list_dump(char **string_list);
uint32_t string_list_len(char **string_list);
char** string_list_append(char **string_list, const char *string);

#endif
