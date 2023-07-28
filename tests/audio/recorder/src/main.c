
#include <zephyr/kernel.h>
#include <zephyr/device.h>

#include <ztest.h>

#include "AudioSystem.h"
#include "AudioTrack.h"
#include "AudioRecord.h"
#include "Errors.h"

#define SAMPLE_RATE (16000)
#define CHANNEL_CNT (1)

#define RECORD_SECS       (5)
#define RECORD_SAMPLE_CNT (SAMPLE_RATE * RECORD_SECS)

static AudioRecord s_record_obj;

__attribute__((section(".psram_section"))) static uint8_t
	recording[sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT];

static void test_16bit_pcm_audio_recorder()
{
	int ret;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioRecord object");

	ret = AudioRecord_dtor(record);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioRecord object");

}

static void test_channel_left_audio_recorder()
{
	int ret;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioRecord object");

	ret = AudioRecord_dtor(record);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioRecord object");

}

static void test_channel_right_audio_recorder()
{
	int ret;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_RIGHT, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioRecord object");

	ret = AudioRecord_dtor(record);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioRecord object");

}

static void test_channel_stereo_audio_recorder()
{
	int ret;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_STEREO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioRecord object");

	ret = AudioRecord_dtor(record);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioRecord object");

}

static void test_0_sample_rate_audio_recorder()
{
	int ret;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, 0, PCM_16_BIT, CHANNEL_IN_LEFT, 320, NULL);
	zassert_equal(ret, BAD_VALUE, "Failed to create AudioRecord object");

}

static void test_negative_frame_count_audio_recorder()
{
	int ret;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, -1, NULL);
	zassert_equal(ret, BAD_VALUE, "Failed to create AudioRecord object");

}

static void test_audio_recorder(void)
{
	int ret;
	AudioRecord *record = &s_record_obj;

	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioRecord object");

	AudioRecord_start(record);

	ret = AudioRecord_read(record, recording,
			       sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to read AudioRecord");
}

static void test_read_0_audio_recorder(void)
{
	int ret;
	AudioRecord *record = &s_record_obj;

	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioRecord object");

	AudioRecord_start(record);

	ret = AudioRecord_read(record, recording,
			       0);
	zassert_equal(ret, 0, "Failed to read AudioRecord");
}

void test_main(void)
{
	ztest_test_suite(audio_recorder_test, 
					ztest_unit_test(test_16bit_pcm_audio_recorder), 
					ztest_unit_test(test_channel_left_audio_recorder),
					ztest_unit_test(test_channel_right_audio_recorder),
					ztest_unit_test(test_channel_stereo_audio_recorder),
					ztest_unit_test(test_0_sample_rate_audio_recorder),
					ztest_unit_test(test_negative_frame_count_audio_recorder),
					ztest_unit_test(test_audio_recorder),
					ztest_unit_test(test_read_0_audio_recorder));
	ztest_run_test_suite(audio_recorder_test);
}
