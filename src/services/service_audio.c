/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT lsf_audio_service

#include <zephyr/device.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lsf_audio, CONFIG_LSF_LOG_LEVEL);

#include <AudioSystem.h>

static int lsf_audio_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	LOG_DBG("Initialize AudioService");

	AudioSystem_initialize();

	return 0;
}

DEVICE_DT_INST_DEFINE(0, lsf_audio_init, NULL, NULL, NULL, APPLICATION,
		      CONFIG_APPLICATION_INIT_PRIORITY, NULL);
