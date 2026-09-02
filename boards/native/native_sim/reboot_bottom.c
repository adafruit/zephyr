/*
 * Copyright (c) 2025 GARDENA GmbH
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <nsi_main.h>
#include <nsi_tasks.h>
#include <nsi_tracing.h>
#include <nsi_cmdline.h>
#include <nsi_host_trampolines.h>

static const char module[] = "native_sim_reboot";

static bool reboot_on_exit;

void native_set_reboot_on_exit(void)
{
	fprintf(stderr, "DBGREBOOT: native_set_reboot_on_exit: flag set\n");
	reboot_on_exit = true;
}

void maybe_reboot(void)
{
	char **argv;
	int argc;

	fprintf(stderr, "DBGREBOOT: maybe_reboot enter (reboot_on_exit=%d)\n", reboot_on_exit);

	if (!reboot_on_exit) {
		return;
	}

	reboot_on_exit = false; /* If we reenter it means we failed to reboot */

	nsi_get_cmd_line_args(&argc, &argv);

	fprintf(stderr, "DBGREBOOT: maybe_reboot calling nsi_host_setenv (setenv may malloc)\n");
	/* Let's set an environment variable which the native_sim hw_info driver may check */
	(void)nsi_host_setenv("NATIVE_SIM_RESET_CAUSE", "SOFTWARE", 1);
	fprintf(stderr, "DBGREBOOT: maybe_reboot setenv done\n");

	nsi_print_warning("%s: Restarting process.\n", module);

	fprintf(stderr, "DBGREBOOT: maybe_reboot calling execv(/proc/self/exe)\n");
	(void)execv("/proc/self/exe", argv);
	fprintf(stderr, "DBGREBOOT: maybe_reboot execv returned (errno=%d %s)\n", errno, strerror(errno));

	nsi_print_error_and_exit("%s: Failed to restart process, exiting (%s)\n", module,
				 strerror(errno));
}

NSI_TASK(maybe_reboot, ON_EXIT_POST, 999);
