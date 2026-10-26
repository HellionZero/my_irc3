/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmd_defines.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/21 18:06:04 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/26 13:54:54 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CMD_DEFINES_HPP
# define CMD_DEFINES_HPP

/***
 * @brief Defines codes for exit status and conventions needed in the IRC protocol.
 * @details This header file contains the definitions for various error codes and
 * reply codes used in the IRC protocol.
 */

# define END_HANDLER "\r\n"

# define RPL_WELCOME 001
# define ERR_UNKNOWNCOMMAND 421
# define ERR_NONICKNAMEGIVEN 431
# define ERR_ERRONEUSNICKNAME 432
# define ERR_NICKNAMEINUSE 433
# define ERR_NOTREGISTERED 451
# define ERR_NEEDMOREPARAMS 461
# define ERR_ALREADYREGISTRED 462
# define ERR_PASSWDMISMATCH 464


#endif