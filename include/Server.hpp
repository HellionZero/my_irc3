/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 16:20:15 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/05 18:37:38 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "mini_irc.hpp"
# include "Client.hpp"
# include "Channel.hpp"
# include "IDispatcher.hpp"
class Server
{
	public:
		/***
		 * Constructor for the Server class.
		 * @param port The port on which the server will listen.
		 * @param password The password required to connect to the server.
		 */
		Server(int port, const std::string &password);
		~Server();

		/**
		 * Sets the command dispatcher used for complete client lines.
		 * The server does not take ownership of the dispatcher.
		 */
		void setDispatcher(IDispatcher *dispatcher);

		/***
		 * Run the server. This method will start the server's main loop, accepting new clients and handling their input/output.
		 * It will block until the server is stopped.
		 */
		void runServer(volatile sig_atomic_t &shutdownFlag);
	private:

		typedef std::map<int, Client*> 		ClientMap;
		typedef ClientMap::iterator			ClientIt;
    	typedef ClientMap::const_iterator	ClientConstIt;

		typedef std::map<std::string, Channel*>	ChannelMap;
		typedef ChannelMap::iterator		ChannelIt;
		typedef ChannelMap::const_iterator	ChannelConstIt;

		ChannelMap	_channels;

		Server(const Server &other);
		Server &operator=(const Server &other);

		int 				_port;
		std::string			_password;
		int 				_listenSocket;
		bool				_isRunning;
		std::vector<pollfd>	_pollFds;
		ClientMap			_clients;
		IDispatcher			*_dispatcher;

		/* ----------- I/O layer -------------------*/

		/* Setup */
		void	setupServerSocket(void);
		void	setNonBlocking(int socketFd);

		/* Event loop */

		void	buildPollFds(void);
		void	handleEvents(void);
		void	cleanupResources(void);

		/* Client management */

		void	acceptNewClient(void);
		void	handleClientInput(int clientFd);
		void	handleClientOutput(int clientFd);
		void	disconnectClient(int clientFd);

		void sendToClient(Client *client, const std::string &message);

		/* Protocol layer */
		void processLine(Client *client, const std::string &line);

		/* Domain layer */
		 Client  *findClientByFd(int fd);
};

#endif