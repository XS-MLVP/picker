#!/bin/sh

if [ "${0##*/}" = "sleep" ]; then
    exit 0
fi

status="${PICKER_EXAMPLE_STATUS:-0}"
echo "PICKER_TEMPLATE_EXAMPLE_SHIM command=${0##*/} status=${status}" >&2
exit "${status}"
