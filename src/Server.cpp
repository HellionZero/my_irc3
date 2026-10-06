/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/30 15:17:14 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/06 18:55:15 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

Server::Server(int port, const std::string &password)
	: _port(port), _password(password), _listenSocket(-1), _isRunning(false), _dispatcher(NULL)
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

void Server::setDispatcher(IDispatcher *dispatcher)
{
    _dispatcher = dispatcher;
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
	if (fcntl(socketFd, F_SETFL, O_NONBLOCK) < 0)
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
    			continue;
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

		char hostBuf[INET_ADDRSTRLEN];
		if (inet_ntop(AF_INET, &peer.sin_addr, hostBuf, sizeof(hostBuf)) == NULL)
		{
			close(clientFd);
			continue;
		}

		Client *client = NULL;
		try
		{
			client = new Client(clientFd, std::string(hostBuf));
		}
		catch (const std::exception &)
		{
			close(clientFd);
			throw;
		}
		if (!client->queueOutput(":server NOTICE * :Connected. MVP mode.\r\n"))
		{
			_clients.erase(clientFd);
			delete client;
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

    char buf[4096];
    const std::size_t maxBytesPerEvent = 16 * 1024;
    std::size_t bytesReadThisEvent = 0;
    bool peerClosed = false;

    while (bytesReadThisEvent < maxBytesPerEvent
        && client->inputBufferSize() < MAX_INPUT_BUFFER)
    {
        std::size_t bytesToRead = maxBytesPerEvent - bytesReadThisEvent;
        if (bytesToRead > sizeof(buf))
            bytesToRead = sizeof(buf);
        if (bytesToRead > MAX_INPUT_BUFFER - client->inputBufferSize())
            bytesToRead = MAX_INPUT_BUFFER - client->inputBufferSize();

        ssize_t n = recv(clientFd, buf, bytesToRead, 0);

        if (n > 0)
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
		if (errno == EINTR)
			continue; // interrupted by signal, try again
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            break;

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

    if (peerClosed)
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
    ClientIt it = _clients.find(clientFd);
    if (it == _clients.end())
        return;

    Client *client = it->second;

    for (ChannelIt channelIt = _channels.begin();
        channelIt != _channels.end();)
    {
        Channel *channel = channelIt->second;
        channel->removeMember(clientFd);
        if (channel->isEmpty())
        {
            delete channel;
            _channels.erase(channelIt++);
        }
        else
            ++channelIt;
    }

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


void Server::processLine(Client *client, const std::string &line)
{
	if (_dispatcher == NULL)
	{
		sendToClient(client, line + END_HANDLER);
		return;
	}
	std::string response = _dispatcher->dispatch(client, line);
	if (!response.empty())
		sendToClient(client, response);
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