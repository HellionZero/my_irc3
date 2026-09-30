/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/30 16:59:53 by lsarraci          #+#    #+#             */
/*   Updated: 2026/09/30 17:10:51 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"

#include <unistd.h>

Client::Client(int socketFd, const std::string &hostname) :
	_fd(socketFd), _host(hostname), _nick(""), _user(""), _state(HANDSHAKE), _inBuf(""), _outBuf("")
{}

Client::~Client()
{
    if (_fd >= 0)
    {
        close(_fd);
        _fd = -1;
    }
}

int				Client::getFd() const { return _fd; }
const			std::string &Client::getHost() const { return _host; }
const			std::string &Client::getNick() const { return _nick; }
const			std::string &Client::getUser() const { return _user; }
Client::State	Client::getState() const { return _state; }

void			Client::setNick(const std::string &nick) { _nick = nick; }
void			Client::setUser(const std::string &user) { _user = user; }
void			Client::setState(State state) { _state = state; }

void			Client::appendInput(const char *data, std::size_t n)
{
    _inBuf.append(data, n);
}

bool			Client::popLine(std::string &out)
{
    std::string::size_type pos = _inBuf.find('\n');
    if (pos == std::string::npos)
        return false;

    out = _inBuf.substr(0, pos);

    // remove last \r if present
    if (!out.empty() && out[out.size() - 1] == '\r')
        out.erase(out.size() - 1);

    _inBuf.erase(0, pos + 1);
    return true;
}

void Client::queueOutput(const std::string &line)
{
    _outBuf += line;
}

bool Client::hasPendingOutput(void) const
{
    return !_outBuf.empty();
}

const std::string &Client::outputBuffer() const
{
    return _outBuf;
}

void Client::consumeOutput(std::size_t n)
{
    if (n >= _outBuf.size())
        _outBuf.clear();
    else
        _outBuf.erase(0, n);
}

