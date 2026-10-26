/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/30 15:17:14 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/26 14:13:26 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

Server::Server(int port, ServerContext &context, IDispatcher &dispatcher)
	: _port(port), _listenSocket(-1), _isRunning(false),
	  _dispatcher(dispatcher), _context(context),
	  _clients(context.getClients()), _channels(context.getChannels())
{
}

Server::~Server()
{
    cleanupResources();
}

void Server::cleanupResources(void)
{
    for (ClientIt it = _clients.begin(); it != _clients.end(); ++it)
        delete it->second;
    _clients.clear();
    for (ChannelIt it = _channels.begin(); it != _channels.end(); ++it)
        delete it->second;
    _channels.clear();
    _pollFds.clear();
    if (_listenSocket >= 0)
	{
        close(_listenSocket);
        _listenSocket = -1;
	}
}

void Server::runServer(volatile sig_atomic_t &shutdownFlag)
{
    cleanupResources();
    _isRunning = false;
    if (shutdownFlag)
        return;

    setupServerSocket();
    buildPollFds();
    _isRunning = true;
    std::cout << "Server running on port " << _port << std::endl;
    while (_isRunning && !shutdownFlag)
        handleEvents();
    cleanupResources();
}

void Server::setupServerSocket(void)
{
    _listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenSocket < 0)
        throw std::runtime_error(
            std::string("socket() failed: ") + std::strerror(errno));

    int opt = 1;
    if (setsockopt(_listenSocket, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)) < 0)
    {
        std::string error = std::string("setsockopt() failed: ")
            + std::strerror(errno);
        close(_listenSocket);
        _listenSocket = -1;
        throw std::runtime_error(error);
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(_port);

	/* bind operation: associate the socket with the address and port */
    if (bind(_listenSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        std::string error = std::string("bind() failed: ")
            + std::strerror(errno);
        close(_listenSocket);
        _listenSocket = -1;
        throw std::runtime_error(error);
    }

	/* listen operation: mark the socket as a passive socket that will be used to accept incoming connection requests */
    if (listen(_listenSocket, SOMAXCONN) < 0)
    {
        std::string error = std::string("listen() failed: ")
            + std::strerror(errno);
        close(_listenSocket);
        _listenSocket = -1;
        throw std::runtime_error(error);
    }

    setNonBlocking(_listenSocket);
}

void Server::setNonBlocking(int socketFd)
{
	if (fcntl(socketFd, F_SETFL, O_NONBLOCK) < 0) /// if the fcntl() call fails, throw a runtime error with the error message
        throw std::runtime_error(
            std::string("fcntl(F_SETFL) failed: ") + std::strerror(errno));
}

void Server::buildPollFds(void)
{
	_pollFds.clear();
	_pollFds.reserve(1 + _clients.size()); // reserve space for the listening socket and all clients
	
	pollfd listeningPollFd;

	listeningPollFd.fd = _listenSocket;
	listeningPollFd.events = POLLIN;
	listeningPollFd.revents = 0;
	_pollFds.push_back(listeningPollFd);
	
	for (ClientMap::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		Client *client = it->second;
		
		pollfd clientPollFd;
		clientPollFd.fd = client->getFd();
		clientPollFd.events = POLLIN;
		if (client->hasPendingOutput())
			clientPollFd.events = static_cast<short>(clientPollFd.events | POLLOUT);
		clientPollFd.revents = 0;
		_pollFds.push_back(clientPollFd);
	}
}

void Server::handleEvents()
{
	// creates the pollfd array and waits for events on the listening socket and all connected client sockets
	int timeout = 1000;
	std::vector<int> pendingInput;
	for (ClientConstIt it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->hasCompleteLine())
		{
			timeout = 0;
			pendingInput.push_back(it->first);
		}
	}

    int ready = poll(&_pollFds[0], _pollFds.size(), timeout);
    if (ready < 0)
    {
        if (errno == EINTR) // if the poll was interrupted by a signal, just return and continue the loop
            return;
        throw std::runtime_error(
            std::string("poll failed: ") + std::strerror(errno));
    }

	if (ready == 0)
	{
		for (std::vector<int>::const_iterator it = pendingInput.begin();
			it != pendingInput.end(); ++it)
		{
			if (_clients.find(*it) != _clients.end())
				handleClientInput(*it);
		}
		buildPollFds();
		return;
	}

    // copy the entire _pollFds vector to avoid issues with iterators invalidation during client disconnection
    std::vector<struct pollfd> events = _pollFds;

    for (std::size_t i = 0; i < events.size(); ++i) // loop over the copied events vector
    {
        int fd = events[i].fd;
        short revents = events[i].revents;

		// no events for this fd
        if (revents == 0)
            continue;

		// listening socket
        if (fd == _listenSocket)
        {
			if (revents & (POLLERR | POLLHUP | POLLNVAL))
				throw std::runtime_error("listening socket poll failure");
            if (revents & POLLIN) // if there is a new connection request
                acceptNewClient();
            continue;
        }

        // client socket
        if (revents & (POLLERR | POLLHUP | POLLNVAL)) // verify if the client still exists and if there is an error, hangup or invalid request.
        {
            disconnectClient(fd);
            continue;
        }

		// verify if the client still exists after handling input, as it may have been disconnected
        if (revents & POLLIN)
            handleClientInput(fd);

        // revalidation after handling input, as the client may have been disconnected
        if (_clients.find(fd) == _clients.end())
            continue;

		// handle output if the client still exists
        if (revents & POLLOUT)
            handleClientOutput(fd);
    }
	buildPollFds(); // rebuild the pollfd array after handling events, as clients may have been added or removed
}

void Server::acceptNewClient()
{
	bool clientAccepted = false;
	
	while (true)
	{
		struct sockaddr_in peer;
		socklen_t peerLen = sizeof(peer);

		int clientFd = accept(_listenSocket, (struct sockaddr *)&peer, &peerLen);
		if (clientFd < 0)
		{
			if (errno == EINTR)
				continue; // interrupted by signal, try again
			if (errno == ECONNABORTED)
    			continue; // connection was aborted, try again
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break; // no more clients to accept
			throw std::runtime_error(
				std::string("accept() failed: ") + std::strerror(errno));
		}
		try
		{
			setNonBlocking(clientFd);
		}
		catch (const std::exception &)
		{
			close(clientFd);
			continue;
		}

		char hostBuf[INET_ADDRSTRLEN]; /// buffer to hold the string representation of the client's IP address
		if (inet_ntop(AF_INET, &peer.sin_addr, hostBuf, sizeof(hostBuf)) == NULL) /// if the conversion fails, close the client socket and continue to the next iteration
		{
			close(clientFd);
			continue;
		}

		Client *client = NULL; // initialize the client pointer to NULL, to be assigned later after successful creation
		try
		{
			client = new Client(clientFd, std::string(hostBuf));
		}
		catch (const std::exception &)
		{
			close(clientFd);
			throw;
		}
		if (!client->queueOutput(":server NOTICE * :Connected. MVP mode.\r\n")) // if the initial output exceeds the limit of 1024 characters, disconnect the client and throw an exception
		{
			_clients.erase(clientFd); /// erase the client from the map of clients, if it was added before the exception was thrown
			delete client; /// delete the client object to free memory, as it was not successfully added to the map of clients
			throw std::runtime_error("initial client output exceeds limit");
		}
		_clients.insert(std::make_pair(clientFd, client));
		
		clientAccepted = true;
	}
	if (clientAccepted)
		buildPollFds(); // rebuild the pollfd array after accepting new clients
}

void Server::handleClientInput(int clientFd)
{
    Client *client = findClientByFd(clientFd);
    if (client == NULL)
        return;

    char buf[MAX_INPUT_BUFFER]; // buffer to hold the data read from the client socket
    const std::size_t maxBytesPerEvent = BUFFER_DEFAULT_SIZE * MAX_CYCLE_LINES; // maximum number of bytes to read from the client socket in a single event
    std::size_t bytesReadThisEvent = 0; /// initialize the number of bytes read from the client socket in this event to 0
    bool peerClosed = false; /// flag to indicate if the client has closed the connection, initialized to false

    while (bytesReadThisEvent < maxBytesPerEvent
        && client->inputBufferSize() < MAX_INPUT_BUFFER) /// loop until the maximum number of bytes per event is reached or the client's input buffer is full
    {
        std::size_t bytesToRead = maxBytesPerEvent - bytesReadThisEvent;
        if (bytesToRead > sizeof(buf))
            bytesToRead = sizeof(buf);
        if (bytesToRead > MAX_INPUT_BUFFER - client->inputBufferSize())
            bytesToRead = MAX_INPUT_BUFFER - client->inputBufferSize();

        ssize_t n = recv(clientFd, buf, bytesToRead, 0); /// read data from the client socket into the buffer

        if (n > 0) /// if data was read from the client socket, append it to the client's input buffer and update the number of bytes read in this event
        {
            client->appendInput(buf, static_cast<std::size_t>(n));
            bytesReadThisEvent += static_cast<std::size_t>(n);
            continue;
        }
        if (n == 0)
        {
            peerClosed = true;
            break;
        }
		if (errno == EINTR) /// if the recv() call was interrupted by a signal, continue to the next iteration of the loop to try again
			continue; // interrupted by signal, try again
        if (errno == EAGAIN || errno == EWOULDBLOCK) 
            break; /// break if the recv() call would block, indicating that there is no more data to read from the client socket at this time

        // real error
        disconnectClient(clientFd);
        return;
    }

	if (client->inputBufferSize() > MAX_LINE_LIMIT
		&& !client->hasCompleteLine())
	{
		disconnectClient(clientFd);
		return;
	}

    // extract complete lines from the client's input buffer and process them
    std::string line;
	std::size_t linesProcessed = 0;
    while (linesProcessed < MAX_CYCLE_LINES && client->popLine(line))
    {
		if (line.size() > MAX_LINE_LIMIT)
		{
			disconnectClient(clientFd);
			return;
		}
        processLine(client, line);
		++linesProcessed;

        // return if the client was disconnected during processing
        if (_clients.find(clientFd) == _clients.end())
            return;
    }

	if (client->inputBufferSize() > MAX_LINE_LIMIT
        && !client->hasCompleteLine())
	{
        disconnectClient(clientFd);
        return;
	}

    if (peerClosed) /// alter the state of the client to DISCONNECTING if the peer has closed the connection, and disconnect the client if there is no pending output
    {
        client->setState(Client::DISCONNECTING);
        if (!client->hasPendingOutput())
            disconnectClient(clientFd);
    }
}

void Server::handleClientOutput(int clientFd)
{
    Client *client = findClientByFd(clientFd);
    if (client == NULL)
        return;

    const std::string &buf = client->outputBuffer();
    if (!buf.empty())
    {
        ssize_t n = send(clientFd, buf.data(), buf.size(), 0);
        if (n > 0)
            client->consumeOutput(static_cast<std::size_t>(n));
        else if (n == 0)
            return;
        else if (n < 0)
        {
			if (errno == EINTR)
				return;
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return;

			disconnectClient(clientFd);
			return;
		}
    }

    // disconnect if the client is in DISCONNECTING state and has no pending output
    if (client->getState() == Client::DISCONNECTING
        && !client->hasPendingOutput())
    {
        disconnectClient(clientFd);
    }
}

void Server::disconnectClient(int clientFd)
{
    ClientIt it = _clients.find(clientFd); /// create an iterator to the client in the _clients map using the clientFd
    if (it == _clients.end())
        return;

    Client *client = it->second; /// get the Client pointer from the iterator
    if (client == NULL)
        return; // if the Client pointer is NULL, return without doing anything

    _context.removeClientFromChannels(clientFd);

    _clients.erase(it);
    delete client;
	buildPollFds(); // rebuild the pollfd array after removing a client
}

// ============================================================================
// I/O helpers
// ============================================================================

void Server::sendToClient(Client *client, const std::string &message)
{
    if (client == NULL)
        return;
	
    if (!client->queueOutput(message))
    {
        disconnectClient(client->getFd());
        return;
    }
    buildPollFds();
}

void Server::broadcastToClients(const std::string &message, int excludedFd)
{
    for (ClientIt it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->first != excludedFd)
            sendToClient(it->second, message);
    }
}


void Server::processLine(Client *client, const std::string &line)
{
	std::string response = _dispatcher.dispatch(client, line);
	if (!response.empty())
	{
		if (client->getState() == Client::DISCONNECTING)
		{
			broadcastToClients(response, client->getFd());
			disconnectClient(client->getFd());
		}
		else
			sendToClient(client, response);
	}
}

// ============================================================================
// Domain helpers
// ============================================================================

Client *Server::findClientByFd(int fd)
{
    ClientIt it = _clients.find(fd);
    if (it == _clients.end())
        return NULL;
    return it->second;
}