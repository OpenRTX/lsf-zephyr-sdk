/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/kscan.h>
#include <zephyr/drivers/counter.h>

#include <lsf/services/vad.h>

#include "AON_IOMuxManager.h"
#include "IOMuxManager.h"

#include "AudioSystem.h"

#include "Driver_PWR.h"
#include "Driver_RTC.h"
#include "Driver_KEYSENSE.h"

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static const struct device *rtc = DEVICE_DT_GET(DT_NODELABEL(rtc1));

static const struct device *kscan = DEVICE_DT_GET(DT_ALIAS(analogkey));

static void kscan_callback(const struct device *dev, uint32_t row, uint32_t column, bool pressed)
{
	if (!pressed) {
		return;
	}

	printk("Key %d pressed at %" PRIu32 "\n", row, k_cycle_get_32());
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

	/* 设置麦克风增益 */
	String8 param;
	String8_ctor_char(&param, "ADC_PDM_GAIN_A_LEFT=0;"
				  "ADC_PDM_GAIN_D_LEFT=0;"
				  "ADC_PDM_GAIN_A_RIGHT=0;"
				  "ADC_PDM_GAIN_D_RIGHT=0");
	AudioSystem_setParameters(0, &param);
	String8_dtor(&param);

	counter_start(rtc);

	uint32_t now;
	counter_get_value(rtc, &now);

	struct counter_alarm_cfg alarm_cfg = {.ticks = now + 5};
	counter_set_channel_alarm(rtc, 0, &alarm_cfg);
	RTC_Control(RTC(), CSK_RTC_CTRL_ALARM_WAKEUP, 1);

	// AON_IOMuxManager_PinConfigure(CSK_IOMUX_PAD_B, 5, CSK_IOMUX_FUNC_ALTER2);

	AON_IOMuxManager_PinConfigure(CSK_AON_IOMUX_PAD_B, 5, CSK_IOMUX_FUNC_DEFAULT);
	kscan_config(kscan, kscan_callback);
	HAL_KEYSENSE_InterruptEnable(KEYSENSE(), CSK_KEYSENSE_INTERRUPT_MODE_WAKEUP);

	printk("Going to sleep after 3 seconds...\n");
	k_sleep(K_SECONDS(3));

	/* 使能 VAD 唤醒 */
	lsf_vad_enable();

	/* 设置 VAD 唤醒并进入休眠模式 */
	uint32_t wakeup_select = 0;
	wakeup_select |= CSK_PWR_WAKE_SELECT_VAD;
	// wakeup_select |= CSK_PWR_WAKE_SELECT_GPIOB_5 | CSK_PWR_WAKE_GPIOB_5_POLARITY_LOW;
	wakeup_select |= CSK_PWR_WAKE_SELECT_RTC;
	wakeup_select |= CSK_PWR_WAKE_SELECT_KEY;
	HAL_PWR_Control(pwr, wakeup_select);
	HAL_PWR_EnterDeepSleepMode(pwr, 1);
}
