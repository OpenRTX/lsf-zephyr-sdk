/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT lsf_thinker_service

#include <zephyr/device.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lsf_thinker, CONFIG_LSF_LOG_LEVEL);

#include <lsf.h>
#include <lsf/services/thinker.h>

#define THINKER_SERVICE_ID DT_INST_REG_ADDR(0)

enum {
	THINKERSERVICE_PROPERTY_INPUT,
	THINKERSERVICE_PROPERTY_OUTPUT,
	THINKERSERVICE_PROPERTY_MODEL,
};

static LsfPropertyConfig thinkerservice_property_defs[] = {
	{
		.property_id = THINKERSERVICE_PROPERTY_INPUT,
		.flag = LSF_PROPERTY_FLAG_SYNC | LSF_PROPERTY_FLAG_SET_ONLY,
		.type = LSF_PROPERTY_TYPE_PTR,
	},
	{
		.property_id = THINKERSERVICE_PROPERTY_OUTPUT,
		.flag = LSF_PROPERTY_FLAG_SYNC | LSF_PROPERTY_FLAG_GET_ONLY,
		.type = LSF_PROPERTY_TYPE_PTR,
	},
	{
		.property_id = THINKERSERVICE_PROPERTY_MODEL,
		.flag = LSF_PROPERTY_FLAG_SYNC | LSF_PROPERTY_FLAG_SET_ONLY,
		.type = LSF_PROPERTY_TYPE_PTR,
	},
};

int lsf_thinker_set_model(void *ptr, uint32_t size)
{
	LsfVariant var;
	int ret;

	LOG_DBG("Set model: addr: %p, size: %u", ptr, size);
	LOG_HEXDUMP_DBG(ptr, MIN(8, size), "Model data (first 8 bytes):");

	LsfVariant_init(&var);
	var.variant_type = type_pointer;
	var.u.pointer.pAddr = ptr;
	var.u.pointer.size = size;

	ret = lsf_set(THINKER_SERVICE_ID, THINKERSERVICE_PROPERTY_MODEL, &var);
	if (ret != 0) {
		LOG_ERR("Failed to set model: %d", ret);
		return -EIO;
	}

	LsfVariant_clear(&var);

	return 0;
}

int lsf_thinker_set_input(void *ptr, uint32_t size)
{
	LsfVariant var;
	int ret;

	LOG_DBG("Set input: addr: %p, size: %u", ptr, size);
	LOG_HEXDUMP_DBG(ptr, MIN(8, size), "Input data (first 8 bytes):");

	LsfVariant_init(&var);
	var.variant_type = type_pointer;
	var.u.pointer.pAddr = ptr;
	var.u.pointer.size = size;

	ret = lsf_set(THINKER_SERVICE_ID, THINKERSERVICE_PROPERTY_INPUT, &var);
	if (ret != 0) {
		LOG_ERR("Failed to set input: %d", ret);
		return -EIO;
	}

	LsfVariant_clear(&var);

	return 0;
}

int lsf_thinker_get_output(void **ptr, uint32_t *size)
{
	LsfVariant *var;
	int ret;

	ret = lsf_get(THINKER_SERVICE_ID, THINKERSERVICE_PROPERTY_OUTPUT, &var);
	if (ret != 0) {
		LOG_ERR("Failed to get output: %d", ret);
		return -EIO;
	}

	*ptr = var->u.pointer.pAddr;
	*size = var->u.pointer.size;
	LsfVariant_unref(var);

	LOG_DBG("Got output: ptr: %p, size: %u", *ptr, *size);
	LOG_HEXDUMP_DBG(*ptr, MIN(8, *size), "Output data (first 8 bytes):");

	return 0;
}

static int lsf_thinker_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	LOG_DBG("Initialize ThinkerService");

	lsf_import_properties(THINKER_SERVICE_ID, thinkerservice_property_defs,
			      ARRAY_SIZE(thinkerservice_property_defs));

	return 0;
}

DEVICE_DT_INST_DEFINE(0, lsf_thinker_init, NULL, NULL, NULL, APPLICATION,
		      CONFIG_APPLICATION_INIT_PRIORITY, NULL);
