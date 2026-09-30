#!/bin/bash

set -u

PORT=${IRC_TEST_PORT:-16667}
PASSWORD="test-password"
SERVER_PID=""
LOG_FILE="$(mktemp)"
TEST_DIR="$(mktemp -d)"
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
PROJECT_ROOT="$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)"
SERVER_BINARY="$PROJECT_ROOT/ircserv"

pass()
{
	printf "\033[32m[PASS]\033[0m %s\n" "$1"
}

fail()
{
	printf "\033[31m[FAIL]\033[0m %s\n" "$1" >&2
	exit 1
}

cleanup()
{
	if [ -n "$SERVER_PID" ] && kill -0 "$SERVER_PID" 2>/dev/null; then
		kill "$SERVER_PID" 2>/dev/null
		wait "$SERVER_PID" 2>/dev/null
	fi
	rm -f "$LOG_FILE"
	rm -rf "$TEST_DIR"
}

trap cleanup EXIT INT TERM

if [ ! -x "$SERVER_BINARY" ]; then
	fail "servidor não encontrado ou não é executável"
fi
if ! command -v nc >/dev/null 2>&1; then
	fail "nc não encontrado; instale netcat para executar a suite"
fi
if ! command -v timeout >/dev/null 2>&1; then
	fail "timeout não encontrado; instale coreutils para executar a suite"
fi

"$SERVER_BINARY" "$PORT" "$PASSWORD" >"$LOG_FILE" 2>&1 &
SERVER_PID=$!

ready=0
for attempt in 1 2 3 4 5 6 7 8 9 10; do
	if ! kill -0 "$SERVER_PID" 2>/dev/null; then
		break
	fi
	if nc -z -w 1 127.0.0.1 "$PORT" >/dev/null 2>&1
	then
		ready=1
		break
	fi
done

if [ "$ready" -ne 1 ]; then
	cat "$LOG_FILE" >&2
	fail "servidor não aceitou conexões"
fi
pass "servidor iniciou e aceitou uma conexão"

receiver_file="$TEST_DIR/receiver"
timeout 2 nc 127.0.0.1 "$PORT" >"$receiver_file" 2>/dev/null &
receiver_pid=$!
sleep 0.1
printf 'raw message\r\n' | nc -q 1 -w 1 127.0.0.1 "$PORT" >/dev/null
wait "$receiver_pid" 2>/dev/null || true
if ! grep -Fq 'raw message' "$receiver_file"; then
	fail "mensagem não foi encaminhada"
fi

nc -w 1 127.0.0.1 "$PORT" </dev/null >/dev/null
printf 'after disconnect\r\n' | nc -q 1 -w 1 127.0.0.1 "$PORT" >/dev/null

receiver_pids=""
for index in 1 2 3 4 5; do
	timeout 2 nc 127.0.0.1 "$PORT" >"$TEST_DIR/receiver_$index" 2>/dev/null &
	receiver_pids="$receiver_pids $!"
done
sleep 0.1
printf 'stability check\r\n' | nc -q 1 -w 1 127.0.0.1 "$PORT" >/dev/null
for receiver_pid in $receiver_pids; do
	wait "$receiver_pid" 2>/dev/null || true
done
for index in 1 2 3 4 5; do
	if ! grep -Fq 'stability check' "$TEST_DIR/receiver_$index"; then
		fail "broadcast para múltiplos clientes falhou"
	fi
done
pass "servidor suportou conexões, broadcast e desconexões"

if ! kill -0 "$SERVER_PID" 2>/dev/null; then
	cat "$LOG_FILE" >&2
	fail "servidor encerrou durante a operação"
fi
pass "servidor permaneceu ativo após a operação"