/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kichkiro <kichkiro@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 16:25:08 by kichkiro          #+#    #+#             */
/*   Updated: 2024/03/09 17:56:08 by kichkiro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.h"

// Class ---------------------------------------------------------------------->

/*!
 * @ref
    Docs:       https://nginx.org/en/docs/http/ngx_http_core_module.html#server
    Syntax:	    server { ... }
    Default:	—
    Context:	http
 */
class Server : public Directive {
    private:
        // From the configuration file
        int _port;
		int _maxConnections;
		timeval _timeout;

		// Socket information
		int _serverSocket;
		sockaddr_in _serverAddress;
		int _clientSocket;
		int _flags;

        // Init attrs
        int _get_port();
        int _get_maxConnections();
        timeval _get_timeout();

        // Server runtime functions
		void _clientRequest();
		int _accept();
		void _answer();

    public:
        Server(string context);
        Server(ifstream &raw_value, string context);
        ~Server();

        // Server setup functions
		int bind();
		int listen(int connections = 10);
		int nonBlocking();

		// Server runtime functions
		int run();
		void close();

		// Getters
		uint16_t getPort() const;
		std::string getPortString() const;
		int getSocket() const;
		sockaddr_in getAddress() const;
};
