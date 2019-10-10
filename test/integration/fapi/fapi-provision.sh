#!/bin/bash

set -e
source helpers.sh

start_up

setup_fapi

function cleanup {
    # Since clean up should already been done during normal run of the test, a
    # failure is expected here. Therefore, we need to pass a successful
    # execution in any case
    tss2_delete --path / || true
    shut_down
}

trap cleanup EXIT

PW=abc

tss2_provision --authValueSh $PW

expect <<EOF
# Change back to empty password
spawn tss2_changeauth --entityPath=/HS
expect "Authorize object: "
send "$PW\r"
expect "Authorize object: "
send "$PW\r"
set ret [wait]
if {[lindex \$ret 2] || [lindex \$ret 3]} {
    send_user "Command failed\n"
    exit 1
}
EOF

PROFILE_NAME=$( tss2_list --searchPath= --pathList=- | cut -d "/" -f2 )

tss2_delete --path /

expect <<EOF
# Test if still objects in path
spawn tss2_list --path
set ret [wait]
if {[lindex \$ret 2] || [lindex \$ret 3] != 1} {
    send_user "Still objects in path\n"
    exit 1
}
EOF

if [ -s $PROFILE_NAME ];then
    echo "Directory still existing"
    exit 99
fi

exit 0