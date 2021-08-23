#!/usr/bin/env bash

set -e

CMAKE=${CMAKE:-cmake}
CTEST=${CTEST:-ctest}
VALGRIND=${VALGRIND:-0}

if [[ "${TIMEOUT}" == "" ]]; then
    TIMEOUT_ARG=""
else
    TIMEOUT_ARG=" --timeout ${TIMEOUT}"
fi

if [ -d _build_ci ] ; then
    rm -rf _build_ci || exit 1
fi
mkdir _build_ci || exit 1

cd _build_ci || exit 1

${CMAKE} $@ ..
echo "======================================"
echo "                Build"
echo "======================================"
${CMAKE} --build . || exit 1
#${CMAKE} --build . --target examples || exit 1

echo "======================================"
echo "         Running unit tests"
echo "======================================"
echo

if [ "${VALGRIND}" == "1" ]; then
    if ${CTEST} -T memcheck -V${TIMEOUT_ARG} ; then
        echo "Test run under valgrind has finished successfully"
        cd ..
    else
        echo "Test run under valgrind failed" >&2
        exit 1
    fi
else
    if ${CTEST} -V${TIMEOUT_ARG} ; then
        echo "Test run has finished successfully"
        cd ..
    else
        echo "Test run failed" >&2
        exit 1
    fi
fi
