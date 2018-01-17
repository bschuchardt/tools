#!/bin/sh
. ~/devel/testing/bin/64bit.sh
jhat -J-Xmx3000m "$@"
