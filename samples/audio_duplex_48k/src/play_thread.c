/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(play_thread);

#include "AudioSystem.h"
#include "AudioTrack.h"

#include "play_thread.h"
#include "record_thread.h"
#include "duplex_config.h"

extern AudioTrack g_track;

static struct k_sem s_sem_run;
static volatile bool s_busy;

/* 最近一次录音的副本, K1 重播时使用 */
static const void *s_last_buffer;
static size_t s_last_bytes;
static uint32_t s_last_channels;

extern void play_thread(void *p1, void *p2, void *p3)
{
	int ret;

	ret = k_sem_init(&s_sem_run, 0, 1);
	assert(ret == 0);

	while (1) {
		LOG_INF("K1=REPLAY, K2=STOP, K3=MUTE, K4=UNMUTE");
		k_sem_take(&s_sem_run, K_FOREVER);

		if (s_last_buffer == NULL || s_last_bytes == 0) {
			LOG_INF("Nothing to replay, press SW2 to record first");
			continue;
		}

		s_busy = true;
		play_thread_inspect(s_last_buffer, s_last_bytes, s_last_channels);
		s_busy = false;
	}
}

#define PLAY_THREAD_STACK_SIZE (8 * 1024)
#define PLAY_THREAD_PRIORITY   (5)

K_THREAD_DEFINE(play_thread_tid, PLAY_THREAD_STACK_SIZE, play_thread, NULL, NULL, NULL,
		PLAY_THREAD_PRIORITY, 0, 0);

void play_thread_start(void)
{
	if (!s_busy) {
		LOG_INF("Replay");
		k_sem_give(&s_sem_run);
	}
}

void play_thread_stop(void)
{
	LOG_INF("Stop");
	AudioTrack_stop(&g_track);
}

void play_thread_mute(void)
{
	LOG_INF("Mute");
	AudioTrack_mute(&g_track, true);
}

void play_thread_unmute(void)
{
	LOG_INF("Unmute");
	AudioTrack_mute(&g_track, false);
}

#define SAMPLE_PER_MS (SAMPLE_RATE / 1000)
#define INSPECT_MS    (5000)

/* 单声道抽取缓冲: 一路通道 5 秒 */
static int16_t inspect_buf[SAMPLE_PER_MS * INSPECT_MS]
	__attribute__((section(".psram_section")));

void play_thread_inspect(const void *buffer, size_t bytes, uint32_t channels)
{
	size_t frames = bytes / sizeof(int16_t) / channels;
	if (frames > SAMPLE_PER_MS * INSPECT_MS) {
		frames = SAMPLE_PER_MS * INSPECT_MS;
	}

	LOG_INF("Inspecting %d frames, %d channels", frames, channels);

	s_last_buffer = buffer;
	s_last_bytes = bytes;
	s_last_channels = channels;

	s_busy = true;

	for (uint32_t i = 0; i < channels; i++) {
		k_msleep(500);
		LOG_INF("Inspect channel %d", i);

		for (size_t j = 0; j < frames; j++) {
			inspect_buf[j] = ((int16_t *)buffer)[j * channels + i];
		}

		LOG_HEXDUMP_INF(inspect_buf, MIN(64, frames * sizeof(int16_t)), "Inspect data");

		AudioTrack_start(&g_track);

		AudioTrack_write(&g_track, inspect_buf, frames * sizeof(int16_t));

		AudioTrack_stop(&g_track);
	}

	LOG_INF("Inspect done");

	s_busy = false;
}
