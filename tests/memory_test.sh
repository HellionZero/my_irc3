#!/bin/bash

set -eu

PORT=${IRC_MEMORY_TEST_PORT:-16669}
PASSWORD=${IRC_MEMORY_TEST_PASSWORD:-test-password}
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
	fail "python3 não encontrado; necessário para o teste de memória"
fi
if [ ! -x "$SERVER_BINARY" ]; then
	fail "servidor não encontrado ou não é executável"
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
    receive(sock, b"Connected. MVP mode.\r\n")
    return sock

def receive(sock, expected):
    data = b""
    deadline = time.time() + 4
    while expected not in data and time.time() < deadline:
        chunk = sock.recv(4096)
        if not chunk:
            raise AssertionError("conexão fechada antes da resposta")
        data += chunk
    if expected not in data:
        raise AssertionError(
            "resposta esperada não recebida: %r (recebido: %r)"
            % (expected, data)
        )
    return data

def close_until_eof(sock):
    deadline = time.time() + 4
    while time.time() < deadline:
        try:
            if not sock.recv(4096):
                return
        except socket.timeout:
            continue
    raise AssertionError("conexão não foi encerrada")

sock = connect()
sock.sendall(b"PASS " + password + b"\r\n")
sock.sendall(b"NICK memtest\r\nUSER memory 0 * :Memory Test\r\n")
receive(sock, b":server 001 memtest :Welcome to the IRC server\r\n")
sock.close()

sock = connect()
sock.sendall(b"PASS wrong\r\n")
receive(sock, b":server 464 * :Password incorrect\r\n")
sock.sendall(b"PASS\r\n")
receive(sock, b":server 461 * PASS :Not enough parameters\r\n")
sock.sendall(b"PASS " + password + b"\r\n")
sock.sendall(b"NICK badcase\r\nUSER bad 0 * :Bad Case\r\n")
receive(sock, b":server 001 badcase :Welcome to the IRC server\r\n")
sock.close()

sock = connect()
sock.sendall(b"PASS " + password + b"\r\n")
sock.sendall(b"USER ordered 0 * :Ordered User\r\n")
sock.sendall(b"NICK ordered\r\n")
receive(sock, b":server 001 ordered :Welcome to the IRC server\r\n")
sock.close()

first = connect()
first.sendall(b"PASS " + password + b"\r\nNICK occupied\r\n")
first.sendall(b"USER first 0 * :First User\r\n")
receive(first, b":server 001 occupied :Welcome to the IRC server\r\n")
second = connect()
second.sendall(b"PASS " + password + b"\r\nNICK occupied\r\n")
receive(second, b":server 433 * occupied :Nickname is already in use\r\n")
second.close()
first.close()

sock = connect()
sock.sendall(b"UNKNOWN stress\r\n")
receive(sock, b":server 421 * UNKNOWN :Unknown command\r\n")
sock.sendall(b"NICK invalid_nick\r\n")
receive(sock, b":server 432 * invalid_nick :Erroneous nickname\r\n")
sock.close()

clients = [connect() for _ in range(12)]
for index, client in enumerate(clients):
    client.sendall(b"PASS " + password + b"\r\n")
    client.sendall(("NICK mem%02d\r\nUSER memory%02d 0 * :Memory User\r\n"
                    % (index, index)).encode())
for index, client in enumerate(clients):
    receive(
        client,
        (":server 001 mem%02d :Welcome to the IRC server\r\n" % index).encode(),
    )
    client.close()

sock = connect()
sock.sendall(b"x" * 513 + b"\r\n")
close_until_eof(sock)
sock.close()

sock = connect()
sock.sendall(b"y" * 4096)
close_until_eof(sock)
sock.close()

sock = connect()
sock.sendall(b"PASS " + password + b"\r\n")
sock.sendall(b"NICK healthy\r\nUSER healthy 0 * :Healthy User\r\n")
receive(sock, b":server 001 healthy :Welcome to the IRC server\r\n")
sock.close()
PY

kill -INT "$SERVER_PID"
if ! wait "$SERVER_PID"; then
	cat "$LOG_FILE" >&2
	cat "$VALGRIND_LOG" >&2
	fail "servidor instrumentado encerrou com erro"
fi
SERVER_PID=""

if grep -Eq "definitely lost: [1-9][0-9]* bytes|indirectly lost: [1-9][0-9]* bytes|ERROR SUMMARY: [1-9][0-9]* errors" "$VALGRIND_LOG"; then
	cat "$VALGRIND_LOG" >&2
	fail "Valgrind encontrou problemas de memória"
fi

pass "registro, erros de protocolo e múltiplas conexões sem erros de memória"
pass "limites de entrada, desconexões e encerramento sem erros de memória"
