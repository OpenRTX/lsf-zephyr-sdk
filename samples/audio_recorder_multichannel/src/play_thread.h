#pragma once

#include <stdint.h>

void play_thread_start(void);
void play_thread_stop(void);
void play_thread_mute(void);
void play_thread_unmute(void);

void play_thread_inspect(const void *buffer, size_t bytes, uint32_t channels);
