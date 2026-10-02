#!/bin/sh
. ../common.sh

PROGRAM_LENGTH="$(./c_app -l)"

test_length "${PROGRAM_LENGTH}"
