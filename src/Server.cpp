/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/30 15:17:14 by lsarraci          #+#    #+#             */
/*   Updated: 2026/09/30 16:44:35 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

Server::Server(int port, const std::string &password)
	: _port(port), _password(password), _listenSocket(-1), _isRunning(false)
{
}

Server::~Server()
{
	std::vector<pollfd>::iterator it = _pollFds.begin();
	while (it != _pollFds.end())
	{
		close(it->fd);
		++it;
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

void Server::handleEvents(void)
{
	int ready = poll(&_pollFds[0], _pollFds.size(), -1);
	if (ready < 0)
	{
		if (errno == EINTR)
			return;
		throw std::runtime_error("poll failed");
	}
	if (_pollFds[0].revents & POLLIN)
		acceptNewClient();
	for (std::size_t index = 1; index < _pollFds.size(); ++index)
	{
		short events = _pollFds[index].revents;
		int clientSocket = _pollFds[index].fd;
		if (events & (POLLERR | POLLHUP | POLLNVAL))
		{
			removeClient(clientSocket);
			--index;
		}
		else if (events & POLLIN)
		{
			handleClientInput(clientSocket);
			if (index >= _pollFds.size() || _pollFds[index].fd != clientSocket)
				--index;
		}
	}
}

void Server::acceptNewClient(void)
{
	int clientSocket = accept(_listenSocket, NULL, NULL);
	if (clientSocket < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		throw std::runtime_error("Could not accept client connection");
	}
	try
	{
		setNonBlocking(clientSocket);
	}
	catch (const std::exception &error)
	{
		close(clientSocket);
		return;
	}
	pollfd clientPollFd;
	clientPollFd.fd = clientSocket;
	clientPollFd.events = POLLIN;
	clientPollFd.revents = 0;
	_pollFds.push_back(clientPollFd);
}

void Server::removeClient(int clientSocket)
{
	for (std::vector<pollfd>::iterator it = _pollFds.begin();
		it != _pollFds.end(); ++it)
	{
		if (it->fd == clientSocket)
		{
			close(clientSocket);
			_pollFds.erase(it);
			return;
		}
	}
}

void Server::handleClientInput(int clientSocket)
{
	char buffer[4096];
	ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
	if (bytesRead == 0)
	{
		removeClient(clientSocket);
		return;
	}
	if (bytesRead < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			removeClient(clientSocket);
		return;
	}
	BroadcastMessage(std::string(buffer, bytesRead), clientSocket);
}

void Server::handleClientOutput(int clientSocket)
{
	(void)clientSocket;
}

void Server::BroadcastMessage(const std::string &message, int senderSocket)
{
	for (std::vector<pollfd>::iterator it = _pollFds.begin();
		it != _pollFds.end(); ++it)
	{
		if (it->fd != _listenSocket && it->fd != senderSocket)
			send(it->fd, message.c_str(), message.size(), 0);
	}
}