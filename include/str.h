/**
 * @file str.h
 * @brief Operations for strings.
 */

#pragma once

#include "types.h"
#include "flexlion/ProcessMemory.hpp"

#ifdef __cplusplus
extern "C" {
#endif

int strcmp_0(char const *str1, char const *str2);
u64 strlen_0(char const *str);
u128 strtoull_0(char const *str, char** strEnd, s32 base);
using strtodptr = double (*)(const char *nptr, char **endptr);
double strtod_impl(const char *nptr, char **endptr);

char* strcpy(char* dest, char const* src);

#ifdef __cplusplus
}
#endif