/* SPDX-License-Identifier: BSD-3-Clause */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools/fapi/tss2_template.h"

/* needed by tpm2_util and tpm2_option functions */
bool output_enabled = false;

/* Context struct used to store passed commandline parameters */
static struct cxt {
    char     const *nvPath;
    char     const *nvTemplate;
    char           *authValue;
    uint32_t        size;
    char     const *policyPath;
} ctx;

/* Parse commandline parameters */
static bool on_option(char key, char *value) {
    switch (key) {
    case 'a':
        ctx.authValue = value ? strdup (value) : ask_for_password ();
        if (!ctx.authValue)
            return false; /* User entered two different passwords */
        break;
    case 'P':
        ctx.policyPath = value;
        break;
    case 'p':
        ctx.nvPath = value;
        break;
    case 's':
        if (!tpm2_util_string_to_uint32 (value, &ctx.size)) {
            fprintf (stderr, "%s cannot be converted to an integer or is" \
                " larger than 2**32 - 1\n", value);
            return false;
        }
        break;
    case 't':
        ctx.nvTemplate = value;
        break;
    }
    return true;
}

/* Define possible commandline parameters */
bool tss2_tool_onstart(tpm2_options **opts) {
    struct option topts[] = {
        {"path",        required_argument, NULL, 'p'},
        {"type",        required_argument, NULL, 't'},
        {"size"       , required_argument, NULL, 's'},
        {"policyPath", optional_argument, NULL, 'P'},
        {"authValue",    optional_argument, NULL, 'a'},
    };
    return (*opts = tpm2_options_new ("P:a:p:s:t:", ARRAY_LEN(topts), topts,
                                      on_option, NULL, 0)) != NULL;
}

/* Execute specific tool */
int tss2_tool_onrun (FAPI_CONTEXT *fctx) {
    /* Check availability of required parameters */
    if (!ctx.authValue){
        ctx.authValue = "";
    }
    if (!ctx.policyPath) {
        ctx.policyPath = "";
    }
    if (!ctx.nvPath) {
        fprintf (stderr, "No NV path provided, use --path=\n");
        return -1;
    }

    if (!ctx.nvTemplate) {
        fprintf (stderr, "No type provided, use --type=[file or" \
            " '-' for standard input]\n");
        return -1;
    }

    /* Execute FAPI command with passed arguments */
    TSS2_RC r = Fapi_CreateNv(fctx, ctx.nvPath, ctx.nvTemplate,
        ctx.size, ctx.policyPath, ctx.authValue);

    if (r != TSS2_RC_SUCCESS)
        LOG_PERR ("Fapi_CreateNv", r);
    return r;
}
