/*
 * Copyright (c) 2025 GARDENA GmbH
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include "reboot_bottom.h"
#include "posix_board_if.h"

void sys_arch_reboot(int type)
{
	fprintf(stderr, "DBGREBOOT: native_sim sys_arch_reboot(type=%d) enter\n", type);
	native_set_reboot_on_exit();
	fprintf(stderr, "DBGREBOOT: native_sim sys_arch_reboot(type=%d) reboot_on_exit set, calling posix_exit(0)\n", type);
	posix_exit(0);
	fprintf(stderr, "DBGREBOOT: native_sim sys_arch_reboot(type=%d) posix_exit returned!\n", type);
}
