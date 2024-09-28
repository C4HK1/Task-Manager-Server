#include <boost/beast/core/string_type.hpp>
#include <boost/beast/http/verb.hpp>
#include <cstddef>
#include <jwt/jwt.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <iostream>

#include "http_connection.h"
#include "../Data Base/data_base_manager.h"

http_connection::http_connection(tcp::socket socket) : socket_(std::move(socket)) {}

// Initiate the asynchronous operations associated with the connection.
void http_connection::start()
{
    read_request();
    check_deadline();
}

// Asynchronously receive a complete request message.
void http_connection::read_request()
{
    auto self = shared_from_this();

    http::async_read(
        socket_,
        buffer_,
        request_,
        [self](beast::error_code ec,
            std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);
            if(!ec)
                self->process_request();
        });
}

// Determine what needs to be done with the request message.
void http_connection::process_request()
{
    response_.version(request_.version());
    response_.keep_alive(false);
    response_.result(http::status::ok);
    response_.set(http::field::server, "Beast");
    response_.set(http::field::content_type, "text/plain");

    switch(request_.method())
    {
    case http::verb::get:
        get_request_handler();
        break;
    case http::verb::post:
        post_request_handler();
        break;
    default:
        // We return responses indicating an error if
        // we do not recognize the request method.
        response_.result(http::status::bad_request);
        response_.set(http::field::content_type, "text/plain");
        
        beast::ostream(response_.body())
            << "Invalid request-method '"
            << std::string(request_.method_string())
            << "'";
        break;
    }

    write_response();
}

// Construct a response message based on the program state.
void http_connection::get_request_handler()
{
    data_base_manager data_base;
    auto userData = jwt.validate_jwt_token(request_).payload().create_json_obj();

    if(request_.target().find("/") == 0)
    {
        if (!userData.empty() && std::atoi(std::string(userData.at("destroy_time")).c_str()) > time(NULL)) {
            beast::ostream(response_.body()) << R"%({"authorizetion info": "true"})%";
        } else {
            beast::ostream(response_.body()) << R"%({"authorizetion info": "false"})%";
        } 
    }
    else if (request_.target().find("/ProfileDeleting") == 0)
    {
        std::string login = userData.at("login");
        std::string password = userData.at("password");

        data_base.delete_profile(login, password);
    }
    else
    {
        response_.result(http::status::not_found);
        response_.set(http::field::content_type, "text/plain");
        beast::ostream(response_.body()) << "File not found\r\n";
    }
}

// Construct a response message based on the program state.
void http_connection::post_request_handler()
{
    data_base_manager data_base;
    nlohmann::json authorizetion_data = nlohmann::json::parse(beast::buffers_to_string(request_.body().data()));

    if(request_.target().find("/ProfileCreation") == 0)
    {
        std::string login = authorizetion_data.at("login");
        std::string password = authorizetion_data.at("password");

        beast::ostream(response_.body()) << R"%({"JWT": ")%" + (data_base.create_profile(login, password) ? jwt.create_jwt(login, password, 60 * 60 * 24 * 7) : "") + R"%("})%";
    }
    else if(request_.target().find("/") == 0)
    {
        std::string login = authorizetion_data.at("login");
        std::string password = authorizetion_data.at("password");
        beast::ostream(response_.body()) << R"%({"JWT": ")%" + (data_base.login_in_profile(login, password) ? jwt.create_jwt(login, password, 60 * 60 * 24 * 7) : "") + R"%("})%";
    }
    else
    {
        response_.result(http::status::not_found);
        response_.set(http::field::content_type, "text/plain");
        beast::ostream(response_.body()) << "File not found\r\n";
    }
}

// Asynchronously transmit the response message.
void http_connection::write_response()
{
    auto self = shared_from_this();

    response_.content_length(response_.body().size());

    http::async_write(
        socket_,
        response_,
        [self](beast::error_code ec, std::size_t)
        {
            self->socket_.shutdown(tcp::socket::shutdown_send, ec);
            self->deadline_.cancel();
        });
}

// Check whether we have spent enough time on this connection.
void http_connection::check_deadline()
{
    auto self = shared_from_this();

    deadline_.async_wait(
        [self](beast::error_code ec)
        {
            if(!ec)
            {
                // Close socket to cancel any outstanding operation.
                self->socket_.close(ec);
            }
        });
}