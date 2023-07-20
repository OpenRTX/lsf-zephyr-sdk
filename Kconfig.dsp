# Copyright (c) 2023 LISTENAI
# SPDX-License-Identifier: Apache-2.0

menu "DSP Manipulation"

config LSF_DSP
	bool "DSP Manipulation"
	default y
	select FLASH
	select FLASH_MAP

if LSF_DSP

config LSF_DSP_INIT_PRIORITY
	int "Init priority"
	default 80

DT_CHOSEN_LSF_DSP_FIRMWARE := lsf,dsp-firmware

config LSF_DSP_LOAD_ON_BOOT
	bool "Load DSP firmware on boot"
	default y
	depends on $(dt_chosen_enabled,$(DT_CHOSEN_LSF_DSP_FIRMWARE))

endif # LSF_DSP

endmenu
