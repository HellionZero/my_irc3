#!/bin/bash

set -eu

TEST_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SERVER_TEST="${TEST_DIR}/server_test.sh"
CLIENT_TEST="${TEST_DIR}/client_test.sh"
CHANNEL_TEST="${TEST_DIR}/channel_test.sh"
MEMORY_TEST="${TEST_DIR}/memory_test.sh"
SERVER_EXECUTABLE="${TEST_DIR}/../ircserv"

RUN_TESTS=1

cleanup()
{
	make -C "${TEST_DIR}/.." fclean >/dev/null 2>&1 || true
}

trap cleanup EXIT INT TERM

if [ "$#" -gt 0 ]; then
	if [ "$1" == "--no-tests" ]; then
		RUN_TESTS=0
	fi
fi

if [ $RUN_TESTS -eq 1 ]; then
	echo "Cleaning build..."
	make -C "${TEST_DIR}/.." fclean
	echo "Building server..."
	make -C "${TEST_DIR}/.."
	echo "Running server tests..."
	${SERVER_TEST}
	echo "Running client tests..."
	${CLIENT_TEST}
	echo "Running channel tests..."
	${CHANNEL_TEST}
	echo "Running memory tests..."
	${MEMORY_TEST}
	echo "Cleaning build artifacts..."
	make -C "${TEST_DIR}/.." fclean
fi

echo "All tests passed!"
