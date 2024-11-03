#include <boost/beast/core/string_type.hpp>
#include <boost/beast/http/verb.hpp>
#include <cstddef>
#include <jwt/jwt.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>

#include "server/http_connection.h"
#include "server/request_handler.h"

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
            if (!ec)
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

    request_handler request_handler(&request_, &response_);

    switch (request_.method())
    {
    case http::verb::get:
        request_handler.get_request_handler();
        break;
    case http::verb::post:
        request_handler.post_request_handler();
        break;
    case http::verb::delete_:
        request_handler.delete_request_handler();
        break;
    case http::verb::patch:
        request_handler.patch_request_handler();
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
            if (!ec)
            {
                // Close socket to cancel any outstanding operation.
                self->socket_.close(ec);
            }
        });
}
