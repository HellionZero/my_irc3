#!/bin/bash

set -eu

PORT=${IRC_CLIENT_TEST_PORT:-16668}
PASSWORD=${IRC_CLIENT_TEST_PASSWORD:-test-password}
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
		kill -INT "$SERVER_PID" 2>/dev/null || true
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
    raise SystemExit("servidor não aceitou conexões")
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
    receive_until(sock, b"Connected. MVP mode.\r\n")
    return sock

def receive_until(sock, expected, timeout=3):
    data = b""
    deadline = time.time() + timeout
    while expected not in data and time.time() < deadline:
        try:
            chunk = sock.recv(4096)
        except socket.timeout:
            continue
        if not chunk:
            raise AssertionError("conexão fechada antes da resposta esperada")
        data += chunk
    if expected not in data:
        raise AssertionError(
            "resposta esperada não recebida: %r (recebido: %r)"
            % (expected, data)
        )
    return data

def assert_no_response(sock, timeout=0.2):
    sock.settimeout(timeout)
    try:
        data = sock.recv(4096)
    except socket.timeout:
        return
    finally:
        sock.settimeout(3)
    if data:
        raise AssertionError("resposta inesperada: %r" % data)

def close_until_eof(sock):
    deadline = time.time() + 3
    while time.time() < deadline:
        try:
            if not sock.recv(4096):
                return
        except socket.timeout:
            continue
    raise AssertionError("conexão não foi encerrada")

def register(sock, nick, user):
    sock.sendall(b"PASS " + password + b"\r\n")
    sock.sendall(("NICK %s\r\nUSER %s 0 * :Real Name\r\n" % (nick, user)).encode())
    receive_until(
        sock,
        (":server 001 %s :Welcome to the IRC server\r\n" % nick).encode(),
    )

sock = connect()
sock.sendall(b"PAS")
time.sleep(0.05)
sock.sendall(b"S " + password + b"\r\n")
assert_no_response(sock)
sock.sendall(b"NICK fragone\r\n")
sock.sendall(b"USER fragment 0 * :Fragmented User\r\n")
receive_until(sock, b":server 001 fragone :Welcome to the IRC server\r\n")
sock.close()

sock = connect()
sock.sendall(b"PASS " + password + b"\r\n")
sock.sendall(b"USER ordered 0 * :Ordered User\r\n")
assert_no_response(sock)
sock.sendall(b"NICK ordered\r\n")
receive_until(sock, b":server 001 ordered :Welcome to the IRC server\r\n")
sock.close()

sock = connect()
sock.sendall(b"PASS\r\n")
receive_until(sock, b" 461 * PASS :Not enough parameters\r\n")
sock.sendall(b"PASS wrong\r\n")
receive_until(sock, b" 464 * :Password incorrect\r\n")
sock.sendall(b"PASS " + password + b"\r\n")
sock.sendall(b"USER incomplete\r\n")
receive_until(sock, b" 461 * USER :Not enough parameters\r\n")
sock.sendall(b"NICK\r\n")
receive_until(sock, b" 431 * :No nickname given\r\n")
sock.sendall(b"NICK 123invalid\r\n")
receive_until(sock, b" 432 * 123invalid :Erroneous nickname\r\n")
sock.sendall(b"USER valid 0 * :Valid User\r\n")
sock.sendall(b"NICK valid\r\n")
receive_until(sock, b":server 001 valid :Welcome to the IRC server\r\n")
sock.sendall(b"PASS " + password + b"\r\n")
receive_until(sock, b" 462 valid :You may not reregister\r\n")
sock.sendall(b"USER again 0 * :Again\r\n")
receive_until(sock, b" 462 valid :You may not reregister\r\n")
sock.close()

first = connect()
register(first, "occupied", "first")
second = connect()
second.sendall(b"PASS " + password + b"\r\n")
second.sendall(b"NICK occupied\r\n")
receive_until(second, b" 433 * occupied :Nickname is already in use\r\n")
second.close()
first.close()

sock = connect()
sock.sendall(b"UNKNOWN command\r\n")
receive_until(sock, b" 421 * UNKNOWN :Unknown command\r\n")
sock.sendall(b"first\r\nsecond\r\nthird\r\n")
receive_until(sock, b" 421 * THIRD :Unknown command\r\n")
sock.close()

sock = connect()
sock.sendall(b"x" * 513 + b"\r\n")
close_until_eof(sock)
sock.close()

sock = connect()
sock.sendall(b"y" * 4096)
close_until_eof(sock)
sock.close()

clients = [connect() for _ in range(8)]
for index, client in enumerate(clients):
    client.sendall(("NICK burst%02d\r\n" % index).encode())
    client.sendall(b"PASS " + password + b"\r\n")
    client.sendall(("USER burst%02d 0 * :Burst User\r\n" % index).encode())
for index, client in enumerate(clients):
    receive_until(
        client,
        (":server 001 burst%02d :Welcome to the IRC server\r\n" % index).encode(),
    )
    client.close()

sock = connect()
sock.sendall(b"abrupt\r\n")
sock.close()

healthy = connect()
healthy.sendall(b"PASS " + password + b"\r\nNICK healthy\r\n")
healthy.sendall(b"USER healthy 0 * :Healthy User\r\n")
receive_until(healthy, b":server 001 healthy :Welcome to the IRC server\r\n")
healthy.close()
PY

if ! kill -0 "$SERVER_PID" 2>/dev/null; then
	cat "$LOG_FILE" >&2
	fail "servidor encerrou durante os cenários de cliente"
fi

pass "registro com PASS, NICK e USER em ordens diferentes"
pass "respostas numéricas para comandos inválidos e reregistro"
pass "fragmentação, múltiplas linhas e conexões simultâneas"
pass "limites de linha, buffer de entrada e desconexões abruptas"

kill -INT "$SERVER_PID"
if ! wait "$SERVER_PID"; then
	cat "$LOG_FILE" >&2
	fail "servidor não encerrou corretamente com SIGINT"
fi
SERVER_PID=""
pass "servidor encerrou corretamente com SIGINT"
