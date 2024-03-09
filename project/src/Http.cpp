/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Http.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kichkiro <kichkiro@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 16:47:13 by kichkiro          #+#    #+#             */
/*   Updated: 2024/03/09 18:16:27 by kichkiro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Http.hpp"

Http::Http(string context) {
    vector<Directive *> value;

    if (context != "main")
        throw WrongContextExc("Http", "main", context);
    this->_type = "http";
    this->_is_context = true;
    this->_value_block = value;
}

Http::Http(ifstream &raw_value, string context) {
    if (context != "main")
        throw WrongContextExc("Http", "main", context);
    this->_type = "http";
    this->_is_context = true;
    this->_parsing_block(raw_value);
}

Http::~Http() {
    for (VecDirIt it = this->_value_block.begin();
        it != this->_value_block.end(); ++it)
        delete *it;
}

void Http::start_servers() {
    int pid;
    int status;
    int num_servers = _value_block.size();

    for (int i = 0; i < num_servers; i++) {
        Server *server = dynamic_cast<Server *>(_value_block[i]);
        pid = fork();

        if (pid == 0) {
            // Bind the socket to the address
            if (Error::checkError(server->bind(), FAILED_BIND))
                Error::exit(FAILED_BIND, *server);

            // Listen for incoming connections
            if (Error::checkError(server->listen(), FAILED_LISTEN))
                Error::exit(FAILED_LISTEN, *server);

            // Set the socket to non-blocking
            if (Error::checkError(server->nonBlocking(), FAILED_NON_BLOCKING))
                Error::exit(FAILED_NON_BLOCKING, *server);

            Log::info("Server is running on port " + server->getPortString());

            // The main loop of the server
            server->run();
            server->close();
            exit(0);
        }
        else if (pid < 0) {
            cerr << "Errore nella creazione del processo" << std::endl;
            exit(1);
        }
    }

    while (waitpid(-1, &status, 0) > 0) {
        continue;
    }
}
