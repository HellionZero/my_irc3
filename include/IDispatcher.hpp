/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IDispatcher.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/05 18:01:13 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/26 13:48:24 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IDISPATCHER_HPP
# define IDISPATCHER_HPP

#include "mini_irc.hpp"
#include "Client.hpp"

/// interface class for the Dispatcher class. The interface is needed to
/// allow for dependency injection and testing of the Dispatcher class.

class IDispatcher
{
	public:
		virtual ~IDispatcher() {}
		virtual std::string dispatch(Client *client, const std::string &line) = 0;
};

#endif
