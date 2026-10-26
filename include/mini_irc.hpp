/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_irc.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/09 16:16:19 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/26 14:10:06 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINI_IRC_HPP
# define MINI_IRC_HPP

# include "cmd_defines.hpp"

# define RED "\033[31m"
# define GREEN "\033[32m"
# define YELLOW "\033[33m"
# define BLUE "\033[34m"
# define MAGENTA "\033[35m"
# define CYAN "\033[36m"
# define RESET "\033[0m"
# define BOLD "\033[1m"

# define SERVER_VERSION "mini_irc v3.0"
# define BUFFER_DEFAULT_SIZE 1024
# define MAX_BUFFER_SIZE 65536 /// the maximum size of the buffer for a client, in bytes
# define MAX_LINE_LIMIT 512 /// the maximum length of a line in the IRC protocol, including the trailing CRLF
# define MAX_INPUT_BUFFER 4096 /// the maximum size of the input buffer for a client, in bytes, following the IRC protocol specification
# define MAX_OUTPUT_BUFFER 65536 /// the maximum size of the output buffer for a client, in bytes
# define MAX_CYCLE_LINES 16 /// the maximum number of lines to process from a client's input buffer in a single event loop iteration

# include <iostream>
# include <string>
# include <vector>
# include <map>
# include <set>
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

/***
 * File to store all the necessary includes and enums for the mini IRC server project.
 */

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