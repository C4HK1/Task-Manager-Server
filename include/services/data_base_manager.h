#pragma once

#include <boost/mysql/tcp_ssl.hpp>
#include <sys/types.h>

class profile;
class room; 
class task; 
class config; 
class timer; 
class invite; 

enum DATA_BASE_EXECUTION_STATUS : u_int64_t {
    //Global part
    DATA_BASE_COMPLETED_SUCCESSFULY,
    DATA_BASE_FAILED,

    //Profile part
    DATA_BASE_PROFILE_WITH_SUCH_LOGIN_IS_ALREADY_EXIST,
    DATA_BASE_PROFILE_WITH_SUCH_NAME_IS_ALREADY_EXIST,
    DATA_BASE_THERE_IS_NO_PROFILE_WITH_SUCH_NAME,
    DATA_BASE_THERE_IS_NO_PROFILE_WITH_SUCH_ID,
    DATA_BASE_THERE_IS_NO_PROFILE_WITH_SUCH_LOGIN_AND_PASSWORD,
    DATA_BASE_THERE_IS_NO_CONFIG_WITH_SUCH_PROFILE_ID,
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
    DATA_BASE_THIS_PROFILE_IS_ALREADY_ASSIGNEE_TO_THIS_TASK,
    DATA_BASE_THIS_ID_IS_NOT_ASSIGNEE_THIS_TASK,
    DATA_BASE_THIS_PROFILE_IS_ALREADY_REVIEW_THIS_TASK,
    DATA_BASE_THIS_ID_IS_NOT_REVIEW_THIS_TASK,
    
    //Invite part
    DATA_BASE_INVITE_ALREADY_SENDED,
    DATA_BASE_THERE_IS_NO_INVITES_WITH_SUCH_PARAMETERS,
    DATA_BASE_THETE_IS_NO_ACCESS_FOR_THIS_INVITE,

    //Join part
};

constexpr size_t DATA_BASE_SHIFT{6};

constexpr char DEFAULT_MANAGER_LOGIN[]{};
constexpr char DEFAULT_MANAGER_PASSWORD[]{};
constexpr char ENV_DATA_BASE_HOST[]{"DATA_BASE_HOST"};
constexpr char DEFAULT_DATA_BASE_HOST[]{"0.0.0.0"};
constexpr char DEFAULT_DATA_BASE_PROFILE_NAME[]{"myuser"};
constexpr char DEFAULT_DATA_BASE_PASSWORD[]{"secret"};
constexpr char DEFAULT_DATA_BASE[]{"mydatabase"};

class data_base_manager {
    boost::asio::io_context ctx;
    boost::asio::ssl::context ssl_ctx;
    boost::mysql::tcp_ssl_connection conn;
    boost::asio::ip::tcp::resolver resolver;
    boost::mysql::results result;
    boost::mysql::execution_state state;
    boost::asio::io_context io_context;

    profile *manager;

    //Table part
    auto create_profiles_table() -> void;
    auto create_rooms_table() -> void;
    auto create_profile_room_table() -> void;
    auto create_tasks_table() -> void;
    auto create_configs_table() -> void;
    auto create_assignees_table() -> void;
    auto create_reviewers_table() -> void;
    auto create_invites_table() -> void;

    auto clean_room_table() -> void;

    //Data base table management part
    auto convert_data_base_response_to_matrix(const boost::mysql::rows_view &rows) -> std::vector<std::vector<boost::mysql::field>>;
    
    auto drop_table(const std::string &name) -> void;
    
    auto update_tables() -> void;

public:
    //Object part
    data_base_manager(
            const std::string &manager_login = DEFAULT_MANAGER_LOGIN,
            const std::string &manager_password = DEFAULT_MANAGER_PASSWORD, 
            const std::string &data_base_host = getenv(ENV_DATA_BASE_HOST) ? getenv(ENV_DATA_BASE_HOST) : DEFAULT_DATA_BASE_HOST, 
            const std::string &data_base_port = boost::mysql::default_port_string,
            const std::string &data_base_profile_name = DEFAULT_DATA_BASE_PROFILE_NAME, 
            const std::string &data_base_password = DEFAULT_DATA_BASE_PASSWORD,
            const std::string &data_base = DEFAULT_DATA_BASE);
    
    ~data_base_manager();

    auto get_manager() -> profile;

    //Profile part
    auto create_profile(
            const std::string &name,
            const std::string &login,
            const std::string &password,
            const std::string &email,
            const std::string &phone,
            profile &result_profile) -> DATA_BASE_EXECUTION_STATUS;

    auto update_profile_config(const char *avatar, const char *configuration) -> DATA_BASE_EXECUTION_STATUS;

    auto get_profile_by_ID(
            const u_int64_t profile_ID, 
            profile &result_profile) -> DATA_BASE_EXECUTION_STATUS;

    auto get_profile_config(config &result_config) -> DATA_BASE_EXECUTION_STATUS;
        
    auto get_profile_assigned_tasks(std::vector<task> &result_tasks) -> DATA_BASE_EXECUTION_STATUS;

    auto get_profile_reviewed_tasks(std::vector<task> &result_tasks) -> DATA_BASE_EXECUTION_STATUS;
    
    auto get_profiles_with_substr_in_name(
            const std::string &prefix,
            const u_int64_t offset,
            std::vector<profile> &result_profiles) -> DATA_BASE_EXECUTION_STATUS;

    auto get_profile_received_invites(std::vector<invite> &result_invites) -> DATA_BASE_EXECUTION_STATUS;
    
    auto get_profile_sended_invites(std::vector<invite> &result_invites) -> DATA_BASE_EXECUTION_STATUS;
       
    auto delete_profile() -> DATA_BASE_EXECUTION_STATUS;

    auto loggin_profile(const std::string &login, const std::string &password) -> DATA_BASE_EXECUTION_STATUS;

    auto profile_authenticate() -> DATA_BASE_EXECUTION_STATUS;


    //Room part
    auto create_room(
            const std::string &room_name,
            const std::string &description, 
            room &result_room) -> DATA_BASE_EXECUTION_STATUS;

    auto get_room(
            const u_int64_t room_creator_ID, 
            const std::string &room_name, 
            room &result_room) -> DATA_BASE_EXECUTION_STATUS;

    auto leave_from_room(
            u_int64_t room_creator_ID, 
            std::string room_name) -> DATA_BASE_EXECUTION_STATUS;

    auto delete_room(
            const u_int64_t room_creator_ID, 
            const std::string &room_name) -> DATA_BASE_EXECUTION_STATUS;

    //Task part
    auto create_task(
            const u_int64_t room_creator_ID, 
            const std::string &room_name,
            const std::string &task_name,
            const std::string &description,
            const std::string &label,
            const u_int64_t status,
            const time_t &time_to_live,
            task &result_task) -> DATA_BASE_EXECUTION_STATUS;

    auto add_task_to_assignee(
            const u_int64_t room_creator_ID,
            const std::string &room_name,
            const std::string &task_name,
            const u_int64_t assignee_ID) -> DATA_BASE_EXECUTION_STATUS;
    
    auto add_task_to_reviewer(
            const u_int64_t room_creator_ID,
            const std::string &room_name,
            const std::string &task_name,
            const u_int64_t reviewer_ID) -> DATA_BASE_EXECUTION_STATUS;
    
    auto get_task(
            const u_int64_t room_creator_ID,
            const std::string &room_name, 
            const std::string &task_name, 
            task &result_task) -> DATA_BASE_EXECUTION_STATUS;

    auto get_task_assignees(
            const u_int64_t room_creator_ID,
            const std::string &room_name, 
            const std::string &task_name, 
            std::vector<profile> &result_assignees) -> DATA_BASE_EXECUTION_STATUS;

    auto get_task_reviewers(
            const u_int64_t room_creator_ID,
            const std::string &room_name, 
            const std::string &task_name, 
            std::vector<profile> &result_reviewers) -> DATA_BASE_EXECUTION_STATUS;

    auto delete_task(
            const u_int64_t room_creator_ID, 
            const std::string &room_name, 
            const std::string &task_name) -> DATA_BASE_EXECUTION_STATUS;
    
    auto remove_task_from_assignee(
            const u_int64_t room_creator_ID,
            const std::string &room_name,
            const std::string &task_name,
            const u_int64_t assignee_ID) -> DATA_BASE_EXECUTION_STATUS;
    
    auto remove_task_from_reviewer(
            const u_int64_t room_creator_ID,
            const std::string &room_name,
            const std::string &task_name,
            const u_int64_t reviewer_ID) -> DATA_BASE_EXECUTION_STATUS;

    //Invites part
    auto create_invite(
            const u_int64_t receiver_ID, 
            const u_int64_t room_creator_ID, 
            const std::string room_name,
            invite &result_invite) -> DATA_BASE_EXECUTION_STATUS;

    auto get_invite(
            const u_int64_t sender_ID, 
            const u_int64_t receiver_ID, 
            const u_int64_t room_creator_ID, 
            const std::string room_name,
            invite &result_invite) -> DATA_BASE_EXECUTION_STATUS;
    
    auto accept_invite(
            u_int64_t room_creator_ID, 
            std::string room_name,
            invite &accepted_invite) -> DATA_BASE_EXECUTION_STATUS;

    auto delete_sended_invite(
            u_int64_t receiver_ID, 
            u_int64_t room_creator_ID, 
            std::string room_name,
            invite &deleted_invite) -> DATA_BASE_EXECUTION_STATUS;

    auto delete_received_invite(
            u_int64_t sender_ID, 
            u_int64_t room_creator_ID, 
            std::string room_name,
            invite &deleted_invite) -> DATA_BASE_EXECUTION_STATUS;
        
    auto delete_invite(
            u_int64_t receiver_ID, 
            u_int64_t sender_ID, 
            u_int64_t room_creator_ID, 
            std::string room_name,
            invite &deleted_invite) -> DATA_BASE_EXECUTION_STATUS;


    //Join part
    auto get_profile_rooms(std::vector<room> &result_rooms) -> DATA_BASE_EXECUTION_STATUS;
    
    auto get_profile_tasks(std::vector<task> &result_tasks) -> DATA_BASE_EXECUTION_STATUS;
    
    auto get_room_profiles(
            const u_int64_t room_creator_ID,
            const std::string &room_name, 
            std::vector<profile> &result_profiles) -> DATA_BASE_EXECUTION_STATUS;
    
    auto get_room_tasks(
            const u_int64_t room_creator_ID,
            const std::string &room_name, 
            std::vector<task> &result_tasks) -> DATA_BASE_EXECUTION_STATUS;

    //Data base table management part
    auto print_tabel(const std::string &name) -> void;

    auto drop_tables() -> void;
};