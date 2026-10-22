/* SPDX-License-Identifier: Apache-2.0 */

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <zephyr/settings/settings.h>
#if defined(CONFIG_SETTINGS_NVS)
#include <zephyr/kvss/nvs.h>
#else
#include <zephyr/kvss/zms.h>
#endif
#include <zephyr/storage/flash_map.h>
#include <zephyr/ztest.h>

#define AREA_ID PARTITION_ID(storage_partition)
#define EMPTY_GEOMETRY (DT_REG_SIZE(DT_NODELABEL(storage_partition)) == 0x800)

static const uint32_t saved_value = 0x12345678;
static unsigned int loaded;
static unsigned int mount_calls;

/* Detect attempts to use undefined geometry, even if the mount later rejects
 * the accidental sector size with the same errno as the correct early guard.
 * Valid partitions still exercise the real storage backend.
 */
#if defined(CONFIG_SETTINGS_NVS)
int __real_nvs_mount(struct nvs_fs *fs);

int __wrap_nvs_mount(struct nvs_fs *fs)
{
	mount_calls++;
	return EMPTY_GEOMETRY ? -EIO : __real_nvs_mount(fs);
}
#else
int __real_zms_mount(struct zms_fs *fs);

int __wrap_zms_mount(struct zms_fs *fs)
{
	mount_calls++;
	return EMPTY_GEOMETRY ? -EIO : __real_zms_mount(fs);
}
#endif

static int load_value(const char *key, size_t len, settings_read_cb read_cb,
		      void *cb_arg, void *param)
{
	uint32_t value;

	ARG_UNUSED(param);
	zassert_equal(strcmp(key, "value"), 0);
	zassert_equal(len, sizeof(value));
	zassert_equal(read_cb(cb_arg, &value, sizeof(value)), sizeof(value));
	zassert_equal(value, saved_value);
	loaded++;
	return 0;
}

ZTEST(settings_geometry, test_flash_geometry)
{
	struct flash_sector sector = {.fs_off = 123, .fs_size = 456};
	uint32_t count = 1;
	int rc = flash_area_get_sectors(AREA_ID, &count, &sector);

	if (EMPTY_GEOMETRY) {
		zassert_ok(rc);
		zassert_equal(count, 0);
		zassert_equal(sector.fs_off, 123);
		zassert_equal(sector.fs_size, 456);
	} else {
		/* A one-entry buffer still returns the first sector on -ENOMEM. */
		zassert_equal(rc, -ENOMEM);
		zassert_equal(count, 1);
		zassert_equal(sector.fs_off, 0);
		zassert_equal(sector.fs_size, 4096);
	}
}

ZTEST(settings_geometry, test_backend_init)
{
	int rc = settings_subsys_init();

	if (EMPTY_GEOMETRY) {
		zassert_equal(rc, -EINVAL, "No settings erase sector must fail explicitly");
		zassert_equal(mount_calls, 0, "Invalid geometry must not reach a mount");
	} else {
		zassert_ok(rc);
		zassert_equal(mount_calls, 1);
		zassert_ok(settings_save_one("geometry/value", &saved_value, sizeof(saved_value)));
		zassert_ok(settings_load_subtree_direct("geometry", load_value, NULL));
		zassert_equal(loaded, 1);
	}
}

ZTEST_SUITE(settings_geometry, NULL, NULL, NULL, NULL, NULL);
