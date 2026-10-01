/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/01 17:16:55 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/01 17:20:49 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Parser.hpp"

Parser::Command Parser::parse(const std::string &line)
{
	Command command;
	size_t pos = 0;

	if (line.empty())
		throw ParseException();

	// Check for prefix
	if (!line.empty() && line[0] == ':')
	{
		pos = line.find(' ');
		if (pos == std::string::npos || pos == 1)
			throw ParseException();
		command._prefix = line.substr(1, pos - 1);
		while (pos < line.size() && line[pos] == ' ')
			++pos;
	}
	if (pos == line.size())
		throw ParseException();

	// Extract command name
	size_t endPos = line.find(' ', pos);
	if (endPos != std::string::npos)
	{
		command._name = line.substr(pos, endPos - pos);
		if (command._name.empty())
			throw ParseException();
		pos = endPos + 1;
	}
	else
	{
		command._name = line.substr(pos);
		if (command._name.empty())
			throw ParseException();
		command._isValid = true;
		return command;
	}

	// Extract parameters
	while (pos < line.size())
	{
		while (pos < line.size() && line[pos] == ' ')
			++pos;
		if (pos == line.size())
			break;
		if (line[pos] == ':') // Trailing parameter
		{
			command._params.push_back(line.substr(pos + 1));
			break;
		}
		endPos = line.find(' ', pos);
		if (endPos != std::string::npos)
		{
			command._params.push_back(line.substr(pos, endPos - pos));
			pos = endPos + 1;
		}
		else
		{
			command._params.push_back(line.substr(pos));
			break;
		}
	}

	if (command._name.empty())
		throw ParseException();
	command._isValid = true;
	return command;
}