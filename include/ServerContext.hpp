/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerContext.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 16:22:19 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/26 14:41:45 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONTEXT_HPP
# define SERVERCONTEXT_HPP

# include "IServerContext.hpp"
# include "ServerTypes.hpp"

/// @brief The ServerContext class is responsible for managing
/// the state of the server, including the list of connected clients
/// and channels. 
/// It holds the values of the server password, and provides
/// methods to find clients by their nicknames and check if 
/// a nickname is already in use.
class ServerContext : public IServerContext
{
	public:
		ServerContext(const std::string &password);
		virtual ~ServerContext();

		virtual const std::string &getPass(void) const;
		virtual Client *findClientByNick(const std::string &nick);
		virtual bool isNickInUse(const std::string &nick) const;

		/* getters */
		ClientMap &getClients(void);
		ChannelMap &getChannels(void);

		/* channel management */
		Channel *findChannel(const std::string &name);
		Channel *createChannel(const std::string &name);
		bool addClientToChannel(const std::string &name, int clientFd);
		bool removeClientFromChannel(const std::string &name, int clientFd);
		void removeClientFromChannels(int clientFd);

	private:
		std::string _password;
		ClientMap	_clients;
		ChannelMap	_channels;
};

#endif
