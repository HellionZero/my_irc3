/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/01 17:09:09 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/01 17:24:35 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP

# include "mini_irc.hpp"


class Parser
{
	public:
		struct Command
		{
			std::string					_prefix;
			std::string					_name;
			std::vector<std::string>	_params;
			bool						_isValid;
			Command() : _prefix(""), _name(""), _params(), _isValid(false) {}
		};

		class ParseException : public std::exception
		{
			public:
				const char *what() const throw()
				{
					return "ParseException: Invalid command format";
				}
		};
		static Command parse(const std::string &line);
	private:
		/***
		 * Private constructor to prevent instantiation of the Parser class, as it only contains static methods.
		 */
		Parser(void);
		Parser(const Parser &other);
		Parser &operator=(const Parser &other);
		~Parser();
};

#endif