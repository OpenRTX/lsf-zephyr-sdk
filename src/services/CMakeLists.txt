# Copyright (c) 2023 LISTENAI
# SPDX-License-Identifier: Apache-2.0

zephyr_library_sources_ifdef(CONFIG_LSF_AUDIO_SERVICE service_audio.c)
zephyr_library_link_libraries_ifdef(CONFIG_LSF_AUDIO_SERVICE af)

# 由于历史原因，这些头文件需要被暴露在外
if(CONFIG_LSF_AUDIO_SERVICE)
zephyr_include_directories(${ZEPHYR_LSF_MODULE_DIR}/lsf_client/include)
zephyr_include_directories(${ZEPHYR_AF_MODULE_DIR})
endif()

zephyr_library_sources_ifdef(CONFIG_LSF_THINKER_SERVICE service_thinker.c)
zephyr_library_sources_ifdef(CONFIG_LSF_WASM_SERVICE service_wasm.c)
