# !/bin/sh
# Call this file from Eclipse scanner profile instead of the compiler directly. It will pick up the defines set in the make file
# which makes the indexing work almost flawlessly.
# The code highlighting with #ifdef sections will also be correct.
#
# Project properties:
#   C/C++ Build -> Discovery Options:
#     (CHECK) Automate discover...
#     (CHECK) Report path...
#     (UNCHK) Enable build...
#     (CHECK) Enable genrate scane build command
#     Compiler invocation command: sh
#     Compiler invocation arguments: scannerinfo.sh -E -P -v -dD ${plugin_state_location}/specs.c


export SCANNER_INFO_EXPORT_FILE="$1 $2 $3 $4 $5 $6 $7 $8"
make -s scanner-info2
