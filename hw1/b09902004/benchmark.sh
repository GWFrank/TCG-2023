#!/usr/bin/env bash
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
    time ${EXECUTABLE_FILE} <${TESTCASES_DIR}/${testcase} | head -n 1 | xargs -0 -d '\n' printf "%s steps\n"

    echo '-----------'

done
