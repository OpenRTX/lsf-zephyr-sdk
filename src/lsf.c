/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT lsf_service_controller

#include <zephyr/device.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lsf, CONFIG_LSF_LOG_LEVEL);

#include <lsf.h>
#include <ic_proxy.h>
#include <ic_fence.h>

static int service_controller_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	LOG_DBG("Initializing LSF service controller");

	/* Initialize LSF */
	lsf_init();
	lsf_connect();

	/* Wait for ready signal */
	ICFenceHandle fence = IC_Proxy_getRemoteFence(0);

	ICFence_syncWithRemote(fence);
	LOG_DBG("DSP synced");

	uint32_t val;
	ICFence_wait(fence, &val);
	LOG_DBG("DSP ready");

	return 0;
}

DEVICE_DT_INST_DEFINE(0, service_controller_init, NULL, NULL, NULL, APPLICATION,
		      CONFIG_APPLICATION_INIT_PRIORITY, NULL);
