/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/01 16:53:44 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/01 16:55:01 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Channel.hpp"

Channel::Channel(const std::string &name)
    : _name(name), _topic(""), _members(), _operators()
{
}

Channel::~Channel() {}

const std::string   &Channel::getName() const      { return _name; }
const std::string   &Channel::getTopic() const     { return _topic; }
const std::set<int> &Channel::getMembers() const   { return _members; }
const std::set<int> &Channel::getOperators() const { return _operators; }

void Channel::setTopic(const std::string &topic) { _topic = topic; }

void Channel::addMember(int fd)
{
    _members.insert(fd);
}

void Channel::removeMember(int fd)
{
    _members.erase(fd);
    _operators.erase(fd);
}

bool Channel::isMember(int fd) const
{
    return _members.find(fd) != _members.end();
}

void Channel::addOperator(int fd)
{
    if (isMember(fd) && !isOperator(fd))
        _operators.insert(fd);
}

void Channel::removeOperator(int fd)
{
	if (isOperator(fd))
    	_operators.erase(fd);
}

bool Channel::isOperator(int fd) const
{
    return _operators.find(fd) != _operators.end();
}

bool Channel::isEmpty() const
{
    return _members.empty();
}