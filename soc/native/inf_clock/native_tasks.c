/*
 * Copyright (c) 2017 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <stdio.h>
#include <pthread.h>

/**
 * @brief Run the set of special native tasks corresponding to the given level
 *
 * @param level One of _NATIVE_*_LEVEL as defined in soc.h
 */
void run_native_tasks(int level)
{
	extern void (*__native_PRE_BOOT_1_tasks_start[])(void);
	extern void (*__native_PRE_BOOT_2_tasks_start[])(void);
	extern void (*__native_PRE_BOOT_3_tasks_start[])(void);
	extern void (*__native_FIRST_SLEEP_tasks_start[])(void);
	extern void (*__native_ON_EXIT_tasks_start[])(void);
	extern void (*__native_tasks_end[])(void);

	static void (**native_pre_tasks[])(void) = {
		__native_PRE_BOOT_1_tasks_start,
		__native_PRE_BOOT_2_tasks_start,
		__native_PRE_BOOT_3_tasks_start,
		__native_FIRST_SLEEP_tasks_start,
		__native_ON_EXIT_tasks_start,
		__native_tasks_end
	};

	void (**fptr)(void);

	for (fptr = native_pre_tasks[level]; fptr < native_pre_tasks[level+1];
		fptr++) {
		if (*fptr) { /* LCOV_EXCL_BR_LINE */
			fprintf(stderr, "DBGREBOOT: run_native_tasks(level=%d) calling task %p (tid=%lu)\n",
				level, (void *)*fptr, (unsigned long)pthread_self());
			(*fptr)();
			fprintf(stderr, "DBGREBOOT: run_native_tasks(level=%d) task %p returned\n",
				level, (void *)*fptr);
		}
	}
}
