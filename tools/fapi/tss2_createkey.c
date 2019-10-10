/* SPDX-License-Identifier: BSD-3-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools/fapi/tss2_template.h"

/* needed by tpm2_util and tpm2_option functions */
bool output_enabled = false;

/* Context struct used to store passed commandline parameters */
static struct cxt {
    char const *keyPath;
    char const *keyType;
    char const *policyPath;
    char       *authValue;
} ctx;

/* Parse commandline parameters */
static bool on_option(char key, char *value) {
    switch (key) {
    case 'a':
        ctx.authValue = value ? strdup (value) : ask_for_password ();
        if (!ctx.authValue)
            return false; /* User entered two different passwords */
        break;
    case 'p':
        ctx.keyPath = value;
        break;
    case 'P':
        ctx.policyPath = value;
        break;
    case 't':
        ctx.keyType = value;
        break;
    }
    return true;
}

/* Define possible commandline parameters */
bool tss2_tool_onstart(tpm2_options **opts) {
    struct option topts[] = {
        {"path"       , required_argument, NULL, 'p'},
        {"type"       , required_argument, NULL, 't'},
        {"policyPath", required_argument, NULL, 'P'},
        {"authValue"   , optional_argument, NULL, 'a'},
    };
    return (*opts = tpm2_options_new ("a:p:P:t:", ARRAY_LEN(topts), topts,
                                      on_option, NULL, 0)) != NULL;
}

/* Execute specific tool */
int tss2_tool_onrun (FAPI_CONTEXT *fctx) {
    /* Check availability of required parameters */
    if (!ctx.keyPath) {
        fprintf (stderr, "key path missing, use --path\n");
        free (ctx.authValue);
        return -1;
    }
    if (!ctx.keyType) {
        fprintf (stderr, "key type missing, use --type\n");
        free (ctx.authValue);
        return -1;
    }

    /* Execute FAPI command with passed arguments */
    TSS2_RC r = Fapi_CreateKey (fctx, ctx.keyPath, ctx.keyType, ctx.policyPath,
        ctx.authValue);
    if (r != TSS2_RC_SUCCESS){
        free (ctx.authValue);
        LOG_PERR ("Fapi_CreateKey", r);
        return r;
    }

    free (ctx.authValue);
    return r;
}
