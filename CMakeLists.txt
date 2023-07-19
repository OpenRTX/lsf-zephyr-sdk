# Copyright (c) 2023 LISTENAI
# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_include_directories(include)

zephyr_library_sources_ifdef(CONFIG_LSF_DSP src/dsp.c)

if(CONFIG_LSF_SERVICE_CONTROLLER)
    zephyr_library_sources(src/lsf.c)
    zephyr_library_link_libraries(lsf_client)
    add_subdirectory(src/services)
endif() # CONFIG_LSF_SERVICE_CONTROLLER
