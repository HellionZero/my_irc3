/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerContext.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 16:22:19 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/23 16:23:39 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONTEXT_HPP
# define SERVERCONTEXT_HPP

#include "IServerContext.hpp"
#include "ServerTypes.hpp"

class ServerContext : public IServerContext
{
	public:
		ServerContext(const std::string &password);
		virtual ~ServerContext();

		virtual const std::string &getPass(void) const;
		virtual Client *findClientByNick(const std::string &nick);
		virtual bool isNickInUse(const std::string &nick) const;

		ClientMap &clients(void);
		ChannelMap &channels(void);

	private:
		std::string _password;
		ClientMap _clients;
		ChannelMap _channels;
};

#endif
