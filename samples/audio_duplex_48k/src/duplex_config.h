/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

/*
 * Set to 1 to additionally capture the two DAC echo reference channels
 * (6 channels interleaved: 4 mic + echo L/R).
 *
 * NOTE: once ECHO is enabled, ONLY 16000 Hz is supported. This is a
 * hardware constraint: the DAC echo reference path cannot run at 48000 Hz,
 * so the DSP firmware rejects any 48000 Hz request that includes the ECHO
 * channels (AudioSystem_setParameters() / AudioRecord_ctor() fail with
 * BAD_VALUE). Keep SAMPLE_RATE at 16000 in the ECHO configuration below.
 */
#define DUPLEX_WITH_ECHO 0

#if DUPLEX_WITH_ECHO
#define SAMPLE_RATE       (16000)
#define RECORD_CHNS       (CHANNEL_IN_LEFT | CHANNEL_IN_RIGHT | \
			   CHANNEL_IN_AUX | CHANNEL_IN_ECHO)
#define RECORD_CHNS_CNT   (6)
#else
#define SAMPLE_RATE       (48000)
#define RECORD_CHNS       (CHANNEL_IN_LEFT | CHANNEL_IN_RIGHT | CHANNEL_IN_AUX)
#define RECORD_CHNS_CNT   (4)
#endif

#define DUPLEX_DURATION_MS (5000)
