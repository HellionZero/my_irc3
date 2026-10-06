/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/30 15:12:34 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/05 17:48:24 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <cstddef>

class Client
{
	public:
		/***
		 * enum to represent the state of the client connection.
		 */
		enum State
		{
			HANDSHAKE,
			CONNECTED,
			REGISTERED,
			DISCONNECTING,
		};
		
		Client(int socketFd, const std::string &hostname);
		~Client();
		
		/* identity */
		int                 getFd(void) const;
		const std::string  &getHost(void) const;

		/* information */
		// getters and setters
		const std::string  &getNick(void) const;
		const std::string  &getUser(void) const;
		State               getState(void) const;

		void setNick(const std::string &nick);
		void setUser(const std::string &user);
		void setState(State state);

		/* input */

		void appendInput(const char *data, std::size_t n);
		std::size_t inputBufferSize(void) const;
		bool hasCompleteLine(void) const;
		bool popLine(std::string &line);

		/* output */

		bool queueOutput(const std::string &line);
		bool hasPendingOutput(void) const;
		const std::string  &outputBuffer(void) const;
		void consumeOutput(std::size_t n);

	private:
		int          _fd;
		std::string  _host;
		std::string  _nick;
		std::string  _user;
		State        _state;
		std::string  _inBuf;
		std::string  _outBuf;
};

#endif