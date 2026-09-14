#pragma once

#include <stdint.h>
#include <stddef.h>

void play_thread_start(void);
void play_thread_stop(void);
void play_thread_mute(void);
void play_thread_unmute(void);

/* 依次播音每个通道: 把 buffer 中的第 i 路抽出来单声道播放 */
void play_thread_inspect(const void *buffer, size_t bytes, uint32_t channels);
