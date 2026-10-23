/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerContext.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 16:23:52 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/23 16:24:43 by lsarraci         ###   ########.fr       */
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
	(void)nick;
	// This method should search for a client by their nickname.
	// Since we don't have access to the client list here, this is just a placeholder.
	// In a real implementation, you would need to have access to the client list.
	return NULL;
}

bool ServerContext::isNickInUse(const std::string &nick) const
{
	(void)nick;
	// This method should check if a nickname is already in use.
	// Since we don't have access to the client list here, this is just a placeholder.
	// In a real implementation, you would need to have access to the client list.
	return false;
}