#pragma once

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

void record_thread_start(void);

/* 最近一次录音的缓冲与元数据, 供播音线程回放 */
extern int16_t g_record_buf[];
extern ssize_t g_record_bytes;
extern uint32_t g_record_channels;
