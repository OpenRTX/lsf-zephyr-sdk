
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
	recording[sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT];

static void test_create_mono_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_create_stereo_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_STEREO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_unsupport_channel_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, 0, 320, NULL);
	zassert_equal(ret, BAD_VALUE, "Failed to create AudioTrack object");

}

static void test_0_sample_rate_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, 0, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, BAD_VALUE, "Failed to create AudioTrack object");

}

static void test_create_16bit_pcm_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_create_8bit_pcm_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_8_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_play_non_offload_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 320, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	AudioTrack_start(track);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	ret = AudioTrack_write(track, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, NO_ERROR, "Failed to write audio track");

	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_stop_non_offload_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 320, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	AudioTrack_start(track);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	ret = AudioTrack_write(track, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, NO_ERROR, "Failed to write audio track");

	AudioTrack_stop(track);
	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

void test_main(void)
{
	ztest_test_suite(audio_recorder_test, 
					ztest_unit_test(test_create_mono_audio_track),
					ztest_unit_test(test_create_stereo_audio_track),
					ztest_unit_test(test_unsupport_channel_audio_track),
					ztest_unit_test(test_0_sample_rate_audio_track),
					ztest_unit_test(test_create_16bit_pcm_audio_track),
					ztest_unit_test(test_create_8bit_pcm_audio_track),
					ztest_unit_test(test_play_non_offload_audio_track),
					ztest_unit_test(test_stop_non_offload_audio_track));
	ztest_run_test_suite(audio_recorder_test);
}
