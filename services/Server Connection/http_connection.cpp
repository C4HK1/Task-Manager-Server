
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

    switch (request_.method())
    {
    case http::verb::get:
        get_request_handler();
        break;
    case http::verb::post:
        post_request_handler();
        break;
    case http::verb::delete_:
        delete_request_handler();
        break;
    case http::verb::patch:
        patch_request_handler();
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
    auto request_header_data = jwt.validate_jwt_token(request_).payload().create_json_obj();

    if (request_.target().find("/ProfileLogining") == 0)
    {
        nlohmann::json request_body_data = nlohmann::json::parse(beast::buffers_to_string(request_.body().data()));

        beast::ostream(response_.body()) << R"%({"JWT": ")%" + (data_base.login_in_profile(request_body_data.at("login"), request_body_data.at("password")) ? jwt.create_jwt(request_body_data.at("login"), request_body_data.at("password"), 60 * 60 * 24 * 7) : "") + R"%("})%";
    }
    else if (request_.target().find("/GetProfileInfo") == 0)
    {
        nlohmann::json request_body_data = nlohmann::json::parse(beast::buffers_to_string(request_.body().data()));
    }
    else if (request_.target().find("/GetProfileRooms") == 0)
    {
        auto rooms = data_base.get_profile_rooms(data_base.get_profile_id(request_header_data.at("login"), request_header_data.at("password")));
        
        beast::ostream(response_.body()) << R"%({"profile rooms count": ")%" + std::to_string(rooms.size()) + R"%(",)%" + "\n" +
            R"%("items": [)%" + "\n";

        for (int i = 0; i < rooms.size(); ++i) {
            beast::ostream(response_.body()) << rooms[i].to_json();

            if (i != rooms.size() - 1) {
                beast::ostream(response_.body()) << R"%(,)%" << "\n";
            }
        }

        beast::ostream(response_.body()) << "\n" << R"%(]})%";
            
        std::cout << "get profile rooms\n";
    }
    else if (request_.target().find("/GetProfileTasks") == 0)
    {
        auto tasks = data_base.get_profile_tasks(data_base.get_profile_id(request_header_data.at("login"), request_header_data.at("password")));
        
        beast::ostream(response_.body()) << R"%({"profile tasks count": ")%" + std::to_string(tasks.size()) + R"%(",)%" + "\n" +
            R"%("items": [)%" + "\n";

        for (int i = 0; i < tasks.size(); ++i) {
            beast::ostream(response_.body()) << tasks[i].to_json();

            if (i != tasks.size() - 1) {
                beast::ostream(response_.body()) << R"%(,)%" << "\n";
            }
        }

        beast::ostream(response_.body()) << "\n" << R"%(]})%";

        std::cout << "get profile tasks\n";
    }
    else if (request_.target().find("/GetRoomTasks") == 0)
    {
        nlohmann::json request_body_data = nlohmann::json::parse(beast::buffers_to_string(request_.body().data()));

        auto tasks = data_base.get_room_tasks(data_base.get_room_id(request_body_data.at("room creator id"), request_body_data.at("room label")));
        
        beast::ostream(response_.body()) << R"%({"room tasks count": ")%" + std::to_string(tasks.size()) + R"%(",)%" + "\n" +
            R"%("items": [)%" + "\n";

        for (int i = 0; i < tasks.size(); ++i) {
            beast::ostream(response_.body()) << tasks[i].to_json();

            if (i != tasks.size() - 1) {
                beast::ostream(response_.body()) << R"%(,)%" << "\n";
            }
        }

        beast::ostream(response_.body()) << "\n" << R"%(]})%";

        std::cout << "get room tasks\n";
    }
    else if (request_.target().find("/GetRoomProfiles") == 0)
    {
        nlohmann::json request_body_data = nlohmann::json::parse(beast::buffers_to_string(request_.body().data()));

        auto profiles = data_base.get_room_profiles(data_base.get_room_id(request_body_data.at("room creator id"), request_body_data.at("room label")));
        
        beast::ostream(response_.body()) << R"%({"room profiles count": ")%" + std::to_string(profiles.size()) + R"%(",)%" + "\n" +
            R"%("items": [)%" + "\n";

        for (int i = 0; i < profiles.size(); ++i) {
            beast::ostream(response_.body()) << profiles[i].to_json();

            if (i != profiles.size() - 1) {
                beast::ostream(response_.body()) << R"%(,)%" << "\n";
            }
        }

        beast::ostream(response_.body()) << "\n" << R"%(]})%";

        std::cout << "get room profiles\n";
    }
    else if (request_.target().find("/") == 0)
    {
        beast::ostream(response_.body()) << R"%({"authorizetion info": ")%" << std::boolalpha << (!request_header_data.empty() && std::atoi(std::string(request_header_data.at("destroy_time")).c_str()) > time(NULL) && data_base.login_in_profile(request_header_data.at("login"), request_header_data.at("password"))) << R"%("})%";
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
    auto request_header_data = jwt.validate_jwt_token(request_).payload().create_json_obj();
    nlohmann::json request_body_data = nlohmann::json::parse(beast::buffers_to_string(request_.body().data()));

    if (request_.target().find("/ProfileCreation") == 0)
    {
        std::string name = request_body_data.at("name");
        std::string login = request_body_data.at("login");
        std::string password = request_body_data.at("password");

        beast::ostream(response_.body()) << R"%({"JWT": ")%" + (data_base.create_profile(name, login, password) ? jwt.create_jwt(login, password, 60 * 60 * 24 * 7) : "") + R"%("})%";
    }
    else if (request_.target().find("/RoomCreation") == 0)
    {
        beast::ostream(response_.body()) << R"%({"room creation info": ")%" << std::boolalpha << data_base.create_room(data_base.get_profile_id(request_header_data.at("login"), request_header_data.at("password")), request_body_data.at("room label")) << R"%("})%";
        
        std::cout << "room creating\n";
    }
    else if (request_.target().find("/TaskCreation") == 0)
    {
        beast::ostream(response_.body()) << R"%({"task creation info": ")%" << std::boolalpha << std::to_string(data_base.create_task(data_base.get_room_id(request_body_data.at("room creator id"), request_body_data.at("room label")), request_body_data.at("task label"), data_base.get_profile_id(request_header_data.at("login"), request_header_data.at("password")))) << R"%("})%";

        std::cout << "task creating\n";
    }
    else if (request_.target().find("/RoomDeleting") == 0)
    {
        data_base.delete_room(request_body_data.at("room creator id"), request_body_data.at("room label"));

        std::cout << "room deleting\n";
    }
    else if (request_.target().find("/TaskDeleting") == 0)
    {
        data_base.delete_task(data_base.get_room_id(request_body_data.at("room creator id"), request_body_data.at("room label")), request_body_data.at("task label"));
        
        std::cout << "task deleting\n";
    }
    else
    {
        response_.result(http::status::not_found);
        response_.set(http::field::content_type, "text/plain");
        beast::ostream(response_.body()) << "File not found\r\n";
    }
}

// Construct a response message based on the program state.
void http_connection::delete_request_handler()
{
    data_base_manager data_base;
    auto request_header_data = jwt.validate_jwt_token(request_).payload().create_json_obj();

    if (request_.target().find("/ProfileDeleting") == 0)
    {
        std::string login = request_header_data.at("login");
        std::string password = request_header_data.at("password");

        data_base.delete_profile(login, password);

        beast::ostream(response_.body()) << R"%({"profile deleting info": "profile deleted"})%";
    }
    else
    {
        response_.result(http::status::not_found);
        response_.set(http::field::content_type, "text/plain");
        beast::ostream(response_.body()) << "File not found\r\n";
    }
}

// Construct a response message based on the program state.
void http_connection::patch_request_handler()
{
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
