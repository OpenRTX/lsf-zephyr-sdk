/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/counter.h>

#include <lsf/services/vad.h>

#include "AON_IOMuxManager.h"

#include "AudioSystem.h"

#include "Driver_PWR.h"
#include "Driver_RTC.h"

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static void init_rtc_wakeup(void)
{
	const struct device *rtc = DEVICE_DT_GET(DT_NODELABEL(rtc1));

	/* 使能 RTC */
	counter_start(rtc);

	/* 获取当前时间 */
	uint32_t now;
	counter_get_value(rtc, &now);

	/* 设置 RTC Alarm */
	struct counter_alarm_cfg alarm_cfg = {.ticks = now + 5};
	counter_set_channel_alarm(rtc, 0, &alarm_cfg);

	/* 使能 RTC Alarm 的唤醒能力 */
	RTC_Control(RTC(), CSK_RTC_CTRL_ALARM_WAKEUP, 1);
}

static void init_vad_wakeup(void)
{
	/* 设置麦克风增益 */
	String8 param;
	String8_ctor_char(&param, "ADC_PDM_GAIN_A_LEFT=0;"
				  "ADC_PDM_GAIN_D_LEFT=0;"
				  "ADC_PDM_GAIN_A_RIGHT=0;"
				  "ADC_PDM_GAIN_D_RIGHT=0");
	AudioSystem_setParameters(0, &param);
	String8_dtor(&param);

	/* 使能 VAD 唤醒 */
	lsf_vad_enable();
}

static void init_gpio_wakeup(void)
{
	/* 设置 GPIO PB5 为唤醒源 */
	AON_IOMuxManager_PinConfigure(CSK_AON_IOMUX_PAD_B, 5, CSK_AON_IOMUX_FUNC_ALTER2);
}

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
		if (IP_PMUCTRL->REG_PMU_CTRL1.bit.IS_RTC_WAKEUP) {
			printk("Wake up by RTC\n");
		} else if (IP_PMUCTRL->REG_PMU_CTRL1.bit.IS_VAD_WAKEUP) {
			printk("Wake up by VAD\n");
		} else if (IP_PMUCTRL->REG_PMU_CTRL1.bit.IS_GPIO_WAKEUP) {
			uint32_t pin = __builtin_ctz(IP_PMUCTRL->REG_PMU_CTRL1.bit.IS_GPIO_WAKEUP);
			printk("Wake up by GPIO %d\n", pin);
		} else {
			printk("Wake up by other reason\n");
		}

		/* 清除唤醒标记位 */
		IP_PMUCTRL->REG_AON_STATUS_3.bit.IRQ_AON_WAKEUP = 1;
	} else {
		printk("Power on\n");
	}

	uint32_t wakeup_select = 0;

	init_rtc_wakeup();
	wakeup_select |= CSK_PWR_WAKE_SELECT_RTC;

	init_vad_wakeup();
	wakeup_select |= CSK_PWR_WAKE_SELECT_VAD;

	init_gpio_wakeup();
	wakeup_select |= CSK_PWR_WAKE_SELECT_GPIOB_5 | CSK_PWR_WAKE_GPIOB_5_POLARITY_LOW;

	/* 设置唤醒源 */
	HAL_PWR_Control(pwr, wakeup_select);

	printk("Going to sleep after 3 seconds...\n");
	k_sleep(K_SECONDS(3));

	/* 进入深度休眠 */
	HAL_PWR_EnterDeepSleepMode(pwr, 1);
}
