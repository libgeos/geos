#!/bin/sh

cd `dirname $0`
astyle/astyle \
  --style=stroustrup \
  --unpad-paren \
  --pad-header \
  --pad-comma \
  --indent=spaces=4 \
  --align-pointer=type \
  --max-code-length=120 \
  --lineend=linux \
  @$
