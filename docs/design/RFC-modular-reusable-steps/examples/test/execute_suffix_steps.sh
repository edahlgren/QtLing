#!/bin/bash

BLUE='\033[0;36m'
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

function introduce_step {
    STEP_NUMBER=$1
    STEP_DESC=$2
    STEP_COMMAND=$3

    echo -e "\n${BLUE}Step ${STEP_NUMBER}${NC}: ${STEP_DESC}"

echo -e "... Running:\t$ ${STEP_COMMAND}"
    
}

function report_step {
    EXIT_CODE=$1
    ERROR_MSG=$2
    OUTPUT=$3

    if [ $EXIT_CODE -ne 0 ]; then
        echo -e "... Status:\t${RED}FAILURE${NC}"
        echo -e "... Error:\t"$ERROR_MSG
        exit 1
    else
        echo -e "... Status:\t${GREEN}COMPLETE${NC}"
        echo -e "... Output:\t"$OUTPUT
    fi
}

# Input file

DX1_FILE=$1

# Step 1

STEP1="./suffix_sort_words ${DX1_FILE}"

introduce_step 1 "Sorting words for suffix search" "${STEP1}"

SORTED_WORDS_FILE=$( $STEP1 2> error.log )
EXIT_CODE=$?
ERROR_MSG=$(<error.log)

report_step "${EXIT_CODE}" "${ERROR_MSG}" "${SORTED_WORDS_FILE}"

# Step 2

STEP2="./suffix_find_protostems ${SORTED_WORDS_FILE}"

introduce_step 2 "Finding protostems for suffix search" "${STEP2}"

PROTOSTEMS_FILE=$( $STEP2 2> error.log )
EXIT_CODE=$?
ERROR_MSG=$(<error.log)

report_step "${EXIT_CODE}" "${ERROR_MSG}" "${PROTOSTEMS_FILE}"

# Step 3

STEP3="./suffix_find_protostem_words ${SORTED_WORDS_FILE} ${PROTOSTEMS_FILE}"

introduce_step 3 "Associating protostems with words for suffix search" "${STEP3}"

PROTOSTEM_WORDS_FILE=$( $STEP3 2> error.log )
EXIT_CODE=$?
ERROR_MSG=$(<error.log)

report_step "${EXIT_CODE}" "${ERROR_MSG}" "${PROTOSTEM_WORDS_FILE}"
