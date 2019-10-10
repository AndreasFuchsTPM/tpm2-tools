/* SPDX-License-Identifier: BSD-3-Clause */

#include <stdio.h>
#include "tools/fapi/tss2_template.h"

/* needed by tpm2_util and tpm2_option functions */
bool output_enabled = false;

/* Context struct used to store passed commandline parameters */
static struct cxt {
    char *authValueEh;
    char *authValueSh;
    char *authValueLockout;
} ctx;

/* Parse commandline parameters */
static bool on_option(char key, char *value) {
    switch (key) {
    case 'e':
        ctx.authValueEh = value;
        break;
    case 's':
        ctx.authValueSh = value;
        break;
    case 'l':
        ctx.authValueLockout = value;
        break;
    }
    return true;
}

/* Define possible commandline parameters */
bool tss2_tool_onstart(tpm2_options **opts) {
    struct option topts[] = {
        {"authValueEh",         required_argument, NULL, 'e'},
        {"authValueSh",         required_argument, NULL, 's'},
        {"authValueLockout",    required_argument, NULL, 'l'},
    };
    return (*opts = tpm2_options_new ("e:s:l",
        ARRAY_LEN(topts), topts, on_option, NULL, 0)) != NULL;
}

/* Execute specific tool */
int tss2_tool_onrun (FAPI_CONTEXT *fctx) {

    /* Execute FAPI command with passed arguments */
    TSS2_RC r = Fapi_Provision (fctx, ctx.authValueEh, ctx.authValueSh,
        ctx.authValueLockout);
    if (r != TSS2_RC_SUCCESS){
        LOG_PERR ("Fapi_Provision", r);
        return 1;
    }

    return 0;
}
