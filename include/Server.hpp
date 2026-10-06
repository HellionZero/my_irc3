/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 16:20:15 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/06 18:13:11 by lsarraci         ###   ########.fr       */
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
		 * @brief Constructor for the Server class.
		 * @param port The port on which the server will listen.
		 * @param password The password required to connect to the server.
		 */
		Server(int port, const std::string &password);
		~Server();

		/**
		 * @brief Sets the command dispatcher used for complete client lines.
		 * The server does not take ownership of the dispatcher.
		 */
		void setDispatcher(IDispatcher *dispatcher);

		/***
		 * @brief Run the server. This method will start the server's main loop, accepting new clients and handling their input/output.
		 * It will block until the server is stopped.
		 * 
		 * for this method, we will use a volatile sig_atomic_t flag to indicate when the server should stop running.
		 * @param shutdownFlag A reference to a volatile sig_atomic_t flag that indicates when the
		 * server should stop running.
		 * The server will check this flag periodically and exit the main loop when it is set to a non-zero value.
		 * This allows the server to be stopped gracefully from a signal handler or another thread.
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

		/* ----------- I/O LAYER -------------------*/

		/* ------- Setup -----------*/

		/***
		 * @brief Sets up the server socket, binds to the specified port, and starts listening for incoming connections.
		 * Throws a std::runtime_error if any of the socket operations fail.
		 */
		void	setupServerSocket(void);
		
		/***
		 * @brief Sets the given socket file descriptor to non-blocking mode.
		 * @param socketFd The file descriptor of the socket to set to non-blocking mode
		 */
		void	setNonBlocking(int socketFd);

		/* Event loop */

		/***
		 * @brief Builds the pollfd array for the poll() system call. It is necessary to call this method after 
		 * accepting a new client, disconnecting a client, or changing a client's output buffer.
		 * This method rebuilds the array from the current listening socket and all connected clients.
		 * POLLOUT is enabled only for clients with pending output.
		 */
		void	buildPollFds(void);

		/***
		 * @brief Handles events on the server's sockets.
		 * It uses the poll() system call to reconstruct the pollfd array and wait
		 * for events on the listening socket and all connected client sockets.
		 * It will accept new clients, handle input and output for existing clients,
		 * and disconnect clients that have disconnected or encountered an error.
		 */
		void	handleEvents(void);

		/***
		 * @brief Cleans up all resources used by the server, including closing sockets and deleting
		 * client and channel objects.
		 * This method is called when the server is shutting down or when an error occurs that
		 * requires the server to stop.
		 * 
		 * It is safe to call this method multiple times, as it will check if resources have 
		 * already been cleaned up before attempting to clean them up again.
		 * This method will also clear the pollfd array, so that the server can be restarted 
		 * without having to rebuild the pollfd array from scratch.
		 * This method is called by the destructor, so it is not necessary to call it explicitly.
		 */
		void	cleanupResources(void);

		/* -------------------- Client management --------------------*/

		void	acceptNewClient(void);
		void	handleClientInput(int clientFd);
		void	handleClientOutput(int clientFd);
		void	disconnectClient(int clientFd);

		void sendToClient(Client *client, const std::string &message);

		/* --------------------- PROTOCOL LAYER ---------------------*/
		void processLine(Client *client, const std::string &line);

		/* Domain layer */
		 Client  *findClientByFd(int fd);
};

#endif