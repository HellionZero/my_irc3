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
#include "../include/Channel.hpp"

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

Channel *ServerContext::findChannel(const std::string &name)
{
	ChannelIt it = _channels.find(name);
	if (it == _channels.end())
		return NULL;
	return it->second;
}

Channel *ServerContext::createChannel(const std::string &name)
{
	if (name.empty() || findChannel(name) != NULL)
		return NULL;
	Channel *channel = new Channel(name);
	_channels.insert(std::make_pair(name, channel));
	return channel;
}

bool ServerContext::addClientToChannel(const std::string &name, int clientFd)
{
	Channel *channel = findChannel(name);
	if (channel == NULL)
		channel = createChannel(name);
	if (channel == NULL)
		return false;
	channel->addMember(clientFd);
	return true;
}

bool ServerContext::removeClientFromChannel(const std::string &name,
	int clientFd)
{
	ChannelIt it = _channels.find(name);
	if (it == _channels.end())
		return false;
	it->second->removeMember(clientFd);
	if (it->second->isEmpty())
	{
		delete it->second;
		_channels.erase(it);
	}
	return true;
}

void ServerContext::removeClientFromChannels(int clientFd)
{
	for (ChannelIt it = _channels.begin(); it != _channels.end();)
	{
		Channel *channel = it->second;
		channel->removeMember(clientFd);
		if (channel->isEmpty())
		{
			delete channel;
			_channels.erase(it++);
		}
		else
			++it;
	}
}