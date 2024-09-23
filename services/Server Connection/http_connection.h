#pragma once

#include <boost/exception/exception.hpp>
#include <boost/url.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <iostream>
#include <jwt/algorithm.hpp>
#include <memory>
#include <openssl/x509.h>
#include <string>
#include <fstream>
#include <jwt/jwt.hpp>

#include "../JWT/JWT_manager.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;  
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class http_connection : public std::enable_shared_from_this<http_connection>
{
public:
    http_connection(tcp::socket socket);
    JWT_manager jwt;

    // Initiate the asynchronous operations associated with the connection.
    void start();

private:
    // The socket for the currently connected client.
    tcp::socket socket_;

    // The buffer for performing reads.
    beast::flat_buffer buffer_{8192};

    // The rquest message.
    http::request<http::dynamic_body> request_;

    // The response message.
    http::response<http::dynamic_body> response_;

    // The timer for putting a deadline on connection processing.
    net::steady_timer deadline_{socket_.get_executor(), std::chrono::seconds(60)};

    // Asynchronously receive a complete request message.
    void read_request();

    // Determine what needs to be done with the request message.
    void process_request();

    // Construct a response message based on the program state.
    void write_response();

    // Asynchronously transmit the response message.
    void get_request_handler();

    // Asynchronously transmit the response message.
    void post_request_handler();

    // Check whether we have spent enough time on this connection.
    void check_deadline();
};
