#!/bin/sh
. ../common.sh

PROGRAM_LENGTH="$(./cpp_app -l)"

test_length "${PROGRAM_LENGTH}"
