#!/bin/bash

set -eu

PORT=${IRC_CLIENT_TEST_PORT:-16668}
PASSWORD=${IRC_TEST_PASSWORD:-test-password}
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
PROJECT_ROOT="$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)"
SERVER_BINARY="$PROJECT_ROOT/ircserv"
SERVER_PID=""
LOG_FILE="$(mktemp)"
TEST_DIR="$(mktemp -d)"

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
		kill "$SERVER_PID" 2>/dev/null || true
		wait "$SERVER_PID" 2>/dev/null || true
	fi
	rm -f "$LOG_FILE"
	rm -rf "$TEST_DIR"
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

python3 - "$PORT" <<'PY'
import socket
import sys
import time

port = int(sys.argv[1])
for _ in range(20):
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
    raise SystemExit("servidor não aceitou conexões")
PY

python3 - "$PORT" <<'PY'
import socket
import sys
import time

port = int(sys.argv[1])

def connect():
    sock = socket.socket()
    sock.settimeout(2)
    sock.connect(("127.0.0.1", port))
    return sock

def receive_until(sock, expected, timeout=3):
    data = b""
    deadline = time.time() + timeout
    while expected not in data and time.time() < deadline:
        data += sock.recv(4096)
    if expected not in data:
        raise AssertionError("resposta esperada não recebida")
    return data

def close_until_eof(sock):
    deadline = time.time() + 2
    while time.time() < deadline:
        if not sock.recv(4096):
            return
    raise AssertionError("conexão não foi encerrada")

sock = connect()
receive_until(sock, b"Connected. MVP mode.\r\n")
sock.sendall(b"fragment")
time.sleep(0.05)
sock.sendall(b"ed message\r\n")
receive_until(sock, b"fragmented message\r\n")
sock.close()

sock = connect()
receive_until(sock, b"Connected. MVP mode.\r\n")
sock.sendall(b"first\r\nsecond\r\nthird\r\n")
data = receive_until(sock, b"third\r\n")
for message in (b"first\r\n", b"second\r\n", b"third\r\n"):
    if message not in data:
        raise AssertionError("linha múltipla ausente")
sock.close()

sock = connect()
receive_until(sock, b"Connected. MVP mode.\r\n")
for index in range(20):
    sock.sendall(("line-%02d\r\n" % index).encode())
data = b""
deadline = time.time() + 3
while b"line-19\r\n" not in data and time.time() < deadline:
    data += sock.recv(4096)
if b"line-19\r\n" not in data:
    raise AssertionError("linhas pendentes não foram processadas")
sock.close()

sock = connect()
receive_until(sock, b"Connected. MVP mode.\r\n")
sock.sendall(b"x" * 513 + b"\r\n")
close_until_eof(sock)
sock.close()

sock = connect()
receive_until(sock, b"Connected. MVP mode.\r\n")
sock.sendall(b"y" * 4096)
close_until_eof(sock)
sock.close()

clients = [connect() for _ in range(8)]
for client in clients:
    receive_until(client, b"Connected. MVP mode.\r\n")
for index, client in enumerate(clients):
    client.sendall(("burst-%02d\r\n" % index).encode())
for index, client in enumerate(clients):
    receive_until(client, ("burst-%02d\r\n" % index).encode())
    client.close()

sock = connect()
receive_until(sock, b"Connected. MVP mode.\r\n")
sock.sendall(b"abrupt\r\n")
sock.close()

healthy = connect()
receive_until(healthy, b"Connected. MVP mode.\r\n")
healthy.sendall(b"healthy\r\n")
receive_until(healthy, b"healthy\r\n")
healthy.close()

sock = connect()
receive_until(sock, b"Connected. MVP mode.\r\n")
sock.sendall(b"signal-check\r\n")
receive_until(sock, b"signal-check\r\n")
sock.close()
PY

if ! kill -0 "$SERVER_PID" 2>/dev/null; then
	cat "$LOG_FILE" >&2
	fail "servidor encerrou durante os cenários de cliente"
fi

pass "mensagens fragmentadas e múltiplas linhas"
pass "limites de linha e buffer de entrada"
pass "conexões simultâneas e desconexões abruptas"
kill -INT "$SERVER_PID"
if ! wait "$SERVER_PID"; then
	cat "$LOG_FILE" >&2
	fail "servidor não encerrou corretamente com SIGINT"
fi
SERVER_PID=""
pass "servidor encerrou corretamente com SIGINT"
