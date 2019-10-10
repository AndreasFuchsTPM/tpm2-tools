/* SPDX-License-Identifier: BSD-3-Clause */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "tools/fapi/tss2_template.h"

/* needed by tpm2_util and tpm2_option functions */
bool output_enabled = false;

/* Context struct used to store passed command line parameters */
static struct cxt {
    char const *nvPath;
    char const *policyPath;
} ctx;

/* Parse command line parameters */
static bool on_option(char key, char *value) {
    switch (key) {
    case 'n':
        ctx.nvPath = value;
        break;
    case 'l':
        ctx.policyPath = value;
        break;
    }
    return true;
}

/* Define possible command line parameters */
bool tss2_tool_onstart(tpm2_options **opts) {
    struct option topts[] = {
        {"nvPath"  , required_argument, NULL, 'n'},
        {"policyPath"  , required_argument, NULL, 'l'}
    };
    return (*opts = tpm2_options_new ("n:l:", ARRAY_LEN(topts), topts,
                                      on_option, NULL, 0)) != NULL;
}

/* Execute specific tool */
int tss2_tool_onrun (FAPI_CONTEXT *fctx) {
    /* Check availability of required parameters */
    if (!ctx.nvPath) {
        fprintf (stderr, "No NV path provided, use --nvPath=\n");
        return -1;
    }
    if (!ctx.policyPath) {
        fprintf (stderr, "No policy path provided, use --policyPath=\n");
        return -1;
    }

    /* Execute FAPI command with passed arguments */
    TSS2_RC r = Fapi_WriteAuthorizeNv(fctx, ctx.nvPath, ctx.policyPath);
    if (r != TSS2_RC_SUCCESS){
        LOG_PERR ("Fapi_WriteAuthorizeNv", r);
        return 1;
    }

    return 0;
}
