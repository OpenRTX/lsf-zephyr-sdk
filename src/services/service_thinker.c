/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT lsf_thinker_service

#include <zephyr/device.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lsf_thinker, CONFIG_LSF_LOG_LEVEL);

#include <lsf.h>
#include <lsf/service.h>
#include <lsf/services/thinker.h>

#include "ic_allocator.h"
extern IC_Allocator *g_pFreeer;

#define THINKER_SERVICE_ID DT_INST_REG_ADDR(0)

enum {
	THINKERSERVICE_PROPERTY_INPUT,
	THINKERSERVICE_PROPERTY_OUTPUT,
	THINKERSERVICE_PROPERTY_MODEL,
	THINKERSERVICE_PROPERTY_OUTPUT_COUNT,
};

static LsfPropertyConfig thinkerservice_property_defs[] = {
	{
		.property_id = THINKERSERVICE_PROPERTY_INPUT,
		.flag = LSF_PROPERTY_FLAG_SYNC | LSF_PROPERTY_FLAG_SET_ONLY,
		.type = LSF_PROPERTY_TYPE_PTR,
	},
	{
		.property_id = THINKERSERVICE_PROPERTY_OUTPUT,
		.flag = LSF_PROPERTY_FLAG_SYNC | LSF_PROPERTY_FLAG_SET_GET,
		.type = LSF_PROPERTY_TYPE_PTR,
	},
	{
		.property_id = THINKERSERVICE_PROPERTY_MODEL,
		.flag = LSF_PROPERTY_FLAG_SYNC | LSF_PROPERTY_FLAG_SET_ONLY,
		.type = LSF_PROPERTY_TYPE_PTR,
	},
	{
		.property_id = THINKERSERVICE_PROPERTY_OUTPUT_COUNT,
		.flag = LSF_PROPERTY_FLAG_SYNC | LSF_PROPERTY_FLAG_GET_ONLY,
		.type = LSF_PROPERTY_TYPE_UINT32,
	},
};

int lsf_thinker_set_model(const void *ptr, uint32_t size)
{
	LsfVariant var;
	int ret;

	LOG_DBG("Set model: addr: %p, size: %u", ptr, size);
	LOG_HEXDUMP_DBG(ptr, MIN(8, size), "Model data (first 8 bytes):");

	LsfVariant_init(&var);
	var.variant_type = type_pointer;
	var.u.pointer.pAddr = (void *)ptr;
	var.u.pointer.size = size;

	ret = lsf_set(THINKER_SERVICE_ID, THINKERSERVICE_PROPERTY_MODEL, &var);
	if (ret != 0) {
		LOG_ERR("Failed to set model: %d", ret);
		ret = -EIO;
	}

	LsfVariant_clear(&var);
	return ret;
}

int lsf_thinker_set_input(const void *ptr, uint32_t size)
{
	LsfVariant var;
	int ret;

	LOG_DBG("Set input: addr: %p, size: %u", ptr, size);
	LOG_HEXDUMP_DBG(ptr, MIN(8, size), "Input data (first 8 bytes):");

	LsfVariant_init(&var);
	var.variant_type = type_pointer;
	var.u.pointer.pAddr = (void *)ptr;
	var.u.pointer.size = size;

	ret = lsf_set(THINKER_SERVICE_ID, THINKERSERVICE_PROPERTY_INPUT, &var);
	if (ret != 0) {
		LOG_ERR("Failed to set input: %d", ret);
		ret = -EIO;
	}

	LsfVariant_clear(&var);
	return ret;
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

int lsf_thinker_get_output_by_index(uint32_t idx, void **ptr, uint32_t *size)
{
	LsfVariant var;
	int ret;

	LOG_DBG("Get output: idx: %u", idx);

	LsfVariant_init(&var);
	var.variant_type = type_uint;
	var.u.uiVal = idx;

	ret = lsf_set(THINKER_SERVICE_ID, THINKERSERVICE_PROPERTY_OUTPUT, &var);
	if (ret <= 0) {
		LOG_ERR("Failed to get output: %d", ret);
		ret = -EIO;
		goto ret;
	}

	LsfVariant ret_var;
	LsfVariant_init(&ret_var);
	ret_var.variant_type = type_pointer;

	size_t ret_var_size = LsfVariant_getMarshalSize(&ret_var);
	uint8_t *ret_var_buf = (uint8_t *)ret;

	ret = ((int32_t *)ret_var_buf)[0];
	if (ret != 0) {
		LOG_ERR("Failed to get output: %d", ret);
		ret = -EIO;
		goto ret_var;
	}

	LsfVariantClass_unmarshal(&ret_var, (uint8_t *)(ret_var_buf + sizeof(int32_t)),
				  ret_var_size, &ret_var_size);
	*ptr = ret_var.u.pointer.pAddr;
	*size = ret_var.u.pointer.size;

	LOG_DBG("Got output: ptr: %p, size: %u", *ptr, *size);
	LOG_HEXDUMP_DBG(*ptr, MIN(8, *size), "Output data (first 8 bytes):");

ret_var:
	IC_Sharemem_detach(ret_var_buf, sizeof(int32_t) + ret_var_size);
	IC_Allocator_free(g_pFreeer, ret_var_buf);
ret:
	LsfVariant_clear(&var);
	return ret;
}

int lsf_thinker_get_output_count(void)
{
	LsfVariant *var;
	uint32_t count;
	int ret;

	ret = lsf_get(THINKER_SERVICE_ID, THINKERSERVICE_PROPERTY_OUTPUT_COUNT, &var);
	if (ret < 0) {
		LOG_ERR("Failed to get output count: %d", ret);
		return -EIO;
	}

	count = var->u.uiVal;
	LsfVariant_unref(var);

	LOG_DBG("Got output count: %d", count);

	return count;
}

static int lsf_thinker_init(void)
{
	LOG_DBG("Initialize ThinkerService");

	lsf_import_properties(THINKER_SERVICE_ID, thinkerservice_property_defs,
			      ARRAY_SIZE(thinkerservice_property_defs));

	return 0;
}

LSF_SERVICE_DEFINE(lsf_thinker, lsf_thinker_init);
