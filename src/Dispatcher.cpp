/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Dispatcher.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/21 18:00:51 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/23 17:10:00 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Dispatcher.hpp"

#include <cctype>
#include <iomanip>
#include <sstream>

Dispatcher::Dispatcher(IServerContext &context) : _context(context) {}

Dispatcher::~Dispatcher() {}

std::string Dispatcher::dispatch(Client *client, const std::string &line)
{
	Parser::Command command;

	if (client == NULL)
		return "";
	try
	{
		command = Parser::parse(line);
	}
	catch (const Parser::ParseException &)
	{
		return makeNumeric(client, ERR_UNKNOWNCOMMAND,
			":Invalid command");
	}
	if (!command._isValid)
		return makeNumeric(client, ERR_UNKNOWNCOMMAND,
			":Invalid command");

	command._name = upper(command._name);
	if (command._name == "PASS")
		return handlePass(client, command);
	if (command._name == "NICK")
		return handleNick(client, command);
	if (command._name == "USER")
		return handleUser(client, command);
	if (command._name == "QUIT")
		return handleQuit(client, command);
	return makeNumeric(client, ERR_UNKNOWNCOMMAND,
		command._name + " :Unknown command");
}

std::string Dispatcher::handlePass(Client *client,
	const Parser::Command &command)
{
	if (client->getState() == Client::REGISTERED)
		return makeNumeric(client, ERR_ALREADYREGISTRED,
			":You may not reregister");
	if (command._params.size() != 1)
		return makeNumeric(client, ERR_NEEDMOREPARAMS,
			"PASS :Not enough parameters");
	if (command._params[0] != _context.getPass())
		return makeNumeric(client, ERR_PASSWDMISMATCH,
			":Password incorrect");

	client->setPasswordAccepted(true);
	if (client->getState() == Client::HANDSHAKE)
		client->setState(Client::CONNECTED);
	return "";
}

std::string Dispatcher::handleNick(Client *client,
	const Parser::Command &command)
{
	if (command._params.size() != 1 || command._params[0].empty())
		return makeNumeric(client, ERR_NONICKNAMEGIVEN,
			":No nickname given");

	const std::string &nick = command._params[0];
	if (nick.size() > 9 || !std::isalpha(static_cast<unsigned char>(nick[0])))
		return makeNumeric(client, ERR_ERRONEUSNICKNAME,
			nick + " :Erroneous nickname");
	for (std::size_t i = 1; i < nick.size(); ++i)
	{
		unsigned char character = static_cast<unsigned char>(nick[i]);
		if (!std::isalnum(character) && nick[i] != '-')
			return makeNumeric(client, ERR_ERRONEUSNICKNAME,
				nick + " :Erroneous nickname");
	}
	if (_context.isNickInUse(nick)
		&& client->getNick() != nick)
		return makeNumeric(client, ERR_NICKNAMEINUSE,
			nick + " :Nickname is already in use");

	client->setNick(nick);
	if (tryRegister(client))
		return makeNumeric(client, RPL_WELCOME,
			":Welcome to the IRC server");
	return "";
}

std::string Dispatcher::handleUser(Client *client,
	const Parser::Command &command)
{
	if (client->getState() == Client::REGISTERED)
		return makeNumeric(client, ERR_ALREADYREGISTRED,
			":You may not reregister");
	if (command._params.size() != 4)
		return makeNumeric(client, ERR_NEEDMOREPARAMS,
			"USER :Not enough parameters");

	client->setUser(command._params[0]);
	if (tryRegister(client))
		return makeNumeric(client, RPL_WELCOME,
			":Welcome to the IRC server");
	return "";
}

std::string Dispatcher::handleQuit(Client *client,
	const Parser::Command &command)
{
	std::string message;

	if (command._params.size() > 1)
		return makeNumeric(client, ERR_NEEDMOREPARAMS,
			"QUIT :Too many parameters");
	if (!command._params.empty())
		message = command._params[0];
	client->setState(Client::DISCONNECTING);
	if (message.empty())
		message = "Client Quit";
	return std::string(":") + clientName(client) + " QUIT :" + message
		+ END_HANDLER;
}

std::string Dispatcher::makeNumeric(Client *client, int code,
	const std::string &message) const
{
	std::ostringstream number;
	number << std::setfill('0') << std::setw(3) << code;
	return std::string(":server ") + number.str() + " "
		+ clientName(client) + " " + message + END_HANDLER;
}

std::string Dispatcher::clientName(const Client *client) const
{
	if (client != NULL && !client->getNick().empty())
		return client->getNick();
	return "*";
}

std::string Dispatcher::upper(const std::string &value) const
{
	std::string result = value;
	for (std::size_t i = 0; i < result.size(); ++i)
		result[i] = static_cast<char>(
			std::toupper(static_cast<unsigned char>(result[i])));
	return result;
}

bool Dispatcher::tryRegister(Client *client) const
{
	if (!client->hasAcceptedPassword()
		|| client->getNick().empty()
		|| client->getUser().empty()
		|| client->getState() == Client::REGISTERED)
		return false;
	client->setState(Client::REGISTERED);
	return true;
}
