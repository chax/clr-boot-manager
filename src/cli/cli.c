/*
 * This file is part of clr-boot-manager.
 *
 * Copyright © 2016-2018 Intel Corporation
 *
 * clr-boot-manager is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */

#define _GNU_SOURCE

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"

static struct option default_opts[] = { { "path", required_argument, 0, 'p' },
                                        { "image", no_argument, 0, 'i' },
                                        { 0, 0, 0, 0 } };

bool cli_default_args_init(int *argc, char ***argv, char **root, bool *forced_image)
{
        int o_in = 0;
        int c;
        char *_root = NULL;

        /* We actually want to use getopt, so rewind one for getopt */;
        --(*argv);
        ++(*argc);

        if (!root) {
                return false;
        }

        /* Allow setting the root */
        while (true) {
                c = getopt_long(*argc, *argv, "ip:", default_opts, &o_in);
                if (c == -1) {
                        break;
                }
                switch (c) {
                case 0:
                case 'p':
                        if (optarg) {
                                if (_root) {
                                        free(_root);
                                        _root = NULL;
                                }
                                _root = strdup(optarg);
                        }
                        break;
                case 'i':
                        if (forced_image) {
                                *forced_image = true;
                        }
                        break;
                case '?':
                        goto bail;
                        break;
                default:
                        abort();
                }
        }
        *argc -= optind;

        if (_root) {
                *root = _root;
        }
        return true;
bail:
        if (_root) {
                free(_root);
        }
        return false;
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
