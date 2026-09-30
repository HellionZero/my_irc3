/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/30 15:17:02 by lsarraci          #+#    #+#             */
/*   Updated: 2026/09/30 16:47:10 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/mini_irc.hpp"

static void handleSigint(int signal)
{
	(void)signal;
	std::cout << "\nServer shutting down...\n";
	exit(0);
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return 1;
	}

	int port = std::atoi(argv[1]);
	std::string password = argv[2];

	if (port == 0)
    {
        std::cerr << "Invalid port\n";
        return 1;
    }

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, handleSigint);

    try
    {
        Server server(port, password);
        server.runServer();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
	return 0;
}