/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IServerContext.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/23 16:14:09 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/23 16:19:28 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ISERVERCONTEXT_HPP
# define ISERVERCONTEXT_HPP

# include "mini_irc.hpp"
# include "Client.hpp"

class IServerContext
{
	public:
		virtual ~IServerContext() {}
		
		virtual const std::string &getPass(void) const = 0;
		virtual Client *findClientByNick(const std::string &nick) = 0;
		virtual bool isNickInUse(const std::string &nick) const = 0;
};

#endif