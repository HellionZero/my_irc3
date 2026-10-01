/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_irc.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 16:16:19 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/01 17:24:21 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINI_IRC_HPP
# define MINI_IRC_HPP

# define RED "\033[31m"
# define GREEN "\033[32m"
# define YELLOW "\033[33m"
# define BLUE "\033[34m"
# define MAGENTA "\033[35m"
# define CYAN "\033[36m"
# define RESET "\033[0m"
# define BOLD "\033[1m"

# include <iostream>
# include <string>
# include <vector>
# include <map>
# include <algorithm>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <poll.h>
# include <netinet/in.h>
# include <unistd.h>
# include <fcntl.h>
# include <cstring>
# include <cstdlib>
# include <csignal>
# include <cerrno>
# include <ctime>
# include <exception>

typedef enum fontSelector
{
	RED_FONT = 1,
	GREEN_FONT,
	YELLOW_FONT,
	BLUE_FONT,
	MAGENTA_FONT,
	CYAN_FONT,
	RESET_FONT,
	BOLD_FONT
	
} t_fontSelector;

typedef enum commandList
{
	COMMAND_PASS = 1,
	COMMAND_NICK,
	COMMAND_USER,
	COMMAND_JOIN,
	COMMAND_PART,
	COMMAND_PING,
	COMMAND_QUIT,

} t_commandList;

#endif