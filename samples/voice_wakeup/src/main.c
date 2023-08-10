/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <lsf/services/vad.h>

#include "AudioSystem.h"
#include "AudioRecord.h"

#include "Driver_PWR.h"

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static AudioRecord s_record_obj;

void main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);

	/* 初始化电源管理模块 */
	void *pwr = PWR();
	HAL_PWR_Initialize(pwr);
	HAL_PWR_PowerControl(pwr, CSK_POWER_FULL);

	/* 初始化并点亮 LED */
	gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);

	/* 获取唤醒原因 */
	int32_t cause = HAL_PWR_GetWakeCause(pwr);
	printk("Wake cause: 0x%x\n", cause);

	/* 初始化音频组件 */
	AudioRecord *record = &s_record_obj;
	AudioRecord_ctor(record, 0, 16000, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	printk("Going to sleep after 3 seconds...\n");
	k_sleep(K_SECONDS(3));

	/* 使能 VAD 唤醒 */
	lsf_vad_enable();

	/* 设置 VAD 唤醒并进入休眠模式 */
	HAL_PWR_Control(pwr, CSK_PWR_WAKE_SELECT_VAD);
	HAL_PWR_EnterDeepSleepMode(pwr, 1);
}
