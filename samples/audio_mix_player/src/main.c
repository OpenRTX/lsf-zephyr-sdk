/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/kscan.h>
#include <zephyr/sys/util.h>

#include "AudioSystem.h"
#include "AudioTrack.h"

#define SAMPLE_RATE (16000)
#define CHANNEL_CNT (1)

#define FLASH_ADDR (DT_REG_ADDR(DT_CHOSEN(zephyr_flash)))

#define TEST_BG_AUDIO_ADDR (FLASH_ADDR + FLASH_AREA_OFFSET(test_bg_audio))
#define TEST_BG_AUDIO_SIZE (FLASH_AREA_SIZE(test_bg_audio))

#define TEST_FG_AUDIO_ADDR (FLASH_ADDR + FLASH_AREA_OFFSET(test_fg_audio))
#define TEST_FG_AUDIO_SIZE (FLASH_AREA_SIZE(test_fg_audio))

enum csk_adc_key_t {
	KEY_K4,
	KEY_K3,
	KEY_K2,
	KEY_K1,
	KEY_SW2,
};

static const struct device *kscan = DEVICE_DT_GET(DT_ALIAS(analogkey));

static AudioTrack track_bg;
static AudioTrack track_fg;

K_SEM_DEFINE(sem_start_bg, 0, 1);
K_SEM_DEFINE(sem_start_fg, 0, 1);

static float volume = 1.0f;

static void kscan_callback(const struct device *dev, uint32_t row, uint32_t column, bool pressed)
{
	if (!pressed) {
		return;
	}

	printk("Key %d pressed at %" PRIu32 "\n", row, k_cycle_get_32());

	switch (row) {
	case KEY_SW2:
		AudioTrack_stop(&track_bg);
		AudioTrack_stop(&track_fg);
		break;
	case KEY_K1:
		if (AudioTrack_stopped(&track_bg)) {
			k_sem_give(&sem_start_bg);
		} else {
			AudioTrack_stop(&track_bg);
		}
		break;
	case KEY_K2: {
		if (AudioTrack_stopped(&track_fg)) {
			k_sem_give(&sem_start_fg);
		} else {
			AudioTrack_stop(&track_fg);
		}
	} break;
	case KEY_K3: {
		volume = MAX(0.0f, volume - 0.1f);
		AudioTrack_setVolume(&track_bg, volume, volume);
		AudioTrack_setVolume(&track_fg, volume, volume);
		printk("Volume DN: %.2f\n", volume);
	} break;
	case KEY_K4: {
		volume = MAX(0.0f, volume + 0.1f);
		AudioTrack_setVolume(&track_bg, volume, volume);
		AudioTrack_setVolume(&track_fg, volume, volume);
		printk("Volume UP: %.2f\n", volume);
	} break;
	}
}

void main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);

	/* 初始化 GPIO */
	kscan_config(kscan, kscan_callback);

	printk("K1: PLAY/STOP BG, K2: PLAY/STOP FG, K3: VOL-, K4: VOL+\n");

	while (1) {
		k_msleep(1000);
	}
}

static void bg_task(void *p1, void *p2, void *p3)
{
	int ret;

	ret = AudioTrack_ctor(&track_bg, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 0, NULL);
	if (ret != 0) {
		printk("AudioTrack_ctor: %d\n", ret);
		return;
	}

	while (1) {
		k_sem_take(&sem_start_bg, K_FOREVER);

		printk("Start BG\n");
		AudioTrack_start(&track_bg);

		size_t wrote = 0;
		for (size_t i = 0; i < TEST_BG_AUDIO_SIZE && !AudioTrack_stopped(&track_bg);
		     i += KB(4)) {
			wrote += AudioTrack_write(&track_bg, (const void *)(TEST_BG_AUDIO_ADDR + i),
						  KB(4));
			k_yield();
		}

		AudioTrack_stop(&track_bg);
		printk("BG finished, actual wrote %d of %d\n", wrote, TEST_BG_AUDIO_SIZE);
	}
}

K_THREAD_DEFINE(bg_task_id, KB(4), bg_task, NULL, NULL, NULL, 5, 0, 0);

static void fg_task(void *p1, void *p2, void *p3)
{
	int ret;

	ret = AudioTrack_ctor(&track_fg, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 0, NULL);
	if (ret != 0) {
		printk("AudioTrack_ctor: %d\n", ret);
		return;
	}

	while (1) {
		k_sem_take(&sem_start_fg, K_FOREVER);

		printk("Start FG\n");
		AudioTrack_start(&track_fg);

		size_t wrote = 0;
		for (size_t i = 0; i < TEST_FG_AUDIO_SIZE && !AudioTrack_stopped(&track_fg);
		     i += KB(4)) {
			wrote += AudioTrack_write(&track_fg, (const void *)(TEST_FG_AUDIO_ADDR + i),
						  KB(4));
			k_yield();
		}

		AudioTrack_stop(&track_fg);
		printk("FG finished, actual wrote %d of %d\n", wrote, TEST_FG_AUDIO_SIZE);
	}
}

K_THREAD_DEFINE(fg_task_id, KB(4), fg_task, NULL, NULL, NULL, 5, 0, 0);
