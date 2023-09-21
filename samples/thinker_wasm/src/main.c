/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/storage/flash_map.h>

#include <lsf/services/thinker.h>

#include "apple_input.h"

#define FLASH_ADDR         (DT_REG_ADDR(DT_CHOSEN(zephyr_flash)))
#define THINKER_MODEL_ADDR (FLASH_ADDR + FLASH_AREA_OFFSET(thinker_model))

#define THINKER_TEST_DATA_ADDR (FLASH_ADDR + FLASH_AREA_OFFSET(thinker_test_data))

void main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);

	/* 加载 Thinker 模型，注意传入模型的实际字节数 */
	lsf_thinker_set_model((const void *)THINKER_MODEL_ADDR, 436224);
	printk("Model loaded\n");

	int8_t *output;
	uint32_t output_size;

	/*
	 * 在实际应用中你可能需要持续不断地输入数据，这里仅用一个循环来演示。
	 * 请根据实际情况修改。
	 */
	for (uint32_t i = 0;; i++) {
		k_msleep(200);
		printk("========== LOOP %d ==========\n", i);

		printk("setInput, size: %d\n", input_resource_len);
		lsf_thinker_set_input((const void *)THINKER_TEST_DATA_ADDR, 98304);

		int count = lsf_thinker_get_output_count();
		printk("getOutputCount, count: %d\n", count);

		for (int j = 0; j < count; j++) {
			lsf_thinker_get_output_by_index(j, (void **)&output, &output_size);
			printk("getOutputByIndex, index: %d, ptr: %p, size: %d\n", j, output,
			       output_size);
		}

		/* 故意 get 一个不存在的 index 试试 */
		int ret = lsf_thinker_get_output_by_index(count, (void **)&output, &output_size);
		printk("getOutputByIndex, index: %d, ret: %d\n", count, ret);
	}
}
