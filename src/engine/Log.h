#pragma once
#include <android/log.h>
#define LOGE(...)   ((void)__android_log_print(ANDROID_LOG_ERROR, "ANTI", __VA_ARGS__))
#define LOG(...)    ((void)__android_log_print(ANDROID_LOG_INFO, "ANTI", __VA_ARGS__))