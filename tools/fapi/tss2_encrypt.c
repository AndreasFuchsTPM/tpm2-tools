/* SPDX-License-Identifier: BSD-3-Clause */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "tools/fapi/tss2_template.h"

/* needed by tpm2_util and tpm2_option functions */
bool output_enabled = false;

/* Context struct used to store passed commandline parameters */
static struct cxt {
    char const *keyPath;
    char const *policyPath;
    char const *plainText;
    char const *cipherText;
    bool        overwrite;
} ctx;

/* Parse commandline parameters */
static bool on_option(char key, char *value) {
    switch (key) {
    case 'f':
        ctx.overwrite = true;
        break;
    case 'p':
        ctx.policyPath = value;
        break;
    case 'c':
        ctx.cipherText = value;
        break;
    case 'k':
        ctx.keyPath = value;
        break;
    case 'P':
        ctx.plainText = value;
        break;
    }
    return true;
}

/* Define possible commandline parameters */
bool tss2_tool_onstart(tpm2_options **opts) {
    struct option topts[] = {
        {"keyPath",     required_argument, NULL, 'k'},
        {"policyPath",  required_argument, NULL, 'p'},
        {"plainText",   required_argument, NULL, 'P'},
        {"cipherText",  required_argument, NULL, 'c'},
        {"force",       no_argument      , NULL, 'f'},
    };
    return (*opts = tpm2_options_new ("f:p:c:k:P:", ARRAY_LEN(topts), topts,
                                      on_option, NULL, 0)) != NULL;
}

/* Execute specific tool */
int tss2_tool_onrun (FAPI_CONTEXT *fctx) {
    /* Check availability of required parameters */
    if (!ctx.keyPath) {
        fprintf (stderr, "No key path provided, use --keyPath=\n");
        return -1;
    }
    if (!ctx.plainText) {
        fprintf (stderr, "No text to encrypt provided, use --plainText=[file" \
            ", or '-' for standard input]\n");
        return -1;
    }
    if (!ctx.cipherText) {
        fprintf (stderr, "No output file provided, --cipherText=[file, or " \
            "'-' for standard output]\n");
        return -1;
    }

    /* Read plaintext file */
    uint8_t *plainText;
    size_t plainTextSize;
    TSS2_RC r = open_read_and_close (ctx.plainText, (void**)&plainText,
        &plainTextSize);
    if (r){
        LOG_PERR ("open_read_and_close plainText", r);
        return 1;
    }

    /* Execute FAPI command with passed arguments */
    uint8_t *cipherText;
    size_t cipherTextSize;
    r = Fapi_Encrypt (fctx, ctx.keyPath, plainText, plainTextSize,
        &cipherText, &cipherTextSize);
    if (r != TSS2_RC_SUCCESS) {
        LOG_PERR ("Fapi_Encrypt", r);
        free (plainText);
        return 1;
    }
    free (plainText);

    /* Write returned data to file(s) */
    r = open_write_and_close (ctx.cipherText, ctx.overwrite, cipherText,
        cipherTextSize);
    if (r) {
        LOG_PERR ("open_write_and_close cipherText", r);
        return 1;
    }

    Fapi_Free (cipherText);
    return 0;
}
