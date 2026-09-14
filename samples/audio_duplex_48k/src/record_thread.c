/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(record_thread);

#include "AudioSystem.h"
#include "AudioRecord.h"

#include "play_thread.h"
#include "duplex_config.h"

/*
 * 缓冲几何与 SAMPLE_RATE/RECORD_CHNS_CNT 取自 duplex_config.h:
 * 0 = 4 路麦克风 48000 Hz, 1 = 6 路 (含 ECHO) 16000 Hz
 */

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

extern AudioRecord g_record;

static struct k_sem s_sem_run;
static volatile bool s_busy;

#define SAMPLE_PER_MS (SAMPLE_RATE / 1000)
#define DURATION_MS   (5000)

int16_t g_record_buf[SAMPLE_PER_MS * DURATION_MS * RECORD_CHNS_CNT]
	__attribute__((section(".psram_section")));
ssize_t g_record_bytes;
uint32_t g_record_channels = RECORD_CHNS_CNT;

static void record_thread(void *p1, void *p2, void *p3)
{
	int ret;
	ssize_t read;
	uint32_t start_time, end_time;

	ret = k_sem_init(&s_sem_run, 0, 1);
	assert(ret == 0);

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	assert(0 == ret);

	while (1) {
		LOG_INF("Press SW2 to record");
		k_sem_take(&s_sem_run, K_FOREVER);
		s_busy = true;

		gpio_pin_set_dt(&led, 1);
		LOG_INF("Records 5 seconds...");

		AudioRecord_start(&g_record);

		start_time = k_uptime_get_32();
		read = AudioRecord_read(&g_record, g_record_buf, sizeof(g_record_buf));
		end_time = k_uptime_get_32();

		AudioRecord_stop(&g_record);

		gpio_pin_set_dt(&led, 0);
		LOG_INF("Actual read %d of %d bytes in %d ms", read, sizeof(g_record_buf),
			end_time - start_time);

		LOG_HEXDUMP_INF(g_record_buf, MIN(64, read), "Recorded data");

		g_record_bytes = read;

		/* 录完自动依次播音每个通道 */
		play_thread_inspect(g_record_buf, g_record_bytes, g_record_channels);

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
