/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Dispatcher.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/21 17:55:59 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/21 18:19:20 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DISPATCHER_HPP
# define DISPATCHER_HPP

# include "mini_irc.hpp"
# include "IDispatcher.hpp"



/***
 * @addtogroup Dispatcher
 * @brief The Dispatcher class is responsible for dispatching commands to the appropriate handlers.
 * 
 * It is obligatory to implement the IDispatcher interface, which defines the dispatch method that 
 * takes a Client pointer and a command line string as input and returns a response string.
 */
class Dispatcher : public IDispatcher
{
	public:
		Dispatcher();
		virtual ~Dispatcher();

		/***
		 * @brief Dispatches a command line to the appropriate handler.
		 * @param client The client that sent the command.
		 * @param line The command line.
		 * @return The response string.
		 */
		virtual std::string dispatch(Client *client, const std::string &line);
};

#endif