/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include "AudioSystem.h"
#include "AudioTrack.h"
#include "AudioRecord.h"

#define SAMPLE_RATE (16000)
#define CHANNEL_CNT (1)

#define RECORD_SECS       (5)
#define RECORD_SAMPLE_CNT (SAMPLE_RATE * RECORD_SECS)

static const struct gpio_dt_spec btn = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static struct gpio_callback btn_cb_data;

static AudioRecord s_record_obj;
static AudioTrack s_track_obj;

static struct k_sem s_sem_record;

__attribute__((section(".psram_section"))) static uint8_t
	recording[sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT];

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
	String8 param;
	String8_ctor_char(&param, "ADC_PDM_GAIN_A_LEFT=6;"
				  "ADC_PDM_GAIN_D_LEFT=20;"
				  "ADC_PDM_GAIN_A_RIGHT=6;"
				  "ADC_PDM_GAIN_D_RIGHT=20");
	ret = AudioSystem_setParameters(0, &param);
	String8_dtor(&param);
	assert(0 == ret);

	AudioRecord *record = &s_record_obj;
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);
	assert(0 == ret);

	AudioTrack *track = &s_track_obj;
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, 0);
	assert(0 == ret);

	ret = gpio_pin_interrupt_configure_dt(&btn, GPIO_INT_EDGE_TO_ACTIVE);
	assert(0 == ret);

	gpio_init_callback(&btn_cb_data, button_pressed, BIT(btn.pin));
	gpio_add_callback(btn.port, &btn_cb_data);

	printk("Press the button\n");

	ssize_t size;
	while (1) {
		/* 循环等待按键按下 */
		k_sem_take(&s_sem_record, K_FOREVER);

		gpio_pin_set_dt(&led, 1);

		printk("Records 5 seconds...\n");

		AudioRecord_start(record);
		size = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
		AudioRecord_stop(record);
		assert(size > 0);

		gpio_pin_set_dt(&led, 0);

		printk("Plays 5 seconds...\n");

		AudioTrack_start(track);
		size = AudioTrack_write(track, recording, size);
		assert(size > 0);
		AudioTrack_stop(track);
	}
}
