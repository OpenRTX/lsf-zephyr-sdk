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

#include "AudioSystem.h"
#include "AudioTrack.h"

#define SAMPLE_RATE (16000)
#define CHANNEL_CNT (1)

#define FLASH_ADDR      (DT_REG_ADDR(DT_CHOSEN(zephyr_flash)))
#define TEST_AUDIO_ADDR (FLASH_ADDR + FLASH_AREA_OFFSET(test_audio))
#define TEST_AUDIO_SIZE (FLASH_AREA_SIZE(test_audio))

enum csk_adc_key_t {
	KEY_K4,
	KEY_K3,
	KEY_K2,
	KEY_K1,
	KEY_SW2,
};

static const struct device *kscan = DEVICE_DT_GET(DT_ALIAS(analogkey));
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static AudioTrack s_track_obj;

static struct k_sem s_sem_record;

static void kscan_callback(const struct device *dev, uint32_t row, uint32_t column, bool pressed)
{
	if (!pressed) {
		return;
	}

	printk("Key %d pressed at %" PRIu32 "\n", row, k_cycle_get_32());

	if (row == KEY_SW2) {
		k_sem_give(&s_sem_record);
	}
}

void main(void)
{
	int ret;

	printk("Hello World! %s\n", CONFIG_BOARD);

	k_sem_init(&s_sem_record, 0, 1);

	/* 初始化 GPIO */
	ret = kscan_config(kscan, kscan_callback);
	assert(0 == ret);

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	assert(0 == ret);

	/* 初始化音频组件 */
	AudioTrack *track = &s_track_obj;
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, 0);
	assert(0 == ret);

	printk("Press the button\n");

	while (1) {
		/* 循环等待按键按下 */
		k_sem_take(&s_sem_record, K_FOREVER);

		gpio_pin_set_dt(&led, 1);

		AudioTrack_start(track);

		ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);
		assert(ret == 0);

		AudioTrack_stop(track);

		gpio_pin_set_dt(&led, 0);
	}
}
