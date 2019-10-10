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

NV_PATH=/nv/Owner/myNVcounter
NV_COUNTER_READ_FILE=$TEMP_DIR/nv_counter_read_data.file
PCR_POLICY_DATA=$TEMP_DIR/pol_pcr16_0.json
POLICY_PCR=policy/pcr-policy

tss2_provision

expect <<EOF
# Try 2 different passwords
spawn tss2_createnv --path $NV_PATH --type "counter, noDa" --size 0 --authValue
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

tss2_import --path $POLICY_PCR --importData $PCR_POLICY_DATA

tss2_createnv --path $NV_PATH --policyPath=$POLICY_PCR --type "counter, noDa" \
    --size 0

tss2_nvincrement --nvPath $NV_PATH

tss2_nvread --nvPath $NV_PATH --data $NV_COUNTER_READ_FILE --force

tss2_nvincrement --nvPath $NV_PATH

tss2_nvread --nvPath $NV_PATH --data $NV_COUNTER_READ_FILE --force

expect <<EOF
# Try with missing nvPath
spawn tss2_nvincrement
set ret [wait]
if {[lindex \$ret 2] || [lindex \$ret 3] != 1} {
    Command has not failed as expected\n"
    exit 1
}
EOF

exit 0