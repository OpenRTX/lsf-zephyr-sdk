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

#include <lsf/controller.h>

static volatile bool inited = false;

void lsf_controller_init(void)
{
	if (inited) {
		return;
	}

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

	inited = true;
}

static int lsf_controller_init_internal(const struct device *dev)
{
	ARG_UNUSED(dev);

#if DT_HAS_CHOSEN(lsf_dsp_firmware)
	lsf_controller_init();
#endif /* DT_HAS_CHOSEN(lsf_dsp_firmware) */

	return 0;
}

DEVICE_DT_INST_DEFINE(0, lsf_controller_init_internal, NULL, NULL, NULL, APPLICATION,
		      CONFIG_APPLICATION_INIT_PRIORITY, NULL);
