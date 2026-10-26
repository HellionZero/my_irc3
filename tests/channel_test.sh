#!/bin/bash

set -eu

TEST_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
PROJECT_ROOT="$(CDPATH= cd -- "$TEST_DIR/.." && pwd)"
SOURCE_FILE="$(mktemp --suffix=.cpp)"
TEST_BINARY="$(mktemp)"

cleanup()
{
	rm -f "$SOURCE_FILE" "$TEST_BINARY"
}

trap cleanup EXIT INT TERM

cat > "$SOURCE_FILE" <<'CPP'
#include "Channel.hpp"
#include "ServerContext.hpp"

#include <cassert>

static void testChannelMembersAndOperators()
{
	Channel channel("#operators");

	assert(channel.getName() == "#operators");
	assert(channel.getTopic().empty());
	assert(channel.isEmpty());
	assert(!channel.isMember(10));
	assert(!channel.isOperator(10));

	channel.addOperator(10);
	assert(!channel.isOperator(10));
	channel.addMember(10);
	channel.addMember(10);
	channel.addOperator(10);
	assert(channel.isMember(10));
	assert(channel.isOperator(10));
	assert(channel.getMembers().size() == 1);
	assert(channel.getOperators().size() == 1);

	channel.addMember(11);
	channel.addOperator(11);
	assert(channel.getMembers().size() == 2);
	assert(channel.getOperators().size() == 2);
	channel.removeMember(10);
	assert(!channel.isMember(10));
	assert(!channel.isOperator(10));
	assert(channel.isMember(11));
	assert(channel.isOperator(11));
}

static void testChannelLifecycle()
{
	ServerContext context("secret");

	assert(context.findChannel("#dev") == NULL);
	assert(context.createChannel("") == NULL);
	assert(context.addClientToChannel("#dev", 10));
	assert(context.findChannel("#dev") != NULL);
	assert(context.findChannel("#dev")->isMember(10));
	assert(context.createChannel("#dev") == NULL);

	assert(context.addClientToChannel("#dev", 10));
	assert(context.addClientToChannel("#dev", 11));
	assert(context.findChannel("#dev")->getMembers().size() == 2);
	assert(context.removeClientFromChannel("#missing", 10) == false);
	assert(context.removeClientFromChannel("#dev", 10));
	assert(context.findChannel("#dev") != NULL);
	assert(context.findChannel("#dev")->getMembers().size() == 1);
	assert(context.removeClientFromChannel("#dev", 11));
	assert(context.findChannel("#dev") == NULL);
}

static void testClientRemovalFromAllChannels()
{
	ServerContext context("secret");

	assert(context.addClientToChannel("#one", 20));
	assert(context.addClientToChannel("#two", 20));
	assert(context.addClientToChannel("#two", 21));
	assert(context.addClientToChannel("#three", 22));

	context.removeClientFromChannels(20);
	assert(context.findChannel("#one") == NULL);
	assert(context.findChannel("#two") != NULL);
	assert(!context.findChannel("#two")->isMember(20));
	assert(context.findChannel("#two")->isMember(21));
	assert(context.findChannel("#three") != NULL);

	context.removeClientFromChannels(21);
	assert(context.findChannel("#two") == NULL);
	assert(context.findChannel("#three") != NULL);
	context.removeClientFromChannels(999);
	assert(context.findChannel("#three") != NULL);
}

int main()
{
	testChannelMembersAndOperators();
	testChannelLifecycle();
	testClientRemovalFromAllChannels();
	return 0;
}
CPP

c++ -Wall -Wextra -Werror -std=c++98 -I"$PROJECT_ROOT/include" \
	"$SOURCE_FILE" "$PROJECT_ROOT/src/Channel.cpp" \
	"$PROJECT_ROOT/src/Client.cpp" "$PROJECT_ROOT/src/ServerContext.cpp" \
	-o "$TEST_BINARY"
"$TEST_BINARY"
printf "\033[32m[PASS]\033[0m criação, membros, operadores e remoção de canais\n"
printf "\033[32m[PASS]\033[0m remoção de clientes preservou ou excluiu canais corretamente\n"