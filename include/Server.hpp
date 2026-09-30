/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 16:20:15 by lsarraci          #+#    #+#             */
/*   Updated: 2026/09/30 15:58:33 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "mini_irc.hpp"

class Server
{
	public:
		Server(int port, const std::string &password);
		~Server();

		void runServer(void);
	private:
		Server(const Server &other);
		Server &operator=(const Server &other);

		int 				_port;
		std::string			_password;
		int 				_listenSocket;
		bool				_isRunning;
		std::vector<pollfd>	_pollFds;
	
		/* ----------- I/O layer -------------------*/
		
		/* Setup */
		void	setupServerSocket(void);
		void	setNonBlocking(int socketFd);

		/* Event loop */
		
		void	buildPollFds(void);
		void	handleEvents(void);

		/* Client management */

		void	acceptNewClient(void);
		void	removeClient(int clientSocket);
		void	handleClientInput(int clientSocket);
		void	handleClientOutput(int clientSocket);
		
		/* Utility functions */

		void	BroadcastMessage(const std::string &message, int senderSocket);
};

#endif