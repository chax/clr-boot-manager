/*
 * This file is part of clr-boot-manager.
 *
 * Copyright © 2016 Intel Corporation
 *
 * clr-boot-manager is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */

#define _GNU_SOURCE

#include <assert.h>
#include <check.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "bootman.h"
#include "files.h"
#include "nica/files.h"

#include "config.h"
#include "harness.h"

START_TEST(bootman_image_test_simple)
{
        autofree(BootManager) *m = NULL;

        PlaygroundKernel init_kernels[] = {
                { "4.6.0", "native", 180, true },
                { "4.4.4", "native", 160, false },
                { "4.4.0", "native", 140, false },
        };
        PlaygroundConfig start_conf = { NULL, init_kernels, ARRAY_SIZE(init_kernels) };

        m = prepare_playground(&start_conf);
        fail_if(!m, "Fatal: Cannot initialise playground");
        boot_manager_set_image_mode(m, true);

        fail_if(!boot_manager_update(m), "Failed to update in image mode");

        confirm_bootloader();
}
END_TEST

/**
 * Identical to bootman_image_test_simple, but in native mode
 */
START_TEST(bootman_native_test_simple)
{
        autofree(BootManager) *m = NULL;

        PlaygroundKernel init_kernels[] = {
                { "4.6.0", "native", 180, true },
                { "4.4.4", "native", 160, false },
                { "4.4.0", "native", 140, false },
        };
        PlaygroundConfig start_conf = { "4.4.4-160.native",
                                        init_kernels,
                                        ARRAY_SIZE(init_kernels) };

        m = prepare_playground(&start_conf);
        fail_if(!m, "Fatal: Cannot initialise playground");
        boot_manager_set_image_mode(m, false);
        fail_if(!set_kernel_booted(&init_kernels[1], true), "Failed to set kernel as booted");

        fail_if(!boot_manager_update(m), "Failed to update in native mode");

        confirm_bootloader();
}
END_TEST

static void internal_loader_test(bool image_mode)
{
        autofree(BootManager) *m = NULL;
        PlaygroundConfig start_conf = { 0 };

        m = prepare_playground(&start_conf);
        fail_if(!m, "Fatal: Cannot initialise playground");
        boot_manager_set_image_mode(m, image_mode);

        fail_if(!boot_manager_modify_bootloader(m, BOOTLOADER_OPERATION_INSTALL),
                "Failed to install bootloader");

        confirm_bootloader();
        fail_if(!confirm_bootloader_match(), "Installed bootloader is incorrect");

        fail_if(!push_bootloader_update(1), "Failed to bump source bootloader");
        fail_if(confirm_bootloader_match(), "Source shouldn't match target bootloader yet");

        fail_if(!boot_manager_modify_bootloader(m, BOOTLOADER_OPERATION_UPDATE | BOOTLOADER_OPERATION_NO_CHECK),
                "Failed to forcibly update bootloader");
        confirm_bootloader();
        fail_if(!confirm_bootloader_match(), "Bootloader didn't actually update");

        /* We're in sync */
        fail_if(boot_manager_needs_update(m), "Bootloader lied about needing an update");

        fail_if(!push_bootloader_update(2), "Failed to bump source bootloader");
        /* Pushed out of sync, should need update */
        fail_if(!boot_manager_needs_update(m), "Bootloader doesn't know it needs update");
        fail_if(!boot_manager_modify_bootloader(m, BOOTLOADER_OPERATION_UPDATE),
                "Failed to auto-update bootloader");
        fail_if(!confirm_bootloader_match(),
                "Auto-updated bootloader doesn't match source");
}

START_TEST(bootman_loader_test_update_image)
{
        internal_loader_test(true);
}
END_TEST

START_TEST(bootman_loader_test_update_native)
{
        internal_loader_test(false);
}
END_TEST

static Suite *core_suite(void)
{
        Suite *s = NULL;
        TCase *tc = NULL;

        s = suite_create("bootman_update");
        tc = tcase_create("bootman_update_functions");
        tcase_add_test(tc, bootman_image_test_simple);
        tcase_add_test(tc, bootman_native_test_simple);
        suite_add_tcase(s, tc);

        tc = tcase_create("bootman_loader_functions");
        tcase_add_test(tc, bootman_loader_test_update_image);
        tcase_add_test(tc, bootman_loader_test_update_native);
        suite_add_tcase(s, tc);

        return s;
}

int main(void)
{
        Suite *s;
        SRunner *sr;
        int fail;

        /* syncing can be problematic during test suite runs */
        cbm_set_sync_filesystems(false);

        s = core_suite();
        sr = srunner_create(s);
        srunner_run_all(sr, CK_VERBOSE);
        fail = srunner_ntests_failed(sr);
        srunner_free(sr);

        if (fail > 0) {
                return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
}

/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 8
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=8 tabstop=8 expandtab:
 * :indentSize=8:tabSize=8:noTabs=true:
 */
