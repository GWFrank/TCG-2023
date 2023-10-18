#!/usr/bin/env bash

set -euo pipefail

# Set the make command
MAKE_CMD='make'
CLEAN_ARG='clean'

# Set the directory containing the testcases
TESTCASES_DIR='./testcases'

# Compile the C++ code
${MAKE_CMD} ${CLEAN_ARG}
${MAKE_CMD}

# Get the name of the executable file
EXECUTABLE_FILE='./solve'

# Iterate over the testcases
for testcase in $(ls ${TESTCASES_DIR}); do

    # Print the results
    echo "Testcase: ${testcase}"

    # Run the executable file on the testcase
    '../verifier/verifier' ${EXECUTABLE_FILE} ${TESTCASES_DIR}/${testcase} | tail -n 1

    echo '-----------'

done
