/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>
#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/kscan.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#include "AudioSystem.h"
#include "AudioTrack.h"
#include "AudioRecord.h"

#include "play_thread.h"
#include "record_thread.h"
#include "duplex_config.h"

/*
 * 多通道录音 + 逐通道播音样例:
 *   SW2 录音 5 秒, 录完后自动依次播音每个通道;
 *   K1 重播上一次录音的逐通道序列, K2 停止, K3 静音, K4 取消静音。
 *
 * 默认配置: 4 路麦克风 (ADC0-3) 48000 Hz, 采样率在 AP 运行期通过
 * AudioSystem_setParameters() 配置, DSP 固件无需为 48K 单独编译。
 * 通道/采样率/ECHO 开关见 duplex_config.h。
 */

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

	LOG_INF("Key %d pressed at %" PRIu32, row, k_cycle_get_32());

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

	LOG_INF("Hello World! %s", CONFIG_BOARD);

	/*
	 * 增益配置示例: 4 路麦克风 (LEFT/RIGHT = ADC0/1, AUX_LEFT/AUX_RIGHT =
	 * ADC2/3) + 2 路喇叭 (DAC_LEFT/RIGHT)。
	 */
	String8 param;
	String8_ctor_char(&param, "ADC_PDM_GAIN_A_LEFT=6;"
				  "ADC_PDM_GAIN_D_LEFT=20;"
				  "ADC_PDM_GAIN_A_RIGHT=6;"
				  "ADC_PDM_GAIN_D_RIGHT=20;"
				  "ADC_PDM_GAIN_A_AUX_LEFT=6;"
				  "ADC_PDM_GAIN_D_AUX_LEFT=20;"
				  "ADC_PDM_GAIN_A_AUX_RIGHT=6;"
				  "ADC_PDM_GAIN_D_AUX_RIGHT=20;"
				  "DAC_GAIN_A_LEFT=-6;"
				  "DAC_GAIN_D_LEFT=0;"
				  "DAC_GAIN_A_RIGHT=-6;"
				  "DAC_GAIN_D_RIGHT=0");
	ret = AudioSystem_setParameters(0, &param);
	String8_dtor(&param);
	assert(0 == ret);

	/*
	 * 运行期配置采样率/通道数。注意 "samplingRate" 键同时作用于
	 * 录音 (ADC) 和播音 (DAC), 因此 AudioTrack 也用 SAMPLE_RATE 打开。
	 * 须在 openRecord/播音数据流通之前调用;
	 * 带 ECHO 时 48K 请求会被 DSP 固件拒绝 (见文件头注释)。
	 */
	char cfg[64];
	snprintf(cfg, sizeof(cfg), "samplingRate=%d;channels=%d", SAMPLE_RATE, RECORD_CHNS);
	String8_ctor_char(&param, cfg);
	ret = AudioSystem_setParameters(0, &param);
	String8_dtor(&param);
	assert(0 == ret);

	ret = AudioTrack_ctor(&g_track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 0, NULL);
	assert(ret == 0);

	ret = AudioRecord_ctor(&g_record, 0, SAMPLE_RATE, PCM_16_BIT, RECORD_CHNS, 0, NULL);
	assert(ret == 0);

	kscan_config(kscan, kscan_callback);
}
