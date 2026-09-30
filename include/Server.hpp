/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 16:20:15 by lsarraci          #+#    #+#             */
/*   Updated: 2026/09/30 17:24:36 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "mini_irc.hpp"
# include "Client.hpp"

class Server
{
	public:
		Server(int port, const std::string &password);
		~Server();

		void runServer(void);
	private:

		typedef std::map<int, Client*> 		ClientMap;
		typedef ClientMap::iterator			ClientIt;
    	typedef ClientMap::const_iterator	ClientConstIt;
		
		Server(const Server &other);
		Server &operator=(const Server &other);

		int 				_port;
		std::string			_password;
		int 				_listenSocket;
		bool				_isRunning;
		std::vector<pollfd>	_pollFds;
		ClientMap			_clients;
	
		/* ----------- I/O layer -------------------*/
		
		/* Setup */
		void	setupServerSocket(void);
		void	setNonBlocking(int socketFd);

		/* Event loop */
		
		void	buildPollFds(void);
		void	handleEvents(void);

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