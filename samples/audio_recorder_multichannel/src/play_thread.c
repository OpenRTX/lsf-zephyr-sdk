/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/storage/flash_map.h>

#include "AudioSystem.h"
#include "AudioTrack.h"

#define FLASH_ADDR      (DT_REG_ADDR(DT_CHOSEN(zephyr_flash)))
#define TEST_AUDIO_ADDR (FLASH_ADDR + FLASH_AREA_OFFSET(test_audio))
#define TEST_AUDIO_SIZE (FLASH_AREA_SIZE(test_audio))

extern AudioTrack g_track;

static struct k_sem s_sem_run;
static volatile bool s_busy;

#define CHANNEL_CNT (1)

extern void play_thread(void *p1, void *p2, void *p3)
{
	int ret;
	ssize_t wrote;

	ret = k_sem_init(&s_sem_run, 0, 1);
	assert(ret == 0);

	while (1) {
		printk("K1=PLAY, K2=STOP, K3=MUTE, K4=UNMUTE\n");
		k_sem_take(&s_sem_run, K_FOREVER);
		s_busy = true;

		AudioTrack_start(&g_track);

		wrote = AudioTrack_write(&g_track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);
		printk("Actual wrote %d of %d bytes\n", wrote, TEST_AUDIO_SIZE);

		AudioTrack_stop(&g_track);

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
		k_sem_give(&s_sem_run);
	}
}

void play_thread_stop(void)
{
	AudioTrack_stop(&g_track);
}

void play_thread_mute(void)
{
	AudioTrack_mute(&g_track, true);
}

void play_thread_unmute(void)
{
	AudioTrack_mute(&g_track, false);
}

#define SAMPLE_PER_MS (16)
#define INSPECT_MS    (5000)

__attribute__((section(".psram_section"))) static int16_t inspect_buf[SAMPLE_PER_MS * INSPECT_MS];

void play_thread_inspect(const void *buffer, size_t bytes, uint32_t channels)
{
	size_t frames = bytes / sizeof(int16_t) / channels;
	if (frames > SAMPLE_PER_MS * INSPECT_MS) {
		frames = SAMPLE_PER_MS * INSPECT_MS;
	}

	s_busy = true;

	for (int i = 0; i < channels; i++) {
		k_msleep(500);
		printk("Inspect channel %d\n", i);

		for (int j = 0; j < frames; j++) {
			inspect_buf[j] = ((int16_t *)buffer)[j * channels + i];
		}

		AudioTrack_start(&g_track);

		AudioTrack_write(&g_track, inspect_buf, frames * sizeof(int16_t));

		AudioTrack_stop(&g_track);
	}

	printk("Inspect done\n");

	s_busy = false;
}
