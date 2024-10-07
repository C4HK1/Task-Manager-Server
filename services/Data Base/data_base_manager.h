#pragma once

#include <boost/asio/ip/basic_resolver.hpp>
#include <boost/exception/exception.hpp>
#include <boost/mysql/error_code.hpp>
#include <boost/mysql/error_with_diagnostics.hpp>
#include <boost/url.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <boost/format.hpp>
#include <boost/mysql/tcp_ssl.hpp>
#include <stdlib.h>

#include "../../models/Room/room.h"
#include "../../models/Task/task.h"
#include "../../models/Profile/profile.h"

class data_base_manager
{
    boost::asio::io_context ctx;
    boost::asio::ssl::context ssl_ctx;
    boost::mysql::tcp_ssl_connection conn;
    boost::asio::ip::tcp::resolver resolver;
    boost::mysql::results result;
    boost::mysql::execution_state state;
    boost::asio::io_context io_context;
public:
    //Object part
    data_base_manager(const std::string &host = getenv("DATA_BASE_HOST") ? getenv("DATA_BASE_HOST") : "0.0.0.0", const std::string &port = boost::mysql::default_port_string, const std::string &profilename = "myprofile", const std::string &password = "secret", const std::string &db = "mydatabase");
    ~data_base_manager();

    //Profile part
    auto create_profiles_table() -> void;
    auto create_profile(const std::string &login, const std::string &password) -> bool;
    auto get_profile_id(const std::string &login, const std::string &password) -> std::string;
    auto get_profile(const std::string &ID) -> profile;
    auto delete_profile(const std::string &login, const std::string &password) -> void;

    auto login_in_profile(const std::string &login, const std::string &password) -> bool;
    auto find_profiles_with_prefix_in_name(const std::string &prefix) -> std::vector<std::vector<std::string>>;

    //Room part
    auto create_rooms_table() -> void;
    auto create_profile_room_table() -> void;
    auto create_room(const std::string &creator_id, const std::string &label) -> bool;
    auto get_room_id(const std::string &creator_id, const std::string &label) -> std::string;
    auto delete_room(const std::string &creator_id, const std::string &label) -> void;
    
    auto append_member_to_room(const std::string &member_id, const std::string &creator_id, const std::string &label) -> bool;

    //Tasks part
    auto create_tasks_table() -> void;
    auto create_task(const std::string &room_id, const std::string &label, const std::string &creator_id) -> bool;
    auto get_task_id(const std::string &room_id, const std::string &label) -> std::string;
    auto delete_task(const std::string &room_id, const std::string &label) -> bool;

    //Join part
    auto get_profile_rooms(const std::string &profile_id) -> std::vector<room>;
    auto get_profile_tasks(const std::string &profile_id) -> std::vector<task>;
    auto get_room_profiles(const std::string &room_id) -> std::vector<profile>;
    auto get_room_tasks(const std::string &room_id) -> std::vector<task>;

    //Data base table management part
    auto print_tabel(const std::string &name) -> void;
    auto drop_table(const std::string &name) -> void;
    auto update_tables() -> void;
};