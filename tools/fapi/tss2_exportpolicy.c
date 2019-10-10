/* SPDX-License-Identifier: BSD-3-Clause */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "tools/fapi/tss2_template.h"

/* needed by tpm2_util and tpm2_option functions */
bool output_enabled = false;

/* Context struct used to store passed command line parameters */
static struct cxt {
    char *path;
    char *buffer;
    bool overwrite;
} ctx;

/* Parse command line parameters */
static bool on_option(char key, char *value) {
    switch (key) {
    case 'f':
        ctx.overwrite = true;
        break;
    case 'j':
        ctx.buffer = value;
        break;
    case 'p':
        ctx.path = value;
        break;
    }
    return true;
}

/* Define possible command line parameters */
bool tss2_tool_onstart(tpm2_options **opts) {
    struct option topts[] = {
        {"force",       no_argument      , NULL, 'f'},
        {"path",        required_argument, NULL, 'p'},
        {"jsonPolicy",  required_argument, NULL, 'j'},

    };
    return (*opts = tpm2_options_new ("f:j:p:", ARRAY_LEN(topts), topts,
                                      on_option, NULL, 0)) != NULL;
}

/* Execute specific tool */
int tss2_tool_onrun (FAPI_CONTEXT *fctx) {
    /* Check availability of required parameters */
    if (!ctx.path) {
        fprintf (stderr, "path parameter is missing, pass --path=\n");
        return -1;
    }
    if (!ctx.buffer) {
        fprintf (stderr, "output parameter is missing, pass --jsonPolicy=\n");
        return -1;
    }

    /* Execute FAPI command with passed arguments */
    char *buffer;
    TSS2_RC r = Fapi_ExportPolicy (fctx, ctx.path, &buffer);
    if (r != TSS2_RC_SUCCESS) {
        LOG_PERR ("Fapi_PolicyExport", r);
        return 1;
    }

    /* Write returned data to file(s) */
    r = open_write_and_close (ctx.buffer, ctx.overwrite, buffer, 0);
    if (r){
        LOG_PERR ("open_write_and_close buffer", r);
        return 1;
    }

    Fapi_Free (buffer);
    return 0;
}
