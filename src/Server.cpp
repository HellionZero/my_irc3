/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/30 15:17:14 by lsarraci          #+#    #+#             */
/*   Updated: 2026/09/30 17:48:04 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

Server::Server(int port, const std::string &password)
	: _port(port), _password(password), _listenSocket(-1), _isRunning(false)
{
}

Server::~Server()
{
    for (ClientIt it = _clients.begin(); it != _clients.end(); ++it)
        delete it->second;
    _clients.clear();
    if (_listenSocket >= 0)
	{
        close(_listenSocket);
        _listenSocket = -1;
	}
}

void Server::runServer(void)
{
	setupServerSocket();
	buildPollFds();
	_isRunning = true;
	while (_isRunning)
		handleEvents();
}

void Server::setupServerSocket(void)
{
    _listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenSocket < 0)
        throw std::runtime_error("socket() failed");

    int opt = 1;
    if (setsockopt(_listenSocket, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)) < 0)
    {
        close(_listenSocket);
        _listenSocket = -1;
        throw std::runtime_error("setsockopt() failed");
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(_port);

    if (bind(_listenSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        close(_listenSocket);
        _listenSocket = -1;
        throw std::runtime_error("bind() failed");
    }

    if (listen(_listenSocket, SOMAXCONN) < 0)
    {
        close(_listenSocket);
        _listenSocket = -1;
        throw std::runtime_error("listen() failed");
    }

    setNonBlocking(_listenSocket);
}

void Server::setNonBlocking(int socketFd)
{
	if (fcntl(socketFd, F_SETFL, O_NONBLOCK) < 0)
        throw std::runtime_error("fcntl(F_SETFL) failed");
}

void Server::buildPollFds(void)
{
	pollfd listeningPollFd;

	listeningPollFd.fd = _listenSocket;
	listeningPollFd.events = POLLIN;
	listeningPollFd.revents = 0;
	_pollFds.push_back(listeningPollFd);
}

void Server::handleEvents()
{
    int ready = poll(&_pollFds[0], _pollFds.size(), -1);
    if (ready < 0)
    {
        if (errno == EINTR)
            return;
        throw std::runtime_error("poll failed");
    }

    // copia para iterar com segurança: disconnectClient pode alterar _pollFds
    std::vector<struct pollfd> events = _pollFds;

    for (std::size_t i = 0; i < events.size(); ++i)
    {
        int fd = events[i].fd;
        short revents = events[i].revents;

        if (revents == 0)
            continue;

        if (fd == _listenSocket)
        {
            if (revents & POLLIN)
                acceptNewClient();
            continue;
        }

        // cliente
        if (revents & (POLLERR | POLLHUP | POLLNVAL))
        {
            disconnectClient(fd);
            continue;
        }

        if (revents & POLLIN)
            handleClientInput(fd);

        // revalida: handleClientInput pode ter deletado
        if (_clients.find(fd) == _clients.end())
            continue;

        if (revents & POLLOUT)
            handleClientOutput(fd);
    }
}

void Server::acceptNewClient()
{
    struct sockaddr_in peer;
    socklen_t peerLen = sizeof(peer);

    int clientFd = accept(_listenSocket, (struct sockaddr *)&peer, &peerLen);
    if (clientFd < 0)
        return;   // EAGAIN/EWOULDBLOCK: normal on non-blocking sockets

    try
    {
        setNonBlocking(clientFd);
    }
    catch (const std::exception &error)
    {
        close(clientFd);
        return;
    }

    char hostBuf[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &peer.sin_addr, hostBuf, sizeof(hostBuf)) == NULL)
    {
        close(clientFd);
        return;
    }

    Client *client = new Client(clientFd, std::string(hostBuf));
    _clients.insert(std::make_pair(clientFd, client));

    // temporary MVP: echoes back a message to the client upon connection
    client->queueOutput(":server NOTICE * :Connected. MVP mode.\r\n");

    pollfd clientPollFd;

    clientPollFd.fd = clientFd;
    clientPollFd.events = POLLIN | POLLOUT;
    clientPollFd.revents = 0;
    _pollFds.push_back(clientPollFd);
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

    while (bytesReadThisEvent < maxBytesPerEvent)
    {
        std::size_t bytesToRead = maxBytesPerEvent - bytesReadThisEvent;
        if (bytesToRead > sizeof(buf))
            bytesToRead = sizeof(buf);

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
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            break;

        // real error
        disconnectClient(clientFd);
        return;
    }

    // extract complete lines from the client's input buffer and process them
    std::string line;
    while (client->popLine(line))
    {
        processLine(client, line);

        // return if the client was disconnected during processing
        if (_clients.find(clientFd) == _clients.end())
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
        {
            client->consumeOutput(static_cast<std::size_t>(n));
            if (!client->hasPendingOutput())
            {
                for (std::vector<pollfd>::iterator it = _pollFds.begin();
                    it != _pollFds.end(); ++it)
                {
                    if (it->fd == clientFd)
                    {
                        it->events = static_cast<short>(it->events & ~POLLOUT);
                        break;
                    }
                }
            }
        }
        else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
        {
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

    // 1. remove client from all channels and delete empty channels (not implemented yet)

    // 2. removes from map and deletes the client (which closes the fd)
    _clients.erase(it);
    delete client;   // ~Client closes the fd

    for (std::vector<pollfd>::iterator pollIt = _pollFds.begin();
        pollIt != _pollFds.end(); ++pollIt)
    {
        if (pollIt->fd == clientFd)
        {
            _pollFds.erase(pollIt);
            break;
        }
    }
}

// ============================================================================
// I/O helpers
// ============================================================================

void Server::sendToClient(Client *client, const std::string &message)
{
    if (client == NULL)
        return;
    client->queueOutput(message);
    for (std::vector<pollfd>::iterator it = _pollFds.begin();
        it != _pollFds.end(); ++it)
    {
        if (it->fd == client->getFd())
        {
            it->events = static_cast<short>(it->events | POLLOUT);
            break;
        }
    }
}


void Server::processLine(Client *client, const std::string &line)
{
    // MVP: echo the line back to test I/O
    // substitute later for: dispatchCommand(client, Parser::parse(line));
    sendToClient(client, line + "\r\n");
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