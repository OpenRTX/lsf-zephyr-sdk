/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/storage/flash_map.h>

#include <lsf/dsp.h>
#include <lsf/controller.h>
#include <lsf/services/wasm.h>

#define FLASH_ADDR        DT_REG_ADDR(DT_CHOSEN(zephyr_flash))
#define DSP_FIRMWARE_NODE DT_NODE_BY_FIXED_PARTITION_LABEL(dsp_firmware)
#define WASM_APP_NODE     DT_NODE_BY_FIXED_PARTITION_LABEL(wasm_app)

static const struct gpio_dt_spec btn = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

static struct gpio_callback btn_cb_data;
static struct k_sem btn_sem;

static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	k_sem_give(&btn_sem);
}

void main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);

	/* 初始化按钮 */
	k_sem_init(&btn_sem, 0, 1);
	gpio_pin_configure_dt(&btn, GPIO_INPUT);
	gpio_pin_interrupt_configure_dt(&btn, GPIO_INT_EDGE_TO_ACTIVE);
	gpio_init_callback(&btn_cb_data, button_pressed, BIT(btn.pin));
	gpio_add_callback(btn.port, &btn_cb_data);

	printk("Press the button to load DSP firmware\n");
	k_sem_take(&btn_sem, K_FOREVER);

	/* 加载 DSP 固件 */
	const void *dsp_addr = (const void *)(FLASH_ADDR + DT_REG_ADDR(DSP_FIRMWARE_NODE));
	uint32_t dsp_size = DT_REG_SIZE(DSP_FIRMWARE_NODE);
	printk("Loading DSP firmware from %p, size: %u\n", dsp_addr, dsp_size);
	lsf_dsp_load(dsp_addr, dsp_size);

	/* 初始化 LSF */
	lsf_controller_init();
	printk("DSP ready\n");

	printk("Press the button to load WASM app\n");
	k_sem_take(&btn_sem, K_FOREVER);

	/* 加载 WASM 应用 */
	const void *wasm_addr = (const void *)(FLASH_ADDR + DT_REG_ADDR(WASM_APP_NODE));
	uint32_t wasm_size = DT_REG_SIZE(WASM_APP_NODE);
	printk("Loading WASM app from %p, size: %u\n", wasm_addr, wasm_size);
	lsf_wasm_load_app(wasm_addr, wasm_size);

	printk("WASM app ready\n");

	while (1) {
	}
}
