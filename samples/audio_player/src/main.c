/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/drivers/gpio.h>

#include "AudioSystem.h"
#include "AudioTrack.h"

#define SAMPLE_RATE (16000)
#define CHANNEL_CNT (1)

#define FLASH_ADDR      (DT_REG_ADDR(DT_CHOSEN(zephyr_flash)))
#define TEST_AUDIO_ADDR (FLASH_ADDR + FLASH_AREA_OFFSET(test_audio))
#define TEST_AUDIO_SIZE (FLASH_AREA_SIZE(test_audio))

static const struct gpio_dt_spec btn = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static struct gpio_callback btn_cb_data;

static AudioTrack s_track_obj;

static struct k_sem s_sem_record;

static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());

	k_sem_give(&s_sem_record);
}

void main(void)
{
	int ret;

	printk("Hello World! %s\n", CONFIG_BOARD);

	k_sem_init(&s_sem_record, 0, 1);

	/* 初始化 GPIO */
	ret = gpio_pin_configure_dt(&btn, GPIO_INPUT);
	assert(0 == ret);

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	assert(0 == ret);

	/* 初始化音频组件 */
	AudioTrack *track = &s_track_obj;

	ret = gpio_pin_interrupt_configure_dt(&btn, GPIO_INT_EDGE_TO_ACTIVE);
	assert(0 == ret);

	gpio_init_callback(&btn_cb_data, button_pressed, BIT(btn.pin));
	gpio_add_callback(btn.port, &btn_cb_data);

	printk("Press the button\n");

	while (1) {
		/* 循环等待按键按下 */
		k_sem_take(&s_sem_record, K_FOREVER);

		gpio_pin_set_dt(&led, 1);

		ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, 0);
		assert(0 == ret);

		AudioTrack_start(track);

		ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);
		assert(ret == 0);

		AudioTrack_stop(track);
		AudioTrack_dtor(track);

		gpio_pin_set_dt(&led, 0);
	}
}
