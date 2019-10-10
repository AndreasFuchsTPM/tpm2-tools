/* SPDX-License-Identifier: BSD-3-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools/fapi/tss2_template.h"

/* needed by tpm2_util and tpm2_option functions */
bool output_enabled = false;

/* Context struct used to store passed commandline parameters */
static struct cxt {
    char *entityPath;
    char *authValue;
} ctx;

/* Parse commandline parameters */
static bool on_option(char key, char *value) {
    switch (key) {
    case 'a':
        ctx.authValue = value ? strdup (value) : ask_for_password ();
        if (!ctx.authValue)
            return false; /* User entered two different passwords */
        break;
    case 'e':
        ctx.entityPath = value;
        break;
    }
    return true;
}

/* Define possible commandline parameters */
bool tss2_tool_onstart(tpm2_options **opts) {
    struct option topts[] = {
        {"authValue", optional_argument, NULL, 'a'},
        {"entityPath", required_argument, NULL, 'e'}
    };
    return (*opts = tpm2_options_new ("a:e:", ARRAY_LEN(topts), topts,
                                      on_option, NULL, 0)) != NULL;
}

/* Execute specific tool */
int tss2_tool_onrun (FAPI_CONTEXT *fctx) {
    /* Check availability of required parameters */
    if (!ctx.entityPath) {
        fprintf (stderr, "No entity path provided, use --entityPath=\n");
        if(ctx.authValue){
            free (ctx.authValue);
        }
        return -1;
    }

    /* Execute FAPI command with passed arguments */
    TSS2_RC r = Fapi_ChangeAuth(fctx, ctx.entityPath, ctx.authValue);
    if (r != TSS2_RC_SUCCESS) {
        LOG_PERR ("Fapi_ChangeAuth", r);
        return 1;
    }

    if(ctx.authValue){
        free (ctx.authValue);
    }

    return 0;
}
