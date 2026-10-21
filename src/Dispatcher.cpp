/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Dispatcher.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lsarraci <lsarraci@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/21 18:00:51 by lsarraci          #+#    #+#             */
/*   Updated: 2026/10/21 18:08:23 by lsarraci         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Dispatcher.hpp"

Dispatcher::Dispatcher() {}

Dispatcher::~Dispatcher() {}

std::string Dispatcher::dispatch(Client *client, const std::string &line)
{
	(void)client;
	// For now, just echo the line back to the client.
	// In a real implementation, you would parse the line and execute the appropriate command.
	return line + END_HANDLER;
}