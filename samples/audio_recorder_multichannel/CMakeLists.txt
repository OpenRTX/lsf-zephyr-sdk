# Copyright (c) 2023 LISTENAI
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(audio_recorder_multichannel)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/play_thread.c)
target_sources(app PRIVATE src/record_thread.c)
