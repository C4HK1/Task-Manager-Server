#include <boost/mysql/field_view.hpp>
#include <boost/mysql/rows_view.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "data_base_manager.h"

using namespace boost::mysql;

// Object part

data_base_manager::data_base_manager(const std::string &manager_login, const std::string &manager_password, const std::string &host, const std::string &port, const std::string &profilename, const std::string &password, const std::string &db) : ssl_ctx(boost::asio::ssl::context(boost::asio::ssl::context::tls_client)),
                                                                                                                                                                          conn(boost::mysql::tcp_ssl_connection(ctx.get_executor(), ssl_ctx)),
                                                                                                                                                                          resolver(boost::asio::ip::tcp::resolver(ctx.get_executor()))
{
    auto endpoints = resolver.resolve(host, port);

    boost::mysql::handshake_params params(
        profilename,
        password,
        db);

    conn.connect(*endpoints.begin(), params);

    this->update_tables();

    try {
        conn.start_query("SELECT * FROM profiles WHERE login = '" + manager_login + "'AND password = '" + manager_password + "'", state);

        auto profile = this->convert_data_base_response_to_matrix(conn.read_some_rows(state));

        if (profile.size()) {
            this->manager.ID = std::to_string(profile.at(0).at(0).get_uint64());
            this->manager.name = profile.at(0).at(1).get_string();
            this->manager.login = profile.at(0).at(2).get_string();
            this->manager.password = profile.at(0).at(3).get_string();
        }
    } catch(boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;
    }

    // drop_table("profiles");
    // drop_table("rooms");
    // drop_table("profile_room");
    // drop_table("tasks");
}

data_base_manager::~data_base_manager()
{
    conn.close();
}

//Table creation part

auto data_base_manager::create_profiles_table() -> void
{
    conn.query(
        R"%(
            CREATE TABLE profiles (
                ID INT PRIMARY KEY AUTO_INCREMENT NOT NULL UNIQUE,
                name varchar(50) UNIQUE,
                login varchar(50) NOT NULL UNIQUE,
                password varchar(50) NOT NULL
            )
        )%", result);
}

auto data_base_manager::create_rooms_table() -> void
{
    conn.query(
        R"%(
            CREATE TABLE rooms (
                ID INT PRIMARY KEY AUTO_INCREMENT NOT NULL UNIQUE,
                creatorID INT,
                label varchar(50) NOT NULL,
                FOREIGN KEY (creatorID) REFERENCES profiles(ID) ON DELETE NO ACTION
            )
        )%", result);
}

auto data_base_manager::create_profile_room_table() -> void
{
    conn.query(
        R"%(
            CREATE TABLE profile_room (
                profileID INT NOT NULL,
                roomID INT  NOT NULL,
                PRIMARY KEY (profileID , roomID),
                FOREIGN KEY (profileID) REFERENCES profiles(ID) ON DELETE CASCADE,
                FOREIGN KEY (roomID) REFERENCES rooms(ID) ON DELETE CASCADE
            )
        )%", result);
}

auto data_base_manager::create_tasks_table() -> void
{
    conn.query(
        R"%(
            CREATE TABLE tasks (
                ID INT PRIMARY KEY AUTO_INCREMENT NOT NULL UNIQUE,
                roomID INT NOT NULL,
                label varchar(50) NOT NULL,
                creatorID INT,
                FOREIGN KEY (creatorID) REFERENCES profiles(ID) ON DELETE SET NULL,
                FOREIGN KEY (roomID) REFERENCES rooms(ID) ON DELETE CASCADE
            )
        )%", result);
}

// Profile part

auto data_base_manager::create_profile(const std::string &name, const std::string &login, const std::string &password, profile &result_profile) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        conn.start_query("SELECT * FROM profiles WHERE login = '" + this->manager.login + "'", state);

        auto profiles = this->convert_data_base_response_to_matrix(conn.read_some_rows(state));

        if (profiles.size())
            return DATA_BASE_PROFILE_WITH_SUCH_LOGIN_IS_ALREADY_EXIST;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }

    try {
        conn.start_query("SELECT * FROM profiles WHERE name = '" + name + "'", state);

        auto profiles = this->convert_data_base_response_to_matrix(conn.read_some_rows(state));

        if (profiles.size())
            return DATA_BASE_PROFILE_WITH_SUCH_NAME_IS_ALREADY_EXIST;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }

    conn.query("INSERT INTO profiles (name, login, password) VALUES ('" + name +  "', '" + this->manager.login + "', '" + this->manager.password + "')", result);
    
    return this->get_profile_by_name(name, result_profile);
}

auto data_base_manager::get_profile_by_name(const std::string &name, profile &result_profile) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        conn.start_query("SELECT * FROM profiles WHERE name = '" + name + "'", state);

        auto profile = this->convert_data_base_response_to_matrix(conn.read_some_rows(state));

        if (profile.size()) {
            result_profile.ID = std::to_string(profile.at(0).at(0).get_uint64());
            result_profile.name =profile.at(0).at(1).get_string();
            result_profile.login = profile.at(0).at(2).get_string();
            result_profile.password = profile.at(0).at(3).get_string();
            
            return DATA_BASE_COMPLETED_SUCCESSFULY;
        }
        
        return DATA_BASE_THERE_IS_NO_PROFILE_WITH_SUCH_NAME;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_profile_by_ID(const std::string &ID, profile &result_profile) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        conn.start_query("SELECT * FROM profiles WHERE ID = '" + ID + "'", state);

        auto profile = this->convert_data_base_response_to_matrix(conn.read_some_rows(state));

        if (profile.size()) {
            result_profile.ID = std::to_string(profile.at(0).at(0).get_uint64());
            result_profile.name =profile.at(0).at(1).get_string();
            result_profile.login = profile.at(0).at(2).get_string();
            result_profile.password = profile.at(0).at(3).get_string();
            
            return DATA_BASE_COMPLETED_SUCCESSFULY;
        }

        return DATA_BASE_THERE_IS_NO_PROFILE_WITH_SUCH_ID;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::delete_profile() -> DATA_BASE_EXECUTION_STATUS
{
    try {
        conn.start_query("DELETE FROM profiles WHERE login = '" + this->manager.login + "'AND password = '" + this->manager.login + "'", state);
    
        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::login_in_profile() -> DATA_BASE_EXECUTION_STATUS
{
    try {      
        conn.start_query("SELECT * FROM profiles WHERE login = '" + this->manager.login + "' AND password = '" + this->manager.password + "'", state);

        auto profile = this->convert_data_base_response_to_matrix(conn.read_some_rows(state));

        if (profile.size())
            return DATA_BASE_COMPLETED_SUCCESSFULY;

        return DATA_BASE_THERE_IS_NO_PROFILE_WITH_SUCH_LOGIN_AND_PASSWORD;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::find_profiles_with_prefix_in_name(const std::string &prefix, std::vector<profile> &result_profiles) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        conn.start_query("SELECT * FROM profiles WHERE login LIKE '" + prefix + "%'", state);

        auto profiles = this->convert_data_base_response_to_matrix(conn.read_some_rows(state));

        for (auto profile : profiles) {
            struct profile curr_profile;

            curr_profile.ID = std::to_string(profile.at(0).get_uint64());
            curr_profile.name = profile.at(1).get_string();
            curr_profile.login = profile.at(2).get_string();
            curr_profile.password = profile.at(3).get_string();

            result_profiles.push_back(curr_profile);
        }

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

// Room part

auto data_base_manager::create_room(const std::string &room_label, room &result_room) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        conn.start_query("SELECT * FROM rooms WHERE creatorID = '" + this->manager.ID + "' AND label = '" + room_label + "'", state);

        auto rooms = this->convert_data_base_response_to_matrix(conn.read_some_rows(state));

        if (rooms.size())
            return DATA_BASE_ROOM_WITH_SUCH_PARAMETERS_IS_ALREADY_EXIST;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }

    conn.query("INSERT INTO rooms (creatorID, label) VALUES ('" + this->manager.ID + "', '" + room_label + "')", result);
    
    auto status = this->get_room(this->manager.ID, room_label, result_room);

    if (status)
        return status;

    conn.query("INSERT INTO profile_room (profileID, roomID) VALUES ('" + this->manager.ID + "', '" + result_room.ID + "')", result);

    return DATA_BASE_COMPLETED_SUCCESSFULY;
}

// auto data_base_manager::get_room_ID(const std::string &creator_ID, const std::string &label) -> std::string
// {
//     try
//     {
//         conn.start_query("SELECT ID FROM rooms WHERE creatorID = '" + creator_ID + "' AND label = '" + label + "'", state);

//         auto result = this->convert_data_base_response_to_matrix(conn.read_some_rows(state);

//         return result.size() ? std::to_string(result.at(0).at(0).get_int64()) : "0";
//     }
//     catch (boost::mysql::error_with_diagnostics &exception)
//     {
//         return "0";
//     }
// }

auto data_base_manager::get_room(const std::string &room_creator_ID, const std::string &room_label, room &result_room) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        conn.start_query("SELECT * FROM rooms WHERE creatorID = '" + room_creator_ID + "' AND label = '" + room_label + "'", state);

        std::vector<std::vector<field>> room = this->convert_data_base_response_to_matrix(conn.read_some_rows(state));

        if (room.size()) {
            profile creator_profile;
            std::vector<task> tasks;

            this->get_profile_by_ID(room_creator_ID, creator_profile);
            this->get_room_tasks(room_creator_ID, room_label, tasks);
            
            result_room.ID = std::to_string(room.at(0).at(0).get_int64());
            result_room.creator_id = room.at(0).at(1).get_string();
            result_room.creator_name = creator_profile.name;
            result_room.label = room.at(0).at(2).get_string();
            result_room.tasks = tasks;

            return DATA_BASE_COMPLETED_SUCCESSFULY;
        }

        return DATA_BASE_THERE_IS_NO_ROOM_WITH_SUCH_PARAMETERS;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message();
        std::cout << exception.get_diagnostics().client_message();

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::delete_room(const std::string &room_creator_ID, const std::string &room_label) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        conn.start_query("DELETE FROM rooms WHERE creatorID = '" + room_creator_ID + "'AND label = '" + room_label + "'", state);
        
        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::append_member_to_room(const std::string &member_ID, const std::string &creator_ID, const std::string &room_label) -> DATA_BASE_EXECUTION_STATUS
{
    room room;

    auto status = this->get_room(creator_ID, room_label, room);

    if (status)
        return  status;

    try {
        conn.start_query("SELECT * FROM profile_room WHERE creatorID = '" + member_ID + "' AND label = '" + room.ID + "'", state);

        auto profile_room = this->convert_data_base_response_to_matrix(conn.read_some_rows(state));

        if (profile_room.size())
            return DATA_BASE_THIS_PROFILE_IS_ALREADY_EXIST_IN_THIS_ROOM;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }

    conn.query("INSERT INTO profile_room (profileID, roomID) VALUES ('" + member_ID + "', '" + room.ID + "')", result);

    return DATA_BASE_COMPLETED_SUCCESSFULY;
}

// Task part

auto data_base_manager::create_task(const std::string &room_creator_ID, const std::string &room_label, const std::string &task_label, task &result_task) -> DATA_BASE_EXECUTION_STATUS
{
    room room;
    
    auto status = this->get_room(room_creator_ID, room_label, room);

    if (status)
        return status;

    try {
        conn.start_query("SELECT * FROM tasks WHERE roomID = '" + room.ID + "' AND label = '" + task_label + "'", state);
        
        auto tasks = this->convert_data_base_response_to_matrix(conn.read_some_rows(state));

        if (tasks.size())
            return DATA_BASE_TASK_WITH_SUCH_PARAMETERS_IS_ALREADY_EXIST;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }

    conn.query("INSERT INTO tasks (roomID, label, creatorID) VALUES ('" + room.ID + "', '" + task_label + "', '" + this->manager.ID + "')", result);

    return this->get_task(room_creator_ID, room_label, task_label, result_task);
}

// auto data_base_manager::get_task_ID(const std::string &room_ID, const std::string &label) -> std::string
// {
//     try
//     {
//         conn.start_query("SELECT ID FROM tasks WHERE roomID = '" + room_ID + "' AND label = '" + label + "'", state);

//         auto result = this->convert_data_base_response_to_matrix(conn.read_some_rows(state);

//         return result.size() ? std::to_string(result.at(0).at(0).get_int64()) : "0";
//     }
//     catch (boost::mysql::error_with_diagnostics &exception)
//     {
//         return "0";
//     }
// }

auto data_base_manager::get_task(const std::string &room_creator_ID, const std::string &room_label, const std::string &task_label, task &result_task) -> DATA_BASE_EXECUTION_STATUS
{
    room room;
    
    auto status = this->get_room(room_creator_ID, room_label, room);

    if (status)
        return status;

    try {
        conn.start_query("SELECT * FROM tasks WHERE roomID = '" + room.ID + "' AND label = '" + task_label + "'", state);

        auto task = this->convert_data_base_response_to_matrix(conn.read_some_rows(state));

        if (task.size()) {
            profile task_creator;

            status = this->get_profile_by_ID(std::to_string(task.at(0).at(3).get_uint64()), task_creator);

            if (status)
                return status;
            
            result_task.ID = std::to_string(task.at(0).at(0).get_uint64());
            result_task.room_id = room.ID;
            result_task.label = task.at(0).at(2).get_string();
            result_task.creator_id = std::to_string(task.at(0).at(3).get_uint64());
            result_task.creator_name = task_creator.name;

            return DATA_BASE_COMPLETED_SUCCESSFULY;
        }
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }

    return DATA_BASE_THERE_IS_NO_TASK_WITH_SUCH_PARAMETERS;
}


auto data_base_manager::delete_task(const std::string &room_creator_ID, const std::string &room_label, const std::string &task_label) -> DATA_BASE_EXECUTION_STATUS
{
    room room;
    
    auto status = this->get_room(room_creator_ID, room_label, room);

    if (status) {
        return status;
    }

    try {
        conn.start_query("SELECT FROM tasks WHERE roomID = '" + room.ID + "'AND label = '" + task_label + "'", state);

        auto task = this->convert_data_base_response_to_matrix(conn.read_some_rows(state));

        if (!task.size())
            return DATA_BASE_THERE_IS_NO_TASK_WITH_SUCH_PARAMETERS;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }

    try {
        conn.start_query("DELETE FROM tasks WHERE roomID = '" + room.ID + "'AND label = '" + task_label + "'", state);
        
        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

// Join part

auto data_base_manager::get_profile_rooms(std::vector<room> &result_rooms) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        conn.start_query("SELECT * FROM profile_room INNER JOIN rooms ON profile_room.roomID = rooms.ID AND profile_room.profileID = '" + manager.ID + "'", state);

        auto rooms = this->convert_data_base_response_to_matrix(conn.read_some_rows(state));

        for (auto room : rooms) {
            struct profile room_creator;

            auto status = this->get_profile_by_name(std::to_string(room.at(3).get_int64()), room_creator);

            if (status)
                return status;

            struct room curr_room;

            curr_room.ID = std::to_string(room.at(2).get_int64());
            curr_room.creator_id = std::to_string(room.at(3).get_int64());
            curr_room.creator_name = room_creator.name;
            curr_room.label = room_creator.name, room.at(4).get_string();
            
            status = get_room_tasks(curr_room.creator_id, curr_room.label, curr_room.tasks);

            if (status)
                return status;

            result_rooms.push_back(curr_room);
        }

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_profile_tasks(std::vector<task> &result_tasks) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        conn.start_query("SELECT * FROM tasks INNER JOIN (SELECT profile_room.roomID FROM profile_room INNER JOIN rooms ON profile_room.roomID = rooms.ID AND profile_room.profileID = '" + manager.ID + "') t2 ON t2.roomID = tasks.roomID", state);

        auto tasks = this->convert_data_base_response_to_matrix(conn.read_some_rows(state));

        for (auto task : tasks) {
            struct profile task_creator;

            auto status = this->get_profile_by_ID(std::to_string(task.at(3).get_int64()), task_creator);

            if (status)
                return status;

            struct task curr_task;

            curr_task.ID = std::to_string(task.at(0).get_int64());
            curr_task.room_id = std::to_string(task.at(1).get_int64());
            curr_task.label = task.at(2).get_string();
            curr_task.creator_id = std::to_string(task.at(3).get_int64());
            curr_task.creator_name = task_creator.name;

            result_tasks.push_back(curr_task);
        }

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_room_profiles(const std::string &room_creator_ID, const std::string &room_label, std::vector<profile> &result_profiles) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        conn.start_query("SELECT * FROM profile_room INNER JOIN profiles ON profile_room.profileID = profiles.ID", state);

        auto profiles = this->convert_data_base_response_to_matrix(conn.read_some_rows(state));

        for (auto profile : profiles) {
            struct profile curr_profile;

            curr_profile.ID = std::to_string(profile.at(2).get_uint64());
            curr_profile.name = profile.at(3).get_string();
            curr_profile.login = profile.at(4).get_string();
            curr_profile.password = profile.at(5).get_string();
            
            result_profiles.push_back(curr_profile);
        }

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_room_tasks(const std::string &room_creator_ID, const std::string &room_label, std::vector<task> &result_tasks) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        room room;

        auto status = this->get_room(room_creator_ID, room_label, room);

        if (status)
            return status;

        conn.start_query("SELECT * FROM tasks INNER JOIN rooms ON tasks.roomID = rooms.ID AND rooms.ID = '" + room.ID + "'", state);

        auto tasks = this->convert_data_base_response_to_matrix(conn.read_some_rows(state));

        for (auto task : tasks) {
            struct task curr_task;
            
            curr_task.ID = std::to_string(task.at(0).get_int64());
            curr_task.room_id = std::to_string(task.at(1).get_int64());
            curr_task.label = task.at(2).get_string();
            curr_task.creator_id = std::to_string(task.at(3).get_int64());

            struct profile creator;

            status = this->get_profile_by_ID(std::to_string(task.at(3).get_int64()), creator);

            if (status)
                return status;

            curr_task.creator_name = creator.name;
            
            result_tasks.push_back(curr_task);
        }

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

// Data base table management part

auto data_base_manager::convert_data_base_response_to_matrix(const rows_view &rows) -> std::vector<std::vector<boost::mysql::field>>
{
    std::vector<std::vector<field>> matrix;

    for (auto row : rows) {
        matrix.push_back(row.as_vector());
    }

    return matrix;
}

auto data_base_manager::print_tabel(const std::string &name) -> void
{
    try {
        conn.start_query("SELECT * FROM " + name, state);
        auto table = this->convert_data_base_response_to_matrix(conn.read_some_rows(state));

        for (auto row : table) {
            for (auto cell : row) {
                std::cout << cell << " ";
            }
            std::cout << std::endl;
        }
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;
    }
}

auto data_base_manager::drop_table(const std::string &name) -> void
{
    try {
        conn.query("SET FOREIGN_KEY_CHECKS=0", result);
        conn.query("DROP TABLE " + name, result);
        conn.query("SET FOREIGN_KEY_CHECKS=1;", result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;
    }
}

auto data_base_manager::update_tables() -> void
{
    try {
        conn.query("SELECT * FROM profiles", result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        this->create_profiles_table();
    }

    try {
        conn.query("SELECT * FROM rooms", result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        this->create_rooms_table();
    }

    try {
        conn.query("SELECT * FROM profile_room", result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        this->create_profile_room_table();
    }

    try {
        conn.query("SELECT * FROM tasks", result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        this->create_tasks_table();
    }
}