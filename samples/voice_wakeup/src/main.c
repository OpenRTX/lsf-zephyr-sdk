/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <lsf/services/vad.h>

#include "AudioSystem.h"

#include "Driver_PWR.h"

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

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
	if (IP_PMUCTRL->REG_AON_STATUS_3.bit.IRQ_AON_WAKEUP) {
		if (IP_PMUCTRL->REG_PMU_CTRL1.bit.IS_VAD_WAKEUP) {
			printk("Wake up by VAD\n");
		} else {
			printk("Wake up by other reason\n");
		}

		/* 清除唤醒标记位 */
		IP_PMUCTRL->REG_AON_STATUS_3.bit.IRQ_AON_WAKEUP = 1;
	} else {
		printk("Power on\n");
	}

	/* 设置麦克风增益 */
	String8 param;
	String8_ctor_char(&param, "ADC_PDM_GAIN_A_LEFT=6;"
				  "ADC_PDM_GAIN_D_LEFT=20;"
				  "ADC_PDM_GAIN_A_RIGHT=6;"
				  "ADC_PDM_GAIN_D_RIGHT=20");
	AudioSystem_setParameters(0, &param);
	String8_dtor(&param);

	printk("Going to sleep after 3 seconds...\n");
	k_sleep(K_SECONDS(3));

	/* 使能 VAD 唤醒 */
	lsf_vad_enable();

	/* 设置 VAD 唤醒并进入休眠模式 */
	HAL_PWR_Control(pwr, CSK_PWR_WAKE_SELECT_VAD);
	HAL_PWR_EnterDeepSleepMode(pwr, 1);
}
