/*
 * Copyright(c) 2023 LISTENAI
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __LSF_SERVICES_THINKER_H__
#define __LSF_SERVICES_THINKER_H__

#include <stdint.h>

int lsf_thinker_set_model(void *ptr, uint32_t size);

int lsf_thinker_set_input(void *ptr, uint32_t size);

int lsf_thinker_get_output(void **ptr, uint32_t *size);

#endif /* __LSF_SERVICES_THINKER_H__ */
