/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/30 15:17:02 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/05 18:46:33 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

static volatile sig_atomic_t g_shutdown = 0;

/***
 * Signal handler for SIGINT. This function will be called when the server receives a SIGINT signal
 * (usually triggered by pressing Ctrl+C).
 * The main purpose for this function is to shut down the server gracefully when the user interrupts the program,
 * avoiding abrupt termination and potential resource leaks.
 */
static void handleSigint(int signal)
{
	(void)signal;
	g_shutdown = 1;

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
        server.runServer(g_shutdown);
		std::cout << "\nSIGINT received. Shutting down the server..." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
	return 0;
}