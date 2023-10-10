
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
static AudioTrack s_track_obj;

__attribute__((section(".psram_section"))) static uint8_t
	recording_l[sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT];
__attribute__((section(".psram_section"))) static uint8_t
	recording_r[sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT];

static void test_16bit_pcm_audio_recorder()
{
	int ret;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioRecord object");

	ret = AudioRecord_dtor(record);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioRecord object");

}

static void test_channel_left_audio_recorder()
{
	int ret;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioRecord object");
	printk("AudioRecord_ctor\n");
	ret = AudioRecord_dtor(record);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioRecord object");
	printk("AudioRecord_dtor\n");

}

static void test_channel_right_audio_recorder()
{
	int ret;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_RIGHT, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioRecord object");

	ret = AudioRecord_dtor(record);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioRecord object");

}

static void test_channel_stereo_audio_recorder()
{
	int ret;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_STEREO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioRecord object");

	ret = AudioRecord_dtor(record);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioRecord object");

}

static void test_0_sample_rate_audio_recorder()
{
	int ret;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, 0, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);
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

	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioRecord object");

	AudioRecord_start(record);

	ret = AudioRecord_read(record, recording_l,
			       sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to read AudioRecord");
}

static void test_read_0_audio_recorder(void)
{
	int ret;
	AudioRecord *record = &s_record_obj;

	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioRecord object");

	AudioRecord_start(record);

	ret = AudioRecord_read(record, recording_l,
			       0);
	zassert_equal(ret, 160*sizeof(int16_t), "Failed to read AudioRecord");
}

static void test_set_analog_gain_to_0_audio_recorder()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");


	String8 param;
	String8_ctor_char(&param, "ADC_PDM_GAIN_A_LEFT=0;ADC_PDM_GAIN_A_RIGHT=0");
	ret = AudioSystem_setParameters(0, &param);
	zassert_equal(ret, NO_ERROR, "Failed to set parameters");

	String8_dtor(&param);

	String8_ctor_char(&param, "ADC_PDM_GAIN_A_LEFT;ADC_PDM_GAIN_A_RIGHT");
	String8 ret_param;
	ret_param = AudioSystem_getParameters(0, &param);
	printk("get param %s\n", ret_param.mString);
	zassert_equal(strcmp(ret_param.mString, "ADC_PDM_GAIN_A_LEFT=0;ADC_PDM_GAIN_A_RIGHT=0"), 0, "Failed to get parameters");

	AudioRecord_start(record);	
	ret = AudioRecord_read(record, recording_l,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	AudioTrack_start(track);
	ret = AudioTrack_write(track, recording_l,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");
	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_set_analog_gain_to_max_audio_recorder()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");


	String8 param;
	String8_ctor_char(&param, "ADC_PDM_GAIN_A_LEFT=36;ADC_PDM_GAIN_A_RIGHT=36");
	ret = AudioSystem_setParameters(0, &param);
	zassert_equal(ret, NO_ERROR, "Failed to set parameters");

	String8_dtor(&param);

	String8_ctor_char(&param, "ADC_PDM_GAIN_A_LEFT;ADC_PDM_GAIN_A_RIGHT");
	String8 ret_param;
	ret_param = AudioSystem_getParameters(0, &param);
	printk("get param %s\n", ret_param.mString);
	zassert_equal(strcmp(ret_param.mString, "ADC_PDM_GAIN_A_LEFT=36;ADC_PDM_GAIN_A_RIGHT=36"), 0, "Failed to get parameters");

	AudioRecord_start(record);	
	ret = AudioRecord_read(record, recording_l,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	AudioTrack_start(track);
	ret = AudioTrack_write(track, recording_l,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");
	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_set_analog_gain_to_min_audio_recorder()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");


	String8 param;
	String8_ctor_char(&param, "ADC_PDM_GAIN_A_LEFT=-12;ADC_PDM_GAIN_A_RIGHT=-12");
	ret = AudioSystem_setParameters(0, &param);
	zassert_equal(ret, NO_ERROR, "Failed to set parameters");

	String8_dtor(&param);

	String8_ctor_char(&param, "ADC_PDM_GAIN_A_LEFT;ADC_PDM_GAIN_A_RIGHT");
	String8 ret_param;
	ret_param = AudioSystem_getParameters(0, &param);
	printk("get param %s\n", ret_param.mString);
	zassert_equal(strcmp(ret_param.mString, "ADC_PDM_GAIN_A_LEFT=-12;ADC_PDM_GAIN_A_RIGHT=-12"), 0, "Failed to get parameters");

	AudioRecord_start(record);	
	ret = AudioRecord_read(record, recording_l,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	AudioTrack_start(track);
	ret = AudioTrack_write(track, recording_l,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");
	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_set_digital_gain_to_0_audio_recorder()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");


	String8 param;
	String8_ctor_char(&param, "ADC_PDM_GAIN_D_LEFT=0;ADC_PDM_GAIN_D_RIGHT=0");
	ret = AudioSystem_setParameters(0, &param);
	zassert_equal(ret, NO_ERROR, "Failed to set parameters");

	String8_dtor(&param);

	String8_ctor_char(&param, "ADC_PDM_GAIN_D_LEFT;ADC_PDM_GAIN_D_RIGHT");
	String8 ret_param;
	ret_param = AudioSystem_getParameters(0, &param);
	printk("get param %s\n", ret_param.mString);
	zassert_equal(strcmp(ret_param.mString, "ADC_PDM_GAIN_D_LEFT=0;ADC_PDM_GAIN_D_RIGHT=0"), 0, "Failed to get parameters");

	AudioRecord_start(record);	
	ret = AudioRecord_read(record, recording_l,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	AudioTrack_start(track);
	ret = AudioTrack_write(track, recording_l,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");
	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_set_digital_gain_to_max_audio_recorder()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");


	String8 param;
	String8_ctor_char(&param, "ADC_PDM_GAIN_D_LEFT=42;ADC_PDM_GAIN_D_RIGHT=42");
	ret = AudioSystem_setParameters(0, &param);
	zassert_equal(ret, NO_ERROR, "Failed to set parameters");

	String8_dtor(&param);

	String8_ctor_char(&param, "ADC_PDM_GAIN_D_LEFT;ADC_PDM_GAIN_D_RIGHT");
	String8 ret_param;
	ret_param = AudioSystem_getParameters(0, &param);
	printk("get param %s\n", ret_param.mString);
	zassert_equal(strcmp(ret_param.mString, "ADC_PDM_GAIN_D_LEFT=42;ADC_PDM_GAIN_D_RIGHT=42"), 0, "Failed to get parameters");

	AudioRecord_start(record);	
	ret = AudioRecord_read(record, recording_l,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	AudioTrack_start(track);
	ret = AudioTrack_write(track, recording_l,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");
	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_set_digital_gain_to_min_audio_recorder()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");


	String8 param;
	String8_ctor_char(&param, "ADC_PDM_GAIN_D_LEFT=-83;ADC_PDM_GAIN_D_RIGHT=-83");
	ret = AudioSystem_setParameters(0, &param);
	zassert_equal(ret, NO_ERROR, "Failed to set parameters");

	String8_dtor(&param);

	String8_ctor_char(&param, "ADC_PDM_GAIN_D_LEFT;ADC_PDM_GAIN_D_RIGHT");
	String8 ret_param;
	ret_param = AudioSystem_getParameters(0, &param);
	printk("get param %s\n", ret_param.mString);
	zassert_equal(strcmp(ret_param.mString, "ADC_PDM_GAIN_D_LEFT=-83;ADC_PDM_GAIN_D_RIGHT=-83"), 0, "Failed to get parameters");

	AudioRecord_start(record);	
	ret = AudioRecord_read(record, recording_l,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	AudioTrack_start(track);
	ret = AudioTrack_write(track, recording_l,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");
	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_set_analog_digital_gain_to_max_audio_recorder()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");


	String8 param;
	String8_ctor_char(&param, "ADC_PDM_GAIN_A_LEFT=36;ADC_PDM_GAIN_A_RIGHT=36;ADC_PDM_GAIN_D_LEFT=42;ADC_PDM_GAIN_D_RIGHT=42");
	ret = AudioSystem_setParameters(0, &param);
	zassert_equal(ret, NO_ERROR, "Failed to set parameters");

	String8_dtor(&param);

	String8_ctor_char(&param, "ADC_PDM_GAIN_A_LEFT;ADC_PDM_GAIN_A_RIGHT;ADC_PDM_GAIN_D_LEFT;ADC_PDM_GAIN_D_RIGHT");
	String8 ret_param;
	ret_param = AudioSystem_getParameters(0, &param);
	printk("get param %s\n", ret_param.mString);
	zassert_equal(strcmp(ret_param.mString, "ADC_PDM_GAIN_A_LEFT=36;ADC_PDM_GAIN_A_RIGHT=36;ADC_PDM_GAIN_D_LEFT=42;ADC_PDM_GAIN_D_RIGHT=42"), 0, "Failed to get parameters");

	AudioRecord_start(record);	
	ret = AudioRecord_read(record, recording_l,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	AudioTrack_start(track);
	ret = AudioTrack_write(track, recording_l,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");
	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_set_record_audio_track_analog_digital_gain_to_max_audio_recorder()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");


	String8 param;
	String8_ctor_char(&param, "DAC_GAIN_A_LEFT=6;DAC_GAIN_A_RIGHT=6;DAC_GAIN_D_LEFT=30;DAC_GAIN_D_RIGHT=30;ADC_PDM_GAIN_A_LEFT=36;ADC_PDM_GAIN_A_RIGHT=36;ADC_PDM_GAIN_D_LEFT=42;ADC_PDM_GAIN_D_RIGHT=42");
	ret = AudioSystem_setParameters(0, &param);
	zassert_equal(ret, NO_ERROR, "Failed to set parameters");

	String8_dtor(&param);

	String8_ctor_char(&param, "ADC_PDM_GAIN_A_LEFT;ADC_PDM_GAIN_A_RIGHT;ADC_PDM_GAIN_D_LEFT;ADC_PDM_GAIN_D_RIGHT;DAC_GAIN_A_LEFT;DAC_GAIN_A_RIGHT;DAC_GAIN_D_LEFT;DAC_GAIN_D_RIGHT");
	String8 ret_param;
	ret_param = AudioSystem_getParameters(0, &param);
	printk("get param %s\n", ret_param.mString);
	zassert_equal(strcmp(ret_param.mString, "ADC_PDM_GAIN_A_LEFT=36;ADC_PDM_GAIN_A_RIGHT=36;ADC_PDM_GAIN_D_LEFT=42;ADC_PDM_GAIN_D_RIGHT=42;DAC_GAIN_A_LEFT=6;DAC_GAIN_A_RIGHT=6;DAC_GAIN_D_LEFT=30;DAC_GAIN_D_RIGHT=30"), 0, "Failed to get parameters");

	AudioRecord_start(record);	
	ret = AudioRecord_read(record, recording_l,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	AudioTrack_start(track);
	ret = AudioTrack_write(track, recording_l,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");
	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_stereo_audio_recorder()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);	
	ret = AudioRecord_read(record, recording_l,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to read AudioRecord");
	ret = AudioRecord_read(record, recording_r,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to read AudioRecord");

	AudioTrack_start(track);
	ret = AudioTrack_write(track, recording_l,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");
	ret = AudioTrack_write(track, recording_r,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");
	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

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
					ztest_unit_test(test_read_0_audio_recorder), 
					ztest_unit_test(test_set_analog_gain_to_0_audio_recorder), 
					ztest_unit_test(test_set_analog_gain_to_max_audio_recorder), 
					ztest_unit_test(test_set_analog_gain_to_min_audio_recorder), 
					ztest_unit_test(test_set_digital_gain_to_0_audio_recorder), 
					ztest_unit_test(test_set_digital_gain_to_max_audio_recorder), 
					ztest_unit_test(test_set_digital_gain_to_min_audio_recorder), 
					ztest_unit_test(test_set_analog_digital_gain_to_max_audio_recorder), 
					ztest_unit_test(test_set_record_audio_track_analog_digital_gain_to_max_audio_recorder), 
					ztest_unit_test(test_stereo_audio_recorder));
	ztest_run_test_suite(audio_recorder_test);
}
