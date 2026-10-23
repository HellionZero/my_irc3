#!/bin/bash

set -eu

PORT=${IRC_MEMORY_TEST_PORT:-16669}
PASSWORD=${IRC_TEST_PASSWORD:-test-password}
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
PROJECT_ROOT="$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)"
SERVER_BINARY="$PROJECT_ROOT/ircserv"
SERVER_PID=""
LOG_FILE="$(mktemp)"
VALGRIND_LOG="$(mktemp)"

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
		kill -INT "$SERVER_PID" 2>/dev/null || true
		wait "$SERVER_PID" 2>/dev/null || true
	fi
	rm -f "$LOG_FILE" "$VALGRIND_LOG"
}

trap cleanup EXIT INT TERM

if ! command -v valgrind >/dev/null 2>&1; then
	printf "[SKIP] valgrind não encontrado; teste de memória não executado\n"
	exit 0
fi
if ! command -v python3 >/dev/null 2>&1; then
	fail "python3 não encontrado; necessário para o teste de memória\n" >&2
	exit 1
fi
if [ ! -x "$SERVER_BINARY" ]; then
	fail "servidor não encontrado ou não é executável\n" >&2
	exit 1
fi

valgrind \
	--leak-check=full \
	--show-leak-kinds=all \
	--track-origins=yes \
	--errors-for-leak-kinds=definite,indirect \
	--error-exitcode=99 \
	--log-file="$VALGRIND_LOG" \
	"$SERVER_BINARY" "$PORT" "$PASSWORD" >"$LOG_FILE" 2>&1 &
SERVER_PID=$!

python3 - "$PORT" <<'PY'
import socket
import sys
import time

port = int(sys.argv[1])
for _ in range(30):
    sock = socket.socket()
    sock.settimeout(1)
    try:
        sock.connect(("127.0.0.1", port))
        sock.close()
        break
    except OSError:
        sock.close()
        time.sleep(0.1)
else:
    raise SystemExit("servidor instrumentado não aceitou conexões")
PY

python3 - "$PORT" <<'PY'
import socket
import sys

port = int(sys.argv[1])

def connect():
    sock = socket.socket()
    sock.settimeout(2)
    sock.connect(("127.0.0.1", port))
    return sock

def receive(sock, expected):
    data = b""
    while expected not in data:
        chunk = sock.recv(4096)
        if not chunk:
            raise AssertionError("conexão fechada antes da resposta")
        data += chunk

sock = connect()
receive(sock, b"Connected. MVP mode.\r\n")
sock.sendall(b"memory")
sock.sendall(b" test\r\n")
receive(sock, b"memory test\r\n")
sock.close()

clients = [connect() for _ in range(12)]
for client in clients:
    receive(client, b"Connected. MVP mode.\r\n")
for index, client in enumerate(clients):
    client.sendall(("memory-%02d\r\n" % index).encode())
for index, client in enumerate(clients):
    receive(client, ("memory-%02d\r\n" % index).encode())
    client.close()

sock = connect()
receive(sock, b"Connected. MVP mode.\r\n")
sock.sendall(b"x" * 513 + b"\r\n")
sock.settimeout(2)
while sock.recv(4096):
    pass
sock.close()

sock = connect()
receive(sock, b"Connected. MVP mode.\r\n")
sock.sendall(b"healthy\r\n")
receive(sock, b"healthy\r\n")
sock.close()
PY

kill -INT "$SERVER_PID"
if ! wait "$SERVER_PID"; then
	cat "$LOG_FILE" >&2
	cat "$VALGRIND_LOG" >&2
	fail "servidor instrumentado encerrou com erro\n" >&2
	exit 1
fi
SERVER_PID=""

if grep -Eq "definitely lost: [1-9][0-9]* bytes|indirectly lost: [1-9][0-9]* bytes|ERROR SUMMARY: [1-9][0-9]* errors" "$VALGRIND_LOG"; then
	cat "$VALGRIND_LOG" >&2
	fail "Valgrind encontrou problemas de memória\n" >&2
	exit 1
fi

pass "execução, desconexões e encerramento sem erros de memória\n"
