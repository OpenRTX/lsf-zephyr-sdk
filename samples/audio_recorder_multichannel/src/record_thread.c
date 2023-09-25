/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include "AudioSystem.h"
#include "AudioRecord.h"

#include "play_thread.h"

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

extern AudioRecord g_record;

static struct k_sem s_sem_run;
static volatile bool s_busy;

#define CHANNEL_CNT   (2)
#define SAMPLE_PER_MS (16)
#define DURATION_MS   (5000)

__attribute__((section(
	".psram_section"))) static int16_t record_buf[SAMPLE_PER_MS * DURATION_MS * CHANNEL_CNT];

static void record_thread(void *p1, void *p2, void *p3)
{
	int ret;
	ssize_t read;

	ret = k_sem_init(&s_sem_run, 0, 1);
	assert(ret == 0);

	while (1) {
		printk("Press SW2 to record\n");
		k_sem_take(&s_sem_run, K_FOREVER);
		s_busy = true;

		gpio_pin_set_dt(&led, 1);
		printk("Records 5 seconds...\n");

		AudioRecord_start(&g_record);

		read = AudioRecord_read(&g_record, record_buf, sizeof(record_buf));

		AudioRecord_stop(&g_record);

		gpio_pin_set_dt(&led, 0);
		printk("Actual read %d of %d bytes\n", read, sizeof(record_buf));

		play_thread_inspect(record_buf, read, CHANNEL_CNT);

		s_busy = false;
	}
}

#define RECORD_THREAD_STACK_SIZE (8 * 1024)
#define RECORD_THREAD_PRIORITY   (5)

K_THREAD_DEFINE(record_thread_tid, RECORD_THREAD_STACK_SIZE, record_thread, NULL, NULL, NULL,
		RECORD_THREAD_PRIORITY, 0, 0);

void record_thread_start(void)
{
	if (!s_busy) {
		k_sem_give(&s_sem_run);
	}
}
