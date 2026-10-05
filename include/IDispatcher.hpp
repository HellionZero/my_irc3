/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IDispatcher.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/05 18:01:13 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/05 18:09:41 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IDISPATCHER_HPP
# define IDISPATCHER_HPP

#include "mini_irc.hpp"
#include "Client.hpp"

class IDispatcher
{
	public:
		virtual ~IDispatcher() {}
		virtual std::string dispatch(Client *client, const std::string &line) = 0;
};

#endif
