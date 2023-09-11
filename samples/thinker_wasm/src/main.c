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

const char *classes[] = {
	"apple",       "aquarium_fish", "baby",      "bear",       "beaver",       "bed",
	"bee",         "beetle",        "bicycle",   "bottle",     "bowl",         "boy",
	"bridge",      "bus",           "butterfly", "camel",      "can",          "castle",
	"caterpillar", "cattle",        "chair",     "chimpanzee", "clock",        "cloud",
	"cockroach",   "couch",         "crab",      "crocodile",  "cup",          "dinosaur",
	"dolphin",     "elephant",      "flatfish",  "forest",     "fox",          "girl",
	"hamster",     "house",         "kangaroo",  "keyboard",   "lamp",         "lawn_mower",
	"leopard",     "lion",          "lizard",    "lobster",    "man",          "maple_tree",
	"motorcycle",  "mountain",      "mouse",     "mushroom",   "oak_tree",     "orange",
	"orchid",      "otter",         "palm_tree", "pear",       "pickup_truck", "pine_tree",
	"plain",       "plate",         "poppy",     "porcupine",  "possum",       "rabbit",
	"raccoon",     "ray",           "road",      "rocket",     "rose",         "sea",
	"seal",        "shark",         "shrew",     "skunk",      "skyscraper",   "snail",
	"snake",       "spider",        "squirrel",  "streetcar",  "sunflower",    "sweet_pepper",
	"table",       "tank",          "telephone", "television", "tiger",        "tractor",
	"train",       "trout",         "tulip",     "turtle",     "wardrobe",     "whale",
	"willow_tree", "wolf",          "woman",     "worm"};

void main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);

	/* 加载 Thinker 模型，注意传入模型的实际字节数 */
	lsf_thinker_set_model((const void *)THINKER_MODEL_ADDR, 2932416);
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
		lsf_thinker_set_input(input_resource, input_resource_len);

		lsf_thinker_get_output((void **)&output, &output_size);
		printk("getOutput, size: %d\n", output_size);

		int predicted_category_index = 0;
		int8_t max_probability = output[0];
		for (int idx = 1; idx < output_size; idx++) {
			if (output[idx] > max_probability) {
				max_probability = output[idx];
				predicted_category_index = idx;
			}
		}

		printk("Max probability: %d, index: %d\n", max_probability,
		       predicted_category_index);
		printk("Predicted label: %s\n", classes[predicted_category_index]);
	}
}
