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
#include <boost/mysql/field_view.hpp>
#include <boost/mysql/rows_view.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <boost/format.hpp>
#include <boost/mysql/tcp_ssl.hpp>
#include <stdlib.h>
#include <sys/types.h>
#include <utility>

#include "../../models/Room/room.h"
#include "../../models/Task/task.h"
#include "../../models/Profile/profile.h"

enum DATA_BASE_EXECUTION_STATUS {
    //Global part
    DATA_BASE_COMPLETED_SUCCESSFULY,
    DATA_BASE_FAILED,

    //Profile part
    DATA_BASE_PROFILE_WITH_SUCH_LOGIN_IS_ALREADY_EXIST,
    DATA_BASE_PROFILE_WITH_SUCH_NAME_IS_ALREADY_EXIST,
    DATA_BASE_THERE_IS_NO_PROFILE_WITH_SUCH_NAME,
    DATA_BASE_THERE_IS_NO_PROFILE_WITH_SUCH_ID,
    DATA_BASE_THERE_IS_NO_PROFILE_WITH_SUCH_LOGIN_AND_PASSWORD,
    DATA_BASE_UNKNOWN_PROFILE,
    
    //Room part
    DATA_BASE_ROOM_CREATION_FAILED,
    DATA_BASE_THERE_IS_NO_ROOM_WITH_SUCH_PARAMETERS,
    DATA_BASE_ROOM_WITH_SUCH_PARAMETERS_IS_ALREADY_EXIST,
    DATA_BASE_THIS_PROFILE_IS_ALREADY_EXIST_IN_THIS_ROOM,
    DATA_BASE_ROOM_ACCESS_ERROR,
    
    //Task part
    DATA_BASE_TASK_CREATION_FAILED,
    DATA_BASE_THERE_IS_NO_TASK_WITH_SUCH_PARAMETERS,
    DATA_BASE_TASK_WITH_SUCH_PARAMETERS_IS_ALREADY_EXIST,
    
    //Join part
};

constexpr size_t DATA_BASE_SHIFT{5};

constexpr char DEFAULT_MANAGER_LOGIN[]{};
constexpr char DEFAULT_MANAGER_PASSWORD[]{};
constexpr char ENV_DATA_BASE_HOST[]{"DATA_BASE_HOST"};
constexpr char DEFAULT_DATA_BASE_HOST[]{"0.0.0.0"};
constexpr char DEFAULT_DATA_BASE_PROFILE_NAME[]{"myuser"};
constexpr char DEFAULT_DATA_BASE_PASSWORD[]{"secret"};
constexpr char DEFAULT_DATA_BASE[]{"mydatabase"};

class data_base_manager
{
    boost::asio::io_context ctx;
    boost::asio::ssl::context ssl_ctx;
    boost::mysql::tcp_ssl_connection conn;
    boost::asio::ip::tcp::resolver resolver;
    boost::mysql::results result;
    boost::mysql::execution_state state;
    boost::asio::io_context io_context;

    profile manager;

    //Table creation part

    auto create_profiles_table() -> void;
    auto create_rooms_table() -> void;
    auto create_profile_room_table() -> void;
    auto create_tasks_table() -> void;

    //Data base table management part

    auto convert_data_base_response_to_matrix(const boost::mysql::rows_view &rows) -> std::vector<std::vector<boost::mysql::field>>;
    auto drop_table(const std::string &name) -> void;
    auto update_tables() -> void;

public:
    //Object part

    data_base_manager(const std::string &manager_login = DEFAULT_MANAGER_LOGIN,
                      const std::string &manager_password = DEFAULT_MANAGER_PASSWORD, 
                      const std::string &data_base_host = getenv(ENV_DATA_BASE_HOST) ? getenv(ENV_DATA_BASE_HOST) : DEFAULT_DATA_BASE_HOST, 
                      const std::string &data_base_port = boost::mysql::default_port_string,
                      const std::string &data_base_profile_name = DEFAULT_DATA_BASE_PROFILE_NAME, 
                      const std::string &data_base_password = DEFAULT_DATA_BASE_PASSWORD,
                      const std::string &data_base = DEFAULT_DATA_BASE);
    ~data_base_manager();

    auto get_manager() -> profile;

    //Profile part

    auto create_profile(const std::string &name) -> DATA_BASE_EXECUTION_STATUS;
    auto get_profile_by_ID(const u_int64_t ID, profile &profile) -> DATA_BASE_EXECUTION_STATUS;
    auto delete_profile() -> DATA_BASE_EXECUTION_STATUS;

    auto login_in_profile() -> DATA_BASE_EXECUTION_STATUS;
    auto find_profiles_with_prefix_in_name(const std::string &prefix, std::vector<profile> &profiles) -> DATA_BASE_EXECUTION_STATUS;

    //Room part

    auto create_room(const std::string &room_label, room &room) -> DATA_BASE_EXECUTION_STATUS;
    auto get_room(const u_int64_t room_creator_ID, const std::string &room_label, room &room) -> DATA_BASE_EXECUTION_STATUS;
    auto delete_room(const u_int64_t room_creator_ID, const std::string &room_label) -> DATA_BASE_EXECUTION_STATUS;
    auto append_member_to_room(const u_int64_t member_ID, u_int64_t room_creator_ID, const std::string &room_label) -> DATA_BASE_EXECUTION_STATUS;

    //Task part

    auto create_task(const u_int64_t room_creator_ID, const std::string &room_label, const std::string &task_label, task &task) -> DATA_BASE_EXECUTION_STATUS;
    auto get_task(const u_int64_t room_creator_ID, const std::string &room_label, const std::string &task_label, task &task) -> DATA_BASE_EXECUTION_STATUS;
    auto delete_task(const u_int64_t room_creator_ID, const std::string &room_label, const std::string &task_label) -> DATA_BASE_EXECUTION_STATUS;

    //Join part

    auto get_profile_rooms(std::vector<room> &rooms) -> DATA_BASE_EXECUTION_STATUS;
    auto get_profile_tasks(std::vector<task> &tasks) -> DATA_BASE_EXECUTION_STATUS;
    auto get_room_profiles(const u_int64_t room_creator_ID, const std::string &room_label, std::vector<profile> &profiles) -> DATA_BASE_EXECUTION_STATUS;
    auto get_room_tasks(const u_int64_t room_creator_ID, const std::string &room_label, std::vector<task> &tasks) -> DATA_BASE_EXECUTION_STATUS;

    //Data base table management part

    auto print_tabel(const std::string &name) -> void;
};