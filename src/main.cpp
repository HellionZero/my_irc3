/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/30 15:17:02 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/26 14:15:54 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"
#include "../include/Dispatcher.hpp"
#include "../include/ServerContext.hpp"

/***
 * global shutdown flag. This variable is set to 1 when the server receives a SIGINT signal (Ctrl+C).
 * The main loop of the server checks this flag to determine when to exit gracefully.
 * this variable is declared as volatile sig_atomic_t to ensure that it can be safely modified in a signal handler.
 * volatile: This tells the compiler that the value of the variable may change at any time,
 * and the compiler should not optimize accesses to this variable.
 */
static volatile sig_atomic_t g_shutdown = 0;

/**
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
		ServerContext context(password);
        Dispatcher dispatcher(context);
        Server server(port, context, dispatcher);
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