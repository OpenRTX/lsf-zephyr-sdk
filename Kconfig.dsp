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

endif # LSF_DSP

endmenu
