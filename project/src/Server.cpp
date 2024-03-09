/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kichkiro <kichkiro@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/09 16:47:42 by kichkiro          #+#    #+#             */
/*   Updated: 2024/03/09 16:47:53 by kichkiro         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(string context) {
    vector<Directive *> value;

    if (context != "http")
        throw WrongContextExc("Server", "http", context);
    this->_type = "server";
    this->_is_context = true;
    this->_value_block = value;

    // From the configuration file
    this->_port = _get_port();
    this->_maxConnections = _get_maxConnections();
    this->_timeout = (timeval){TIMEOUT_SEC, TIMEOUT_USEC}; // dove si trova nel configfile?

    // Socket information
    this->_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    this->_serverAddress.sin_family = AF_INET;
    this->_serverAddress.sin_addr.s_addr = INADDR_ANY;
    this->_serverAddress.sin_port = htons(_port);
    // this->_clientSocket = _get_clientSocket();
    this->_flags = 0;
}

Server::Server(ifstream &raw_value, string context) {
    if (context != "http")
        throw WrongContextExc("Server", "http", context);
    this->_type = "server";
    this->_is_context = true;
    this->_parsing_block(raw_value);

    // From the configuration file
    this->_port = _get_port();
    this->_maxConnections = _get_maxConnections();
    this->_timeout = (timeval){TIMEOUT_SEC, TIMEOUT_USEC}; // dove si trova nel configfile?

    // Socket information
    this->_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    this->_serverAddress.sin_family = AF_INET;
    this->_serverAddress.sin_addr.s_addr = INADDR_ANY;
    this->_serverAddress.sin_port = htons(_port);
    // this->_clientSocket = _get_clientSocket();
    this->_flags = 0;
}

Server::~Server() {
    for (VecDirIt it = this->_value_block.begin();
        it != this->_value_block.end(); ++it)
        delete *it;
}

int Server::_get_port() {
    for (int i = 0; this->_value_block[i] != this->_value_block.back(); i++) {
        if (this->_value_block[i]->get_type() == "listen")
            return atoi(this->_value_block[i]->get_value_inline()[0].c_str());
    }
    // STAMPARE ERRORE
    return 0;
}

int Server::_get_maxConnections() {
    // provvisorio
    return 10;
}

// timeval Server::_get_timeout() {
//     cout << "trovare il timeout nel configfile" << endl; 
// }


/*!
* @brief
    Bind the socket to the address
* @details
    This function binds the socket to the address using the bind() function from
    the sys/socket.h library.
    - The first argument is the file descriptor of the socket.
    - The second argument is the address information of the server.
    - The third argument is the size of the address information of the server.
* @return
    The result of the bind() function
*/
int Server::bind() {
    memset(_serverAddress.sin_zero, '\0', sizeof _serverAddress.sin_zero);
    return (::bind(_serverSocket, (struct sockaddr *)&_serverAddress, sizeof(_serverAddress)));
}

/*!
* @brief
    Listen for incoming connections
* @param connections
    The maximum length of the queue of pending connections
* @details
    This function listens for incoming connections using the listen() function
    from the sys/socket.h library.
    - The first argument is the file descriptor of the socket.
    - The second argument is the maximum length of the queue of pending
      connections.
* @return
    The result of the listen() function
*/
int Server::listen(int connections) {
    return (::listen(_serverSocket, connections));
}

/*!
* @brief
    Set the socket to non-blocking
* @details
    This function sets the socket to non-blocking using the fcntl() function
    from the fcntl.h library.
    - The first argument is the file descriptor of the socket.
    - The second argument is the command to be executed, in this case, F_SETFL,
      which sets the file status flags to the value specified by the third
      argument.
    - The third argument is the value to be set, in this case, the value of the
      flags attribute OR'd with the O_NONBLOCK macro, which sets the file status
      flags to non-blocking mode.
* @return The result of the fcntl() function
*/
int Server::nonBlocking() {
    return (_flags = fcntl(_serverSocket, F_SETFL, _flags | O_NONBLOCK));
}

/*!
* @brief
    The main loop of the server
* @details
    This function is the main loop of the server, using the select() is possible
    to check if there is any data available to read on the socket without
    blocking.
    The arguments of the select() function are:
    - The range of file descriptors to be checked, set to highest numbered file
      descriptor in any of the three sets, plus 1, to tell the select function
      the effective size of the bit masks it should consider.
    - The file descriptor set to be checked for read activity.
    - The file descriptor set to be checked for write activity.
    - The file descriptor set to be checked for exceptions.
    - The timeout value.
    If the select() function returns a value less than 0, an error occurred, and
    the program exits,
    if the return value is 0, the connection timed out, and the program
    continues, otherwise it accepts a new connection, answers the client, and
    closes the client socket.
* @return
    In case of an error, the exit code of the respective error, otherwise, 0
*/
int Server::run() {
    int r;

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(_serverSocket, &readfds);
        timeval t = _timeout;

        if ((r = select(_serverSocket + 1, &readfds, NULL, NULL, &t) < 0))
            return (Error::exit(FAILED_SELECT, *this));
        else if (r == 0)
            Error::checkError(r, TIMEOUT);

        // If the server socket is not ready, continue
        if (!FD_ISSET(_serverSocket, &readfds))
            continue;

        // Accept a new connection
        if (_accept() < 0)
            return (Error::exit(FAILED_ACCEPT, *this));

        // Get the client request
        _clientRequest();

        // Answer the client
        _answer();

        // Close the client socket
        close();
    }
}

/*!
* @brief
    Get the client request
* @details
    This function gets the client request using the recv() function from the
    sys/socket.h library.
    - The first argument is the file descriptor of the client socket.
    - The second argument is the buffer to store the message.
    - The third argument is the size of the buffer.
* @return
    The result of the recv() function
*/
void Server::_clientRequest() {
    char buffer[1024];
    int bytes = recv(_clientSocket, buffer, 1024, 0);
    buffer[bytes] = '\0';
    Log::request(GET, buffer);
}

/*!
* @brief
    Accept a new connection
* @details
    This function accepts a new connection using the accept() function from the
    sys/socket.h library.
    - The first argument is the file descriptor of the socket.
    - The second argument is the ad dress information of the client.
    - The third argument is the size of the address information of the client.
*/
int Server::_accept() {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    _clientSocket = ::accept(_serverSocket, (sockaddr *)&clientAddr, &clientAddrLen);
    return (_clientSocket);
}

/*!
* @brief
    Answer the client
* @details
    This function answers the client using the send() function from the
    sys/socket.h library.
    - The first argument is the file descriptor of the client socket.
    - The second argument is the message to be sent.
    - The third argument is the size of the message to be sent.
*/
void Server::_answer() {
    string response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<html><body><h1>Hello, World!</h1></body></html>";
    send(_clientSocket, response.c_str(), response.size(), 0);
}

/*!
* @brief
    Close the client socket
* @details
    This function closes the client socket using the close() function from the
    unistd.h library.
*/
void Server::close() {
    if (_clientSocket > 0)
        ::close(_clientSocket);
}

/*!
* @brief
    Get the port number of the server
* @return
    The port number of the server
*/
uint16_t Server::getPort() const {
    return (_port);
}

string Server::getPortString() const {
    stringstream ss;
    ss << _port;
    return (ss.str());
}

/*!
* @brief
    Get the file descriptor of the socket
* @return
    The file descriptor of the socket
*/
int Server::getSocket() const {
    return (_serverSocket);
}

/*!
* @brief
    Get the address information of the server
* @return
    The address information of the server
*/
sockaddr_in Server::getAddress() const {
    return (_serverAddress);
}
