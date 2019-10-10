/* SPDX-License-Identifier: BSD-3-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools/fapi/tss2_template.h"

/* needed by tpm2_util and tpm2_option functions */
bool output_enabled = false;

/* Context struct used to store passed command line parameters */
static struct cxt {
    char *searchPath;
    char *pathList;
} ctx;

/* Parse command line parameters */
static bool on_option(char key, char *value) {
    switch (key) {
    case 's':
        ctx.searchPath = value;
        break;
    case 'p':
        ctx.pathList = value ? value : "-";
        break;
    }
    return true;
}

/* Define possible command line parameters */
bool tss2_tool_onstart(tpm2_options **opts) {
    struct option topts[] = {
        {"searchPath" , optional_argument, NULL, 's'},
        {"pathList" , optional_argument, NULL, 'p'}
    };
    return (*opts = tpm2_options_new ("s:p:", ARRAY_LEN(topts), topts,
                                      on_option, NULL, 0)) != NULL;
}

/* Execute specific tool */
int tss2_tool_onrun (FAPI_CONTEXT *fctx) {
    (void) fctx;
    /* Execute FAPI command with passed arguments */
    char *pathlist = NULL;
    TSS2_RC r = Fapi_List(fctx,
        ctx.searchPath ? ctx.searchPath : "", &pathlist);
    if (r != TSS2_RC_SUCCESS) {
        LOG_PERR ("Fapi_List", r);
        return 1;
    }

    /* Write returned data to file(s) */
    r = open_write_and_close (ctx.pathList, true, pathlist,
        strlen(pathlist));
    if (r){
        LOG_PERR ("open_write_and_close pathlist", r);
        Fapi_Free (pathlist);
        return 1;
    }

    Fapi_Free (pathlist);
    return 0;
}
