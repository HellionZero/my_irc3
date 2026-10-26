/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Dispatcher.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/21 17:55:59 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/26 14:21:16 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DISPATCHER_HPP
# define DISPATCHER_HPP

# include "mini_irc.hpp"
# include "IDispatcher.hpp"
# include "IServerContext.hpp"
# include "Parser.hpp"
# include "ServerTypes.hpp"

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
		Dispatcher(IServerContext &context);
		virtual ~Dispatcher();

		/***
		 * @brief Dispatches a command line to the appropriate handler.
		 * @param client The client that sent the command.
		 * @param line The command line.
		 * @return The response string.
		 */
		virtual std::string dispatch(Client *client, const std::string &line);
	private:
		IServerContext &_context;

		/**
		 * @brief Handles the PASS command.
		 * 
		 * This function checks if the client is already registered, 
		 * if the command has the correct number of parameters, and
		 * if the provided password matches the server's password.
		 * If any of these checks fail, it returns the appropriate
		 * error message.
		 * @param client The client that sent the command.
		 * @param command The parsed command.
		 * @return The response string.
		 */
		std::string handlePass(Client *client, const Parser::Command &command);
		std::string handleNick(Client *client, const Parser::Command &command);
		std::string handleUser(Client *client, const Parser::Command &command);
		std::string handleQuit(Client *client, const Parser::Command &command);
		/**
		 * @brief Generates a numeric reply string for the client.
		 * @param client The client to send the reply to.
		 * @param code The numeric code for the reply.
		 * @param message The message for the reply.
		 * @return The generated reply string.
		 */
		std::string makeNumeric(Client *client, int code,
			const std::string &message) const;
		std::string clientName(const Client *client) const;
		/**
		 * @brief Converts a string to uppercase.
		 * 
		 * helper function to convert a string to uppercase, to normalize
		 * command names.
		 * @param value The string to convert.
		 * @return The converted string.
		 */
		std::string upper(const std::string &value) const;
		/**
		 * @brief Attempts to register a client if it has
		 * provided the correct password, nickname, and username.
		 */
		bool 		tryRegister(Client *client) const;
};

#endif