#!/bin/bash

set -eu

PORT=${IRC_TEST_PORT:-16667}
PASSWORD=${IRC_TEST_PASSWORD:-test-password}
SERVER_PID=""
LOG_FILE="$(mktemp)"
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
		kill -INT "$SERVER_PID" 2>/dev/null || true
		wait "$SERVER_PID" 2>/dev/null || true
	fi
	rm -f "$LOG_FILE"
}

trap cleanup EXIT INT TERM

if [ ! -x "$SERVER_BINARY" ]; then
	fail "servidor não encontrado ou não é executável"
fi
if ! command -v python3 >/dev/null 2>&1; then
	fail "python3 não encontrado"
fi

"$SERVER_BINARY" "$PORT" "$PASSWORD" >"$LOG_FILE" 2>&1 &
SERVER_PID=$!

python3 - "$PORT" "$PASSWORD" <<'PY'
import socket
import sys
import time

port = int(sys.argv[1])
password = sys.argv[2].encode()

def connect():
    sock = socket.socket()
    sock.settimeout(3)
    sock.connect(("127.0.0.1", port))
    return sock

def receive_until(sock, expected):
    data = b""
    deadline = time.time() + 3
    while expected not in data and time.time() < deadline:
        chunk = sock.recv(4096)
        if not chunk:
            raise AssertionError("conexão fechada antes da resposta esperada")
        data += chunk
    if expected not in data:
        raise AssertionError(
            "resposta esperada não recebida: %r (recebido: %r)"
            % (expected, data)
        )
    return data

sock = connect()
receive_until(sock, b"Connected. MVP mode.\r\n")
sock.sendall(b"PASS " + password + b"\r\n")
sock.sendall(b"NICK srvtest\r\n")
sock.sendall(b"USER server 0 * :Server Test\r\n")
receive_until(sock, b":server 001 srvtest :Welcome to the IRC server\r\n")
sock.close()

sock = connect()
receive_until(sock, b"Connected. MVP mode.\r\n")
sock.sendall(b"raw message\r\n")
receive_until(sock, b":server 421 * RAW :Unknown command\r\n")
sock.close()

clients = [connect() for _ in range(5)]
for client in clients:
    receive_until(client, b"Connected. MVP mode.\r\n")
for index, client in enumerate(clients):
    client.sendall(("unknown%02d\r\n" % index).encode())
for index, client in enumerate(clients):
    receive_until(
        client,
        (":server 421 * UNKNOWN%02d :Unknown command\r\n" % index).encode(),
    )
    client.close()
PY

if ! kill -0 "$SERVER_PID" 2>/dev/null; then
	cat "$LOG_FILE" >&2
	fail "servidor encerrou durante a operação"
fi

pass "servidor iniciou, aceitou conexão e registrou cliente"
pass "comando desconhecido recebeu resposta 421"
pass "servidor processou múltiplas conexões simultâneas"
pass "servidor permaneceu ativo após a operação"

kill -INT "$SERVER_PID"
if ! wait "$SERVER_PID"; then
	cat "$LOG_FILE" >&2
	fail "servidor não encerrou corretamente com SIGINT"
fi
SERVER_PID=""
pass "servidor encerrou corretamente com SIGINT"
