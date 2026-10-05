/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/01 16:43:45 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/01 17:54:30 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "mini_irc.hpp"

class Channel {
	private:
		Channel(const Channel &other); // Copy constructor is private to prevent copying
		Channel &operator=(const Channel &other); // Assignment operator is private to prevent assignment
		
		std::string _name;
		std::string _topic;
		std::set<int> _members; // Set of user IDs in the channel
		std::set<int> _operators; // Set of operator user IDs in the channel
	public:
		/***
		 *  Constructor for the Channel class. it is explicit to prevent implicit conversions from std::string to Channel.
		 *  @param name The name of the channel
		 */
		explicit Channel(const std::string &name);
		~Channel();

		/***
		 * Getters and Setters
		 */
		const std::string &getName() const;
		const std::string &getTopic() const;
		const std::set<int> &getMembers() const;
		const std::set<int> &getOperators() const;

		void setTopic(const std::string &topic);
		/***
		 * control methods for managing members and operators
		 */
		void addMember(int userId);
		void removeMember(int userId);
		bool isMember(int userId) const;

		void addOperator(int userId);
		void removeOperator(int userId);
		bool isOperator(int userId) const;

		bool isEmpty(void) const; // Check if the channel has no members
};

#endif