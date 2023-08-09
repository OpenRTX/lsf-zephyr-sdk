


#include <zephyr/kernel.h>
#include <zephyr/device.h>

#include "AudioSystem.h"
#include "AudioTrack.h"
#include "AudioRecord.h"

#include <stdint.h>
#include <string.h>
#include <zephyr/storage/flash_map.h>
#include <ztest.h>

#include <lsf/services/thinker.h>

#include "apple_input.h"


#define SAMPLE_RATE (16000)
#define CHANNEL_CNT (1)

#define RECORD_SECS       (5)
#define RECORD_SAMPLE_CNT (SAMPLE_RATE * RECORD_SECS)

static AudioRecord s_record_obj;
static AudioTrack s_track_obj;

__attribute__((section(".psram_section"))) static uint8_t
	recording[sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT];

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

static void test_thinker()
{
	/* 加载 Thinker 模型，注意传入模型的实际字节数 */
	lsf_thinker_set_model((const void *)THINKER_MODEL_ADDR, 2932400);
	printk("Model loaded\n");

	int8_t *output;
	uint32_t output_size;

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

	zassert_equal(strcmp(classes[predicted_category_index], "apple"), 0, "Failed to predict apple");

}

static void test_record_and_track_thinker()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 320, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	AudioTrack_start(track);
	ret = AudioTrack_write(track, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, NO_ERROR, "Failed to write audio track");

	lsf_thinker_set_model((const void *)THINKER_MODEL_ADDR, 2932400);
	printk("Model loaded\n");

	int8_t *output;
	uint32_t output_size;

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

	zassert_equal(strcmp(classes[predicted_category_index], "apple"), 0, "Failed to predict apple");

	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);
}

static void test_thinker_set_model_0()
{
	/* 加载 Thinker 模型，注意传入模型的实际字节数 */
	lsf_thinker_set_model((const void *)THINKER_MODEL_ADDR, 0);
	printk("Model loaded\n");

	int8_t *output;
	uint32_t output_size;

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
	zassert_equal(strcmp(classes[predicted_category_index], "apple"), 0, "Failed to predict apple");

}

static void test_thinker_set_input_size_0()
{
	/* 加载 Thinker 模型，注意传入模型的实际字节数 */
	lsf_thinker_set_model((const void *)THINKER_MODEL_ADDR, 2932400);
	printk("Model loaded\n");

	int8_t *output;
	uint32_t output_size;

	printk("setInput, size: %d\n", 0);
	lsf_thinker_set_input(input_resource, 0);

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
	zassert_equal(strcmp(classes[predicted_category_index], "apple"), 0, "Failed to predict apple");

}

static void test_record_and_track_thinker_100_times()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 320, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	AudioTrack_start(track);
	ret = AudioTrack_write(track, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, NO_ERROR, "Failed to write audio track");

	lsf_thinker_set_model((const void *)THINKER_MODEL_ADDR, 2932400);
	printk("Model loaded\n");

	int8_t *output;
	uint32_t output_size;

	for (int i=1;i<100;i++)
	{
		printk("count:%d\n", i);
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

		zassert_equal(strcmp(classes[predicted_category_index], "apple"), 0, "Failed to predict apple");
	}

	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);
}

void test_main(void)
{
	ztest_test_suite(thinker_wasm_test,
					ztest_unit_test(test_thinker),
					ztest_unit_test(test_record_and_track_thinker),
					ztest_unit_test(test_thinker_set_model_0),
					ztest_unit_test(test_thinker_set_input_size_0),
					ztest_unit_test(test_record_and_track_thinker_100_times));
	ztest_run_test_suite(thinker_wasm_test);
}
