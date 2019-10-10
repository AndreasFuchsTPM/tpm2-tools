#!/bin/bash

set -e
source helpers.sh

start_up

setup_fapi

function cleanup {
    tss2_delete --path /
    shut_down
}

trap cleanup EXIT

PW=abc
KEY_PATH=HS/SRK/myRSASign
DIGEST_FILE=$TEMP_DIR/digest.file
SIGNATURE_FILE=$TEMP_DIR/signature.file
PUBLIC_KEY_FILE=$TEMP_DIR/public_key.file
IMPORTED_KEY_NAME=importedPubKey

tss2_provision
echo 0123456789012345678 > $DIGEST_FILE
tss2_createkey --path $KEY_PATH --type "noDa, sign"
tss2_sign --keyPath $KEY_PATH --padding "RSA_PSS" --digest $DIGEST_FILE \
    --signature $SIGNATURE_FILE --publicKey $PUBLIC_KEY_FILE

expect <<EOF
# Try 2 different passwords
spawn tss2_changeauth --entityPath $KEY_PATH --authValue
expect "Password: "
send "1\r"
expect "Retype password: "
send "2\r"
expect {
    "Passwords do not match" {
            } eof {
                send_user "Expected password mismatch, but got nothing, or
                rather EOF\n"
                exit 1
            }
        }
        set ret [wait]
        if {[lindex \$ret 2] || [lindex \$ret 3] != 1} {
            send_user "Using -P with different passwords on the prompt has not failed\n"
            exit 1
        }
EOF

tss2_changeauth --entityPath $KEY_PATH --authValue=$PW

expect <<EOF
# Check if system asks for auth value
spawn tss2_sign --keyPath $KEY_PATH --padding "RSA_PSS" --digest $DIGEST_FILE \
    --signature $SIGNATURE_FILE --publicKey $PUBLIC_KEY_FILE --force
expect {
    "Authorize object: " {
    } eof {
        send_user "The system has not asked for password\n"
        exit 1
    }
    }
    send "$PW\r"
    set ret [wait]
    if {[lindex \$ret 2] || [lindex \$ret 3]} {
        send_user "Passing password has failed\n"
        exit 1
    }
EOF

exit 0