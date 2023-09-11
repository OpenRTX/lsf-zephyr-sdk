/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>

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

static int littlefs_flash_erase(unsigned int id)
{
	const struct flash_area *pfa;
	int rc;

	rc = flash_area_open(id, &pfa);
	if (rc < 0) {
		printk("FAIL: unable to find flash area %u: %d\n", id, rc);
		return rc;
	}

	printk("Area %u at 0x%x on %s for %u bytes\n", id, (unsigned int)pfa->fa_off,
	       pfa->fa_dev_name, (unsigned int)pfa->fa_size);

	printk("Erasing flash area (size %d) ...\n", pfa->fa_size);
	rc = flash_area_erase(pfa, 0, pfa->fa_size);
	printk("Erased: %d\n", rc);

	flash_area_close(pfa);
	return rc;
}

#define PARTITION_NODE DT_NODELABEL(lfs1)

FS_FSTAB_DECLARE_ENTRY(PARTITION_NODE);

struct fs_mount_t *mp = &FS_FSTAB_ENTRY(PARTITION_NODE);

static int littlefs_mount(struct fs_mount_t *mp)
{
	int rc;

	rc = littlefs_flash_erase((uintptr_t)mp->storage_dev);
	if (rc < 0) {
		return rc;
	}

	rc = fs_mount(mp);
	if (rc < 0) {
		printk("FAIL: mount id %" PRIuPTR " at %s: %d\n", (uintptr_t)mp->storage_dev,
		       mp->mnt_point, rc);
		return rc;
	}
	printk("%s mount: %d\n", mp->mnt_point, rc);

	return 0;
}

void main(void)
{
	int ret;

	printk("Hello World! %s\n", CONFIG_BOARD);

	/* 加载 Thinker 模型，注意传入模型的实际字节数 */
	lsf_thinker_set_model((const void *)THINKER_MODEL_ADDR, 2932416);
	printk("Model loaded\n");

	int8_t *output;
	uint32_t output_size;

	ret = littlefs_mount(mp);
	assert(0 == ret);

	struct fs_statvfs sbuf;
	ret = fs_statvfs(mp->mnt_point, &sbuf);
	assert(0 == ret);

	printk("%s: bsize = %lu ; frsize = %lu ;"
	       " blocks = %lu ; bfree = %lu\n",
	       mp->mnt_point, sbuf.f_bsize, sbuf.f_frsize, sbuf.f_blocks, sbuf.f_bfree);

	char fname[255];
	snprintf(fname, sizeof(fname), "%s/pattern.bin", mp->mnt_point);

	struct fs_file_t file;
	fs_file_t_init(&file);
	ret = fs_open(&file, fname, FS_O_CREATE | FS_O_RDWR);
	printk("fs_open %s: %d\n", fname, ret);
	assert(0 == ret);

	/*
	 * 在实际应用中你可能需要持续不断地输入数据，这里仅用一个循环来演示。
	 * 请根据实际情况修改。
	 */
	for (uint32_t i = 0;; i++) {
		k_msleep(1000);
		printk("========== LOOP %d ==========\n", i);

		printk("setInput, size: %d\n", input_resource_len);
		lsf_thinker_set_input(input_resource, input_resource_len);

		lsf_thinker_get_output((void **)&output, &output_size);
		printk("getOutput, size: %d\n", output_size);

		ret = fs_write(&file, (const void *)output, output_size);

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
