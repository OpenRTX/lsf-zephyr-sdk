
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
#define TEST_AAC_AUDIO_ADDR (FLASH_ADDR + FLASH_AREA_OFFSET(test_aac_audio))
#define TEST_AAC_AUDIO_SIZE (FLASH_AREA_SIZE(test_aac_audio))

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

static void test_create_too_many_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioTrack track2;
	ret = AudioTrack_ctor(&track2, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_not_equal(ret, NO_ERROR, "Should fail on double create AudioTrack object");

	AudioTrack_dtor(track);
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
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

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

static void test_play_non_offload_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	AudioRecord_stop(record);

	AudioTrack_start(track);
	ret = AudioTrack_write(track, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");

	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, false, "audio track stopped");
	AudioTrack_stop(track);
	return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, true, "audio track not stopped");

	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_stop_non_offload_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);
	printk("AudioRecord_ctor");
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");
	printk("AudioTrack_ctor");

	AudioRecord_start(record);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	AudioRecord_stop(record);

	AudioTrack_start(track);
	ret = AudioTrack_write(track, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");
	AudioTrack_stop(track);
	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, true, "audio track not stopped");

	AudioTrack_stop(track);

	return_value = AudioTrack_stopped(track);
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

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	AudioRecord_stop(record);

	AudioTrack_start(track);
	
	ret = AudioTrack_write(track, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");

	AudioTrack_pause(track);

	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);
	AudioTrack_stop(track);
	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, true, "audio track not stopped");
	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_resume_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	AudioRecord_stop(record);

	AudioTrack_start(track);
	ret = AudioTrack_write(track, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");

	AudioTrack_pause(track);

	AudioTrack_start(track);

	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);
	AudioTrack_stop(track);
	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, true, "audio track not stopped");
	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_double_start_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	AudioTrack_start(track);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	AudioRecord_stop(record);

	ret = AudioTrack_write(track, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	zassert_equal(ret, sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT, "Failed to write audio track");
	AudioTrack_stop(track);
	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, true, "audio track not stopped");
	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_set_max_volume_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	AudioRecord_stop(record);

	AudioTrack_setVolume(track, 1.0, 1.0);
	float left;
	float right;
	AudioTrack_getVolume(track, &left, &right);
	zassert_equal(left, 1.0, "left volume error");
	zassert_equal(right, 1.0, "right volume error");

	AudioTrack_start(track);
	
	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);
	zassert_true(ret>0, "Failed to write audio track");
	AudioTrack_stop(track);
	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, true, "audio track not stopped");

	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_set_min_volume_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	AudioRecord_stop(record);

	AudioTrack_setVolume(track, 0.0, 0.0);
	float left;
	float right;
	AudioTrack_getVolume(track, &left, &right);
	zassert_equal(left, 0.0, "left volume error");
	zassert_equal(right, 0.0, "right volume error");
	AudioTrack_start(track);
	
	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);
	zassert_true(ret>0, "Failed to write audio track");
	AudioTrack_stop(track);
	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, true, "audio track not stopped");

	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_set_average_volume_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	AudioRecord_stop(record);

	AudioTrack_setVolume(track, 0.5f, 0.5f);
	float left;
	float right;
	AudioTrack_getVolume(track, &left, &right);
	zassert_equal(left, 0.5f, "left volume error");
	zassert_equal(right, 0.5f, "right volume error");
	AudioTrack_start(track);
	
	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);
	zassert_true(ret>0, "Failed to write audio track");
	AudioTrack_stop(track);
	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, true, "audio track not stopped");

	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_set_negtive_volume_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);
	AudioRecord_stop(record);

	AudioTrack_setVolume(track, -0.5, -0.5);
	float left;
	float right;
	AudioTrack_getVolume(track, &left, &right);
	zassert_equal(left, 0.0, "left volume error");
	zassert_equal(right, 0.0, "right volume error");
	AudioTrack_start(track);
	
	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);
	zassert_true(ret>0, "Failed to write audio track");
	AudioTrack_stop(track);
	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, true, "audio track not stopped");

	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

static void test_format_channel_count_frame_count_frame_size_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	ret = AudioTrack_format(track);
	zassert_equal(ret, PCM_16_BIT, "Format error");

	ret = AudioTrack_channelCount(track);
	zassert_equal(ret, 1, "Channel error");

	ret = AudioTrack_frameCount(track);
	zassert_equal(ret, 320, "Frame count error");

	ret = AudioTrack_frameSize(track);
	zassert_equal(ret, sizeof(int16_t), "Frame size error");

	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_reinit_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	ret = AudioTrack_set(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, INVALID_OPERATION, "AudioTrack is already intitialized");

	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_flush_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
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
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
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


	AudioTrack_stop(track);
	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_mute_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
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

	AudioTrack_stop(track);
	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_unmute_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
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

	AudioTrack_stop(track);
	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_set_analog_gain_to_0_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	String8 param;
	String8_ctor_char(&param, "DAC_GAIN_A_LEFT=0;DAC_GAIN_A_RIGHT=0");
	ret = AudioSystem_setParameters(0, &param);
	zassert_equal(ret, NO_ERROR, "Failed to set parameters");

	String8_dtor(&param);

	String8_ctor_char(&param, "DAC_GAIN_A_LEFT;DAC_GAIN_A_RIGHT");
	String8 ret_param;
	ret_param = AudioSystem_getParameters(0, &param);
	printk("get param %s\n", ret_param.mString);
	zassert_equal(strcmp(ret_param.mString, "DAC_GAIN_A_LEFT=0;DAC_GAIN_A_RIGHT=0"), 0, "Failed to get parameters");

	AudioTrack_start(track);

	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, false, "audio track stopped");

	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);

	AudioTrack_stop(track);
	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_set_analog_gain_to_max_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	String8 param;
	String8_ctor_char(&param, "DAC_GAIN_A_LEFT=6;DAC_GAIN_A_RIGHT=6");
	ret = AudioSystem_setParameters(0, &param);
	zassert_equal(ret, NO_ERROR, "Failed to set parameters");

	String8_dtor(&param);

	String8_ctor_char(&param, "DAC_GAIN_A_LEFT;DAC_GAIN_A_RIGHT");
	String8 ret_param;
	ret_param = AudioSystem_getParameters(0, &param);
	printk("get param %s\n", ret_param.mString);
	zassert_equal(strcmp(ret_param.mString, "DAC_GAIN_A_LEFT=6;DAC_GAIN_A_RIGHT=6"), 0, "Failed to get parameters");

	AudioTrack_start(track);

	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, false, "audio track stopped");

	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);

	AudioTrack_stop(track);
	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_set_analog_gain_to_min_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	String8 param;
	String8_ctor_char(&param, "DAC_GAIN_A_LEFT=-24;DAC_GAIN_A_RIGHT=-24");
	ret = AudioSystem_setParameters(0, &param);
	zassert_equal(ret, NO_ERROR, "Failed to set parameters");

	String8_dtor(&param);

	String8_ctor_char(&param, "DAC_GAIN_A_LEFT;DAC_GAIN_A_RIGHT");
	String8 ret_param;
	ret_param = AudioSystem_getParameters(0, &param);
	printk("get param %s\n", ret_param.mString);
	zassert_equal(strcmp(ret_param.mString, "DAC_GAIN_A_LEFT=-24;DAC_GAIN_A_RIGHT=-24"), 0, "Failed to get parameters");

	AudioTrack_start(track);

	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, false, "audio track stopped");

	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);

	AudioTrack_stop(track);
	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_set_digital_gain_to_0_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	String8 param;
	String8_ctor_char(&param, "DAC_GAIN_D_LEFT=0;DAC_GAIN_D_RIGHT=0");
	ret = AudioSystem_setParameters(0, &param);
	zassert_equal(ret, NO_ERROR, "Failed to set parameters");

	String8_dtor(&param);

	String8_ctor_char(&param, "DAC_GAIN_D_LEFT;DAC_GAIN_D_RIGHT");
	String8 ret_param;
	ret_param = AudioSystem_getParameters(0, &param);
	printk("get param %s\n", ret_param.mString);
	zassert_equal(strcmp(ret_param.mString, "DAC_GAIN_D_LEFT=0;DAC_GAIN_D_RIGHT=0"), 0, "Failed to get parameters");

	AudioTrack_start(track);

	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, false, "audio track stopped");

	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);

	AudioTrack_stop(track);
	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_set_digital_gain_to_max_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	String8 param;
	String8_ctor_char(&param, "DAC_GAIN_D_LEFT=30;DAC_GAIN_D_RIGHT=30");
	ret = AudioSystem_setParameters(0, &param);
	zassert_equal(ret, NO_ERROR, "Failed to set parameters");

	String8_dtor(&param);

	String8_ctor_char(&param, "DAC_GAIN_D_LEFT;DAC_GAIN_D_RIGHT");
	String8 ret_param;
	ret_param = AudioSystem_getParameters(0, &param);
	printk("get param %s\n", ret_param.mString);
	zassert_equal(strcmp(ret_param.mString, "DAC_GAIN_D_LEFT=30;DAC_GAIN_D_RIGHT=30"), 0, "Failed to get parameters");

	AudioTrack_start(track);

	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, false, "audio track stopped");

	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);

	AudioTrack_stop(track);
	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_set_digital_gain_to_min_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	String8 param;
	String8_ctor_char(&param, "DAC_GAIN_D_LEFT=-113;DAC_GAIN_D_RIGHT=-113");
	ret = AudioSystem_setParameters(0, &param);
	zassert_equal(ret, NO_ERROR, "Failed to set parameters");

	String8_dtor(&param);

	String8_ctor_char(&param, "DAC_GAIN_D_LEFT;DAC_GAIN_D_RIGHT");
	String8 ret_param;
	ret_param = AudioSystem_getParameters(0, &param);
	printk("get param %s\n", ret_param.mString);
	zassert_equal(strcmp(ret_param.mString, "DAC_GAIN_D_LEFT=-113;DAC_GAIN_D_RIGHT=-113"), 0, "Failed to get parameters");

	AudioTrack_start(track);

	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, false, "audio track stopped");

	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);

	AudioTrack_stop(track);
	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_set_analog_digital_gain_to_max_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	String8 param;
	String8_ctor_char(&param, "DAC_GAIN_A_LEFT=6;DAC_GAIN_A_RIGHT=6;DAC_GAIN_D_LEFT=30;DAC_GAIN_D_RIGHT=30");
	ret = AudioSystem_setParameters(0, &param);
	zassert_equal(ret, NO_ERROR, "Failed to set parameters");

	String8_dtor(&param);

	String8_ctor_char(&param, "DAC_GAIN_A_LEFT;DAC_GAIN_A_RIGHT;DAC_GAIN_D_LEFT;DAC_GAIN_D_RIGHT");
	String8 ret_param;
	ret_param = AudioSystem_getParameters(0, &param);
	printk("get param %s\n", ret_param.mString);
	zassert_equal(strcmp(ret_param.mString, "DAC_GAIN_A_LEFT=6;DAC_GAIN_A_RIGHT=6;DAC_GAIN_D_LEFT=30;DAC_GAIN_D_RIGHT=30"), 0, "Failed to get parameters");

	AudioTrack_start(track);

	bool return_value = AudioTrack_stopped(track);
	zassert_equal(return_value, false, "audio track stopped");

	ret = AudioTrack_write(track, (const void *)TEST_AUDIO_ADDR, TEST_AUDIO_SIZE);

	AudioTrack_stop(track);
	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_set_illegal_gain_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	
	ret = AudioTrack_ctor(track, SAMPLE_RATE, PCM_16_BIT, CHANNEL_OUT_MONO, 320, NULL);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	String8 param;
	String8_ctor_char(&param, "DAC_GAIN_A_LEFT=XXX");
	ret = AudioSystem_setParameters(0, &param);
	printk("ret:%d\n", ret);
	zassert_true(ret<0, "Failed to set parameters");

	String8_dtor(&param);

	ret = AudioTrack_dtor(track);
	zassert_equal(ret, NO_ERROR, "Failed to release AudioTrack object");

}

static void test_play_16k_aac_audio_track()
{
	int ret;
	AudioTrack *track = &s_track_obj;
	AudioRecord *record = &s_record_obj;
	
	ret = AudioRecord_ctor(record, 0, SAMPLE_RATE, PCM_16_BIT, CHANNEL_IN_LEFT, 160, NULL);

	audio_offload_info_t offload_info = {
		.channel_mask = CHANNEL_OUT_MONO,
		.format = AAC,
		.sample_rate = SAMPLE_RATE,
	};
	ret = AudioTrack_ctor(track, SAMPLE_RATE, AAC, CHANNEL_OUT_MONO, 0, &offload_info);
	zassert_equal(ret, NO_ERROR, "Failed to create AudioTrack object");

	AudioRecord_start(record);
	
	ret = AudioRecord_read(record, recording,
					sizeof(uint16_t) * RECORD_SAMPLE_CNT * CHANNEL_CNT);

	AudioRecord_stop(record);
	AudioTrack_start(track);
	
	ret = AudioTrack_write(track, (const void *)TEST_AAC_AUDIO_ADDR, TEST_AAC_AUDIO_SIZE);
	zassert_true(ret>0, "Failed to write audio track");

	AudioTrack_stop(track);
	
	ret = AudioTrack_dtor(track);
	ret = AudioRecord_dtor(record);

}

void test_main(void)
{
	ztest_test_suite(audio_track_test, 
					ztest_unit_test(test_create_mono_audio_track), 
					ztest_unit_test(test_create_stereo_audio_track), 
					ztest_unit_test(test_create_too_many_audio_track), 
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
					ztest_unit_test(test_unmute_audio_track), 
					ztest_unit_test(test_set_analog_gain_to_0_audio_track), 
					ztest_unit_test(test_set_analog_gain_to_max_audio_track), 
					ztest_unit_test(test_set_analog_gain_to_min_audio_track), 
					ztest_unit_test(test_set_digital_gain_to_0_audio_track), 
					ztest_unit_test(test_set_digital_gain_to_max_audio_track), 
					ztest_unit_test(test_set_digital_gain_to_min_audio_track), 
					ztest_unit_test(test_set_analog_digital_gain_to_max_audio_track), 
					ztest_unit_test(test_set_illegal_gain_audio_track));
//					ztest_unit_test(test_play_16k_aac_audio_track));
	ztest_run_test_suite(audio_track_test);
}
