/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/kscan.h>

#include "AudioSystem.h"
#include "AudioTrack.h"
#include "AudioRecord.h"

#include "play_thread.h"
#include "record_thread.h"

enum csk_adc_key_t {
	KEY_K4,
	KEY_K3,
	KEY_K2,
	KEY_K1,
	KEY_SW2,
};

static const struct device *kscan = DEVICE_DT_GET(DT_ALIAS(analogkey));

AudioTrack g_track;
AudioRecord g_record;

static void kscan_callback(const struct device *dev, uint32_t row, uint32_t column, bool pressed)
{
	if (!pressed) {
		return;
	}

	printk("Key %d pressed at %" PRIu32 "\n", row, k_cycle_get_32());

	switch (row) {
	case KEY_SW2:
		record_thread_start();
		break;
	case KEY_K1:
		play_thread_start();
		break;
	case KEY_K2:
		play_thread_stop();
		break;
	case KEY_K3:
		play_thread_mute();
		break;
	case KEY_K4:
		play_thread_unmute();
		break;
	}
}

void main(void)
{
	int ret;

	printk("Hello World! %s\n", CONFIG_BOARD);

	String8 param;
	String8_ctor_char(&param, "ADC_PDM_GAIN_A_LEFT=6;"
				  "ADC_PDM_GAIN_D_LEFT=20;"
				  "ADC_PDM_GAIN_A_RIGHT=6;"
				  "ADC_PDM_GAIN_D_RIGHT=20");
	ret = AudioSystem_setParameters(0, &param);
	String8_dtor(&param);
	assert(0 == ret);

	ret = AudioTrack_ctor(&g_track, 16000, PCM_16_BIT, CHANNEL_OUT_MONO, 160, 0);
	assert(ret == 0);

	ret = AudioRecord_ctor(&g_record, 0, 16000, PCM_16_BIT, CHANNEL_IN_STEREO, 160, NULL);
	assert(ret == 0);

	kscan_config(kscan, kscan_callback);
}
