
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/storage/flash_map.h>

#include <ztest.h>

#include "AudioSystem.h"
#include "AudioTrack.h"
#include "AudioRecord.h"
#include "Errors.h"

#define SAMPLE_RATE (16000)
#define CHANNEL_CNT (1)


#define FLASH_ADDR      (DT_REG_ADDR(DT_CHOSEN(zephyr_flash)))
#define TEST_AUDIO_ADDR (FLASH_ADDR + FLASH_AREA_OFFSET(test_audio))
#define TEST_AUDIO_SIZE (FLASH_AREA_SIZE(test_audio))

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
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_create_stereo_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_STEREO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_unsupport_channel_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, 0, 160, NULL);
	zassert_equal(ret, BAD_VALUE, "Failed to create AudioTrack object");

}

static void test_0_sample_rate_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, 0, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, BAD_VALUE, "Failed to create AudioTrack object");

}

static void test_create_16bit_pcm_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_play_non_offload_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	AudioTrack_start(track);
	ret = AudioTrack_write(track, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");

	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, false, "audio track stopped");
	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_stop_non_offload_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	AudioTrack_start(track);
	ret = AudioTrack_write(track, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");

	AudioTrack_stop(track);

	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, true, "audio track not stopped");
	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_pause_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	AudioTrack_start(track);
	
	ret = AudioTrack_write(track, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");

	AudioTrack_pause(track);

	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);
	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_resume_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	AudioTrack_start(track);
	ret = AudioTrack_write(track, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");

	AudioTrack_pause(track);

	AudioTrack_start(track);

	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);
	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_double_start_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	AudioTrack_start(track);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	ret = AudioTrack_write(track, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");
	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_set_max_volume_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	AudioTrack_setVolume(track, 1.0, 1.0);
	float left;
	float right;
	AudioTrack_getVolume(track, &left, &right);
	zassert_equal(left, 1.0, "left volume error");
	zassert_equal(right, 1.0, "right volume error");

	AudioTrack_start(track);
	
	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);
	zassert_equal(ret, TEST_AUDIO_SIZE, "Failed to write audio track");

	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_set_min_volume_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	AudioTrack_setVolume(track, 0.0, 0.0);
	float left;
	float right;
	AudioTrack_getVolume(track, &left, &right);
	zassert_equal(left, 0.0, "left volume error");
	zassert_equal(right, 0.0, "right volume error");
	AudioTrack_start(track);
	
	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);
	zassert_equal(ret, TEST_AUDIO_SIZE, "Failed to write audio track");

	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_set_average_volume_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	AudioTrack_setVolume(track, 0.5, 0.5);
	float left;
	float right;
	AudioTrack_getVolume(track, &left, &right);
	zassert_equal(left, 0.5, "left volume error");
	zassert_equal(right, 0.5, "right volume error");
	AudioTrack_start(track);
	
	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);
	zassert_equal(ret, TEST_AUDIO_SIZE, "Failed to write audio track");

	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_set_negtive_volume_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	AudioTrack_setVolume(track, -0.5, -0.5);
	float left;
	float right;
	AudioTrack_getVolume(track, &left, &right);
	zassert_equal(left, 0.0, "left volume error");
	zassert_equal(right, 0.0, "right volume error");
	AudioTrack_start(track);
	
	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);
	zassert_equal(ret, TEST_AUDIO_SIZE, "Failed to write audio track");

	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_format_channel_count_frame_count_frame_size_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	ret = AudioTrack_format(track);
	zassert_equal(ret, PCM_16_BIT, "Format error");

	ret = AudioTrack_channelCount(track);
	zassert_equal(ret, CHANNEL_OUT_MONO, "Channel error");

	ret = AudioTrack_frameCount(track);
	zassert_equal(ret, 160, "Frame count error");

	ret = AudioTrack_frameSize(track);
	zassert_equal(ret, sizeof(int16_t), "Frame size error");

	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_reinit_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	ret = AudioTrack_set(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, INVALID_OPERATION, "AudioTrack is already intitialized");

	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_flush_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioTrack_start(track);
	
	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);

	AudioTrack_stop(track);

	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);

	AudioTrack_flush(track);

	AudioTrack_start(track);

	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_obtain_buffer_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioTrack_start(track);

	size_t userSize = TEST_AUDIO_SIZE;
	ssize_t written = 0;
    const int8_t *src = (const int8_t *)TEST_AUDIO_ADDR;
    AudioTrack_Buffer audioBuffer;

    do {
        audioBuffer.frameCount = userSize/AudioTrack_frameSize(track);

        // Calling obtainBuffer() with a negative wait count causes
        // an (almost) infinite wait time.
        status_t err = AudioTrack_obtainBuffer(track, &audioBuffer, -1);
        if (err < 0) {
            // out of buffers, return #bytes written
            if (err == (status_t)NO_MORE_BUFFERS)
                break;
			printk("err:%d\n", err);
            return;
        }
		else
		{			
			zassert_equal(err, NO_ERROR, "Failed to obtain buffer");
		}

        size_t toWrite;

        toWrite = audioBuffer.size;
        memcpy(audioBuffer.i8, src, toWrite);
        src += toWrite;

        userSize -= toWrite;
        written += toWrite;

        AudioTrack_releaseBuffer(track, &audioBuffer);
    } while (userSize);


	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_mute_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioTrack_start(track);

	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, false, "audio track stopped");

	AudioTrack_mute(track, true);

	return_value = AudioTrack_muted(track);
	zassert_equal(return_value, true, "audio track muted");

	size_t userSize = TEST_AUDIO_SIZE;
	ssize_t written = 0;
    const int8_t *src = (const int8_t *)TEST_AUDIO_ADDR;
    AudioTrack_Buffer audioBuffer;

    do {
        audioBuffer.frameCount = userSize/AudioTrack_frameSize(track);

        // Calling obtainBuffer() with a negative wait count causes
        // an (almost) infinite wait time.
        status_t err = AudioTrack_obtainBuffer(track, &audioBuffer, -1);
        if (err < 0) {
            // out of buffers, return #bytes written
            if (err == (status_t)NO_MORE_BUFFERS)
                break;
			printk("err:%d\n", err);
            return;
        }
		else
		{			
			zassert_equal(err, NO_ERROR, "Failed to obtain buffer");
		}

        size_t toWrite;

        toWrite = audioBuffer.size;
        memcpy(audioBuffer.i8, src, toWrite);
        src += toWrite;

        userSize -= toWrite;
        written += toWrite;

        AudioTrack_releaseBuffer(track, &audioBuffer);
    } while (userSize);

	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_unmute_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 160, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioTrack_start(track);

	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, false, "audio track stopped");

	AudioTrack_mute(track, true);

	return_value = AudioTrack_muted(track);
	zassert_equal(return_value, true, "audio track muted");

	AudioTrack_mute(track, false);

	return_value = AudioTrack_muted(track);
	zassert_equal(return_value, false, "audio track unmuted");

	size_t userSize = TEST_AUDIO_SIZE;
	ssize_t written = 0;
    const int8_t *src = (const int8_t *)TEST_AUDIO_ADDR;
    AudioTrack_Buffer audioBuffer;

    do {
        audioBuffer.frameCount = userSize/AudioTrack_frameSize(track);

        // Calling obtainBuffer() with a negative wait count causes
        // an (almost) infinite wait time.
        status_t err = AudioTrack_obtainBuffer(track, &audioBuffer, -1);
        if (err < 0) {
            // out of buffers, return #bytes written
            if (err == (status_t)NO_MORE_BUFFERS)
                break;
			printk("err:%d\n", err);
            return;
        }
		else
		{			
			zassert_equal(err, NO_ERROR, "Failed to obtain buffer");
		}

        size_t toWrite;

        toWrite = audioBuffer.size;
        memcpy(audioBuffer.i8, src, toWrite);
        src += toWrite;

        userSize -= toWrite;
        written += toWrite;

        AudioTrack_releaseBuffer(track, &audioBuffer);
    } while (userSize);

	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

void test_main(void)
{
	ztest_test_suite(audio_track_test, 
					ztest_unit_test(test_create_mono_audio_track), 
					ztest_unit_test(test_create_stereo_audio_track), 
					ztest_unit_test(test_unsupport_channel_audio_track), 
					ztest_unit_test(test_0_sample_rate_audio_track), 
					ztest_unit_test(test_create_16bit_pcm_audio_track), 
					ztest_unit_test(test_play_non_offload_audio_track), 
					ztest_unit_test(test_stop_non_offload_audio_track), 
					ztest_unit_test(test_pause_audio_track), 
					ztest_unit_test(test_resume_audio_track), 
					ztest_unit_test(test_double_start_audio_track), 
					ztest_unit_test(test_set_max_volume_audio_track), 
					ztest_unit_test(test_set_min_volume_audio_track), 
					ztest_unit_test(test_set_average_volume_audio_track), 
					ztest_unit_test(test_set_negtive_volume_audio_track),
					ztest_unit_test(test_format_channel_count_frame_count_frame_size_audio_track), 
					ztest_unit_test(test_reinit_audio_track), 
					ztest_unit_test(test_flush_audio_track), 
					ztest_unit_test(test_obtain_buffer_audio_track), 
					ztest_unit_test(test_mute_audio_track), 
					ztest_unit_test(test_unmute_audio_track));
	ztest_run_test_suite(audio_track_test);
}
