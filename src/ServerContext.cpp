/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerContext.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 16:23:52 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/23 16:54:49 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ServerContext.hpp"

ServerContext::ServerContext(const std::string &password) : _password(password) {}

ServerContext::~ServerContext() {}

const std::string &ServerContext::getPass(void) const
{
	return _password;
}

Client *ServerContext::findClientByNick(const std::string &nick)
{
	for (ClientMap::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNick() == nick)
			return it->second;
	}
	return NULL;
}

bool ServerContext::isNickInUse(const std::string &nick) const
{
	for (ClientMap::const_iterator it = _clients.begin();
		it != _clients.end(); ++it)
	{
		if (it->second->getNick() == nick)
			return true;
	}
	return false;
}

ClientMap &ServerContext::getClients(void)
{
	return _clients;
}

ChannelMap &ServerContext::getChannels(void)
{
	return _channels;
}