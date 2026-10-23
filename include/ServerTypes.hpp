/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerTypes.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 16:05:27 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/23 16:07:26 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_TYPES_HPP
# define SERVER_TYPES_HPP

#include <map>
#include <string>

class Client;
class Channel;

typedef std::map<int, Client*> 		ClientMap;
typedef std::map<std::string, Channel*>	ChannelMap;

typedef ClientMap::iterator			ClientIt;
typedef ClientMap::const_iterator	ClientConstIt;
		
typedef ChannelMap::iterator		ChannelIt;
typedef ChannelMap::const_iterator	ChannelConstIt;

#endif