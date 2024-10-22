#include <boost/mysql/field_view.hpp>
#include <boost/mysql/rows_view.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <vector>

#include "data_base_manager.h"

using namespace boost::mysql;

// Object part

data_base_manager::data_base_manager(const std::string &manager_login, 
                                     const std::string &manager_password, 
                                     const std::string &data_base_host, 
                                     const std::string &data_base_port, 
                                     const std::string &data_base_profile_name, 
                                     const std::string &data_base_password, 
                                     const std::string &data_base) 
    : ssl_ctx(boost::asio::ssl::context(boost::asio::ssl::context::tls_client)),
      conn(boost::mysql::tcp_ssl_connection(ctx.get_executor(), ssl_ctx)),
      resolver(boost::asio::ip::tcp::resolver(ctx.get_executor())) {
    auto endpoints = resolver.resolve(data_base_host, data_base_port);

    boost::mysql::handshake_params params(
        data_base_profile_name,
        data_base_password,
        data_base);

    conn.connect(*endpoints.begin(), params);

    this->update_tables();

    this->manager.login = manager_login;
    this->manager.password = manager_password;

    try {
        std::stringstream request;

        request << "SELECT * FROM profiles WHERE login = '"
                << manager_login
                << "' AND password = '"
                << manager_password
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto profile = this->convert_data_base_response_to_matrix(result.rows());

        if (profile.size()) {
            this->manager.ID = profile.at(0).at(0).get_uint64();
            this->manager.name = profile.at(0).at(1).get_string();
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

data_base_manager::~data_base_manager() {
    conn.close();
}

auto data_base_manager::get_manager() -> profile {
    return this->manager;
}

//Table creation part

auto data_base_manager::create_profiles_table() -> void {
    conn.query(
        R"%(
            CREATE TABLE profiles (
                ID BIGINT UNSIGNED PRIMARY KEY AUTO_INCREMENT NOT NULL UNIQUE,
                name varchar(50) UNIQUE,
                login varchar(50) NOT NULL UNIQUE,
                password varchar(50) NOT NULL
            )
        )%", result);
}

auto data_base_manager::create_rooms_table() -> void {
    conn.query(
        R"%(
            CREATE TABLE rooms (
                ID BIGINT UNSIGNED PRIMARY KEY AUTO_INCREMENT NOT NULL UNIQUE,
                creatorID BIGINT UNSIGNED,
                label varchar(50) NOT NULL,
                FOREIGN KEY (creatorID) REFERENCES profiles(ID)
            )
        )%", result);
}

auto data_base_manager::create_profile_room_table() -> void {
    conn.query(
        R"%(
            CREATE TABLE profile_room (
                profileID BIGINT UNSIGNED NOT NULL,
                roomID BIGINT UNSIGNED NOT NULL,
                PRIMARY KEY (profileID , roomID),
                FOREIGN KEY (profileID) REFERENCES profiles(ID),
                FOREIGN KEY (roomID) REFERENCES rooms(ID) ON DELETE CASCADE
            )
        )%", result);
}

auto data_base_manager::create_tasks_table() -> void {
    conn.query(
        R"%(
            CREATE TABLE tasks (
                ID BIGINT UNSIGNED PRIMARY KEY AUTO_INCREMENT NOT NULL UNIQUE,
                roomID BIGINT UNSIGNED NOT NULL,
                label varchar(50) NOT NULL,
                creatorID BIGINT UNSIGNED,
                FOREIGN KEY (creatorID) REFERENCES profiles(ID) ON DELETE SET NULL,
                FOREIGN KEY (roomID) REFERENCES rooms(ID) ON DELETE CASCADE
            )
        )%", result);
}

// Profile part

auto data_base_manager::create_profile(const std::string &name) -> DATA_BASE_EXECUTION_STATUS {
    try {
        std::stringstream request;

        request << "SELECT * FROM profiles WHERE login = '"
                << this->manager.login
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto profiles_with_manager_login = this->convert_data_base_response_to_matrix(result.rows());

        if (profiles_with_manager_login.size())
            return DATA_BASE_PROFILE_WITH_SUCH_LOGIN_IS_ALREADY_EXIST;

        request.str(std::string());

        request << "SELECT * FROM profiles WHERE name = '"
                << name
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto profiles_with_manager_name = this->convert_data_base_response_to_matrix(result.rows());

        if (profiles_with_manager_name.size())
            return DATA_BASE_PROFILE_WITH_SUCH_NAME_IS_ALREADY_EXIST;

        request.str(std::string());

        request << "INSERT INTO profiles (name, login, password) VALUES ('"
                << name 
                << "', '"
                << this->manager.login
                << "', '"
                << this->manager.password
                << "')";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        request.str(std::string());

        request << "SELECT ID FROM profiles WHERE login = '"
                << this->manager.login
                << "' AND password = '"
                << this->manager.password
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto ID = this->convert_data_base_response_to_matrix(result.rows());

        if (!ID.size())
            return DATA_BASE_THERE_IS_NO_PROFILE_WITH_SUCH_LOGIN_AND_PASSWORD;

        u_int64_t profile_ID = ID.at(0).at(0).as_uint64();

        return this->get_profile_by_ID(profile_ID, this->manager);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_profile_by_ID(const u_int64_t ID, profile &result_profile) -> DATA_BASE_EXECUTION_STATUS {
    try {
        std::stringstream request;

        request << "SELECT * FROM profiles WHERE ID = " 
                << ID;

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto profile = this->convert_data_base_response_to_matrix(result.rows());

        if (profile.size()) {
            result_profile.ID = profile.at(0).at(0).get_uint64();
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
        std::stringstream request;
        
        request << "SELECT * FROM profiles WHERE login = '"
                << this->manager.login
                << "' AND password = '"
                << this->manager.password
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto profile = this->convert_data_base_response_to_matrix(result.rows());

        if (!profile.size())
            return DATA_BASE_THERE_IS_NO_PROFILE_WITH_SUCH_LOGIN_AND_PASSWORD;

        request.str(std::string());

        request << "SET FOREIGN_KEY_CHECKS=0";

        conn.execute(request.str(), result);

        request.str(std::string());

        request << "DELETE FROM profiles WHERE login = '"
                << this->manager.login
                << "' AND password = '"
                << this->manager.password
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        request.str(std::string());

        request << "DELETE FROM profile_room WHERE profileID = '"
                << this->manager.ID
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        request.str(std::string());

        request << "SET FOREIGN_KEY_CHECKS=1";

        conn.execute(request.str(), result);

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
        std::stringstream request;
        
        request << "SELECT * FROM profiles WHERE login = '"
                << this->manager.login
                << "' AND password = '"
                << this->manager.password
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto profile = this->convert_data_base_response_to_matrix(result.rows());

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
        std::stringstream request;
        
        request << "SELECT * FROM profiles WHERE login LIKE '"
                << prefix
                << "'%";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto profiles = this->convert_data_base_response_to_matrix(result.rows());

        for (auto profile : profiles) {
            struct profile curr_profile;

            curr_profile.ID = profile.at(0).get_uint64();
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
        struct profile profile;

        auto status = this->get_profile_by_ID(this->manager.ID, profile);
        
        if (status)
            return status;

        std::stringstream request;

        request << "SELECT * FROM rooms WHERE creatorID = "
                << this->manager.ID
                << " AND label = '"
                << room_label
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto rooms = this->convert_data_base_response_to_matrix(result.rows());

        if (rooms.size())
            return DATA_BASE_ROOM_WITH_SUCH_PARAMETERS_IS_ALREADY_EXIST;

        request.str(std::string());

        request << "INSERT INTO rooms (creatorID, label) VALUES ("
                << this->manager.ID 
                << ", '" 
                << room_label 
                << "')";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);
    
        request.str(std::string());

        request << "SELECT ID FROM rooms WHERE creatorID = "
                << this->manager.ID
                << " AND label = '"
                << room_label
                << "'";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);

        auto ID = this->convert_data_base_response_to_matrix(result.rows());

        if (!ID.size())
            return DATA_BASE_THERE_IS_NO_ROOM_WITH_SUCH_PARAMETERS;

        u_int64_t room_ID = ID.at(0).at(0).as_uint64();

        request.str(std::string());

        request << "INSERT INTO profile_room (profileID, roomID) VALUES ("
                << this->manager.ID
                << ", "
                << room_ID
                << ")";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);

        return this->get_room(this->manager.ID, room_label, result_room);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_room(const u_int64_t room_creator_ID, const std::string &room_label, room &result_room) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        std::stringstream request;

        request << "SELECT * FROM rooms WHERE rooms.creatorID = " 
                << room_creator_ID
                << " AND label = '"
                << room_label
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        std::vector<std::vector<field>> room_with_no_access = this->convert_data_base_response_to_matrix(result.rows());

        request.str(std::string());

        request << "SELECT * FROM (SELECT rooms.* FROM profile_room INNER JOIN rooms ON profile_room.roomID = rooms.ID AND profile_room.profileID = "
                << manager.ID
                << ") rooms_with_access WHERE rooms_with_access.creatorID = " 
                << room_creator_ID
                << " AND label = '"
                << room_label
                << "'";

        conn.execute(request.str(), result);

        std::vector<std::vector<field>> room = this->convert_data_base_response_to_matrix(result.rows());

        if (room.size() != room_with_no_access.size())
            return DATA_BASE_ROOM_ACCESS_ERROR;

        if (room.size()) {
            result_room.ID = room.at(0).at(0).get_uint64();
            result_room.creator_id = room.at(0).at(1).get_uint64();
            result_room.label = room.at(0).at(2).get_string();

            profile creator_profile;
            std::vector<task> tasks;

            auto status = this->get_profile_by_ID(result_room.creator_id, creator_profile);
            
            if (status)
                return status;

            result_room.creator_name = creator_profile.name;

            return DATA_BASE_COMPLETED_SUCCESSFULY;
        }

        return DATA_BASE_THERE_IS_NO_ROOM_WITH_SUCH_PARAMETERS;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message();
        std::cout << exception.get_diagnostics().client_message();

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::delete_room(const u_int64_t room_creator_ID, const std::string &room_label) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        room room;
        
        auto status = this->get_room(room_creator_ID, room_label, room);

        if (status)
            return status;

        std::stringstream request;

        request << "DELETE FROM rooms WHERE creatorID = "
                << room_creator_ID
                << " AND label = '"
                << room_label
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);
        
        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::append_member_to_room(const u_int64_t member_ID, const u_int64_t creator_ID, const std::string &room_label) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        room room;

        auto status = this->get_room(creator_ID, room_label, room);

        if (status)
            return  status;

        std::stringstream request;

        request << "SELECT * FROM profile_room WHERE profileID = "
                << member_ID 
                << " AND roomID = '"
                << room.ID
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto profile_room = this->convert_data_base_response_to_matrix(result.rows());

        if (profile_room.size())
            return DATA_BASE_THIS_PROFILE_IS_ALREADY_EXIST_IN_THIS_ROOM;

        request.str(std::string());

        request << "INSERT INTO profile_room (profileID, roomID) VALUES ("
                << member_ID
                << ", "
                << room.ID
                << ")";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

// Task part

auto data_base_manager::create_task(const u_int64_t room_creator_ID, const std::string &room_label, const std::string &task_label, task &result_task) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        struct profile profile;

        auto status = this->get_profile_by_ID(this->manager.ID, profile);
        
        if (status)
            return status;

        room room;
        
        status = this->get_room(room_creator_ID, room_label, room);

        if (status)
            return status;

        std::stringstream request;

        request << "SELECT * FROM tasks WHERE roomID = "
                << room.ID
                << " AND label = '"
                << task_label
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);
        
        auto tasks = this->convert_data_base_response_to_matrix(result.rows());

        if (tasks.size())
            return DATA_BASE_TASK_WITH_SUCH_PARAMETERS_IS_ALREADY_EXIST;
        
        request.str(std::string());

        request << "INSERT INTO tasks (roomID, label, creatorID) VALUES ("
                << room.ID
                << ", '"
                << task_label
                << "', "
                << this->manager.ID
                << ")";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);

        return this->get_task(room_creator_ID, room_label, task_label, result_task);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_task(const u_int64_t room_creator_ID, const std::string &room_label, const std::string &task_label, task &result_task) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        room room;
    
        auto status = this->get_room(room_creator_ID, room_label, room);

        if (status)
            return status;

        std::stringstream request;

        request << "SELECT * FROM tasks WHERE roomID = "
                << room.ID
                << " AND label = '"
                << task_label
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto task = this->convert_data_base_response_to_matrix(result.rows());

        if (task.size()) {
            result_task.ID = task.at(0).at(0).get_uint64();
            result_task.room_id = task.at(0).at(1).get_uint64();
            result_task.label = task.at(0).at(2).get_string();
            result_task.creator_id = task.at(0).at(3).get_uint64();

            profile task_creator;

            status = this->get_profile_by_ID(result_task.creator_id, task_creator);

            if (status)
                return status;
            
            result_task.creator_name = task_creator.name;

            return DATA_BASE_COMPLETED_SUCCESSFULY;
        }
            
        return DATA_BASE_THERE_IS_NO_TASK_WITH_SUCH_PARAMETERS;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}


auto data_base_manager::delete_task(const u_int64_t room_creator_ID, const std::string &room_label, const std::string &task_label) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        room room;
        
        auto status = this->get_room(room_creator_ID, room_label, room);

        if (status)
            return status;

        std::stringstream request;

        request << "SELECT * FROM tasks WHERE roomID = "
                << room.ID
                << " AND label = '" 
                << task_label
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto task = this->convert_data_base_response_to_matrix(result.rows());

        if (!task.size())
            return DATA_BASE_THERE_IS_NO_TASK_WITH_SUCH_PARAMETERS;

        request.str(std::string());

        request << "DELETE FROM tasks WHERE roomID = "
                << room.ID
                << " AND label = '"
                << task_label
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);
        
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
        std::stringstream request;

        request << "SELECT rooms.* FROM profile_room INNER JOIN rooms ON profile_room.roomID = rooms.ID AND profile_room.profileID = "
                << manager.ID;
                
        conn.execute(request.str(), result);

        auto rooms = this->convert_data_base_response_to_matrix(result.rows());

        for (auto room : rooms) {
            struct room curr_room;

            curr_room.ID = room.at(0).get_uint64();
            curr_room.creator_id = room.at(1).get_uint64();
            curr_room.label = room.at(2).get_string();

            struct profile room_creator;

            auto status = this->get_profile_by_ID(curr_room.creator_id, room_creator);

            if (status)
                return status;

            curr_room.creator_name = room_creator.name;

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
        std::stringstream request;

        request << "SELECT tasks.* FROM tasks INNER JOIN (SELECT profile_room.roomID FROM profile_room INNER JOIN rooms ON profile_room.roomID = rooms.ID AND profile_room.profileID = "
                << manager.ID
                << ") rooms_with_access ON rooms_with_access.roomID = tasks.roomID";

        conn.execute(request.str(), result);

        auto tasks = this->convert_data_base_response_to_matrix(result.rows());

        for (auto task : tasks) {
            struct task curr_task;

            curr_task.ID = task.at(0).get_uint64();
            curr_task.room_id = task.at(1).get_uint64();
            curr_task.label = task.at(2).get_string();
            curr_task.creator_id = task.at(3).get_uint64();

            struct profile task_creator;

            auto status = this->get_profile_by_ID(curr_task.creator_id, task_creator);

            if (status)
                return status;

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

auto data_base_manager::get_room_profiles(const u_int64_t room_creator_ID, const std::string &room_label, std::vector<profile> &result_profiles) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        struct room room;

        auto status = this->get_room(room_creator_ID, room_label, room);

        if (status)
            return status;

        std::stringstream request;

        request << "SELECT profiles.* FROM (SELECT * FROM profile_room WHERE profileID = "
                << this->manager.ID
                << ") profile_room_with_access INNER JOIN profiles ON profile_room_with_access.profileID = profiles.ID AND profile_room_with_access.roomID = "
                << room.ID;

        conn.execute(request.str(), result);

        auto profiles = this->convert_data_base_response_to_matrix(result.rows());

        for (auto profile : profiles) {
            struct profile curr_profile;

            curr_profile.ID = profile.at(0).get_uint64();
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

auto data_base_manager::get_room_tasks(const u_int64_t room_creator_ID, const std::string &room_label, std::vector<task> &result_tasks) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        room room;

        auto status = this->get_room(room_creator_ID, room_label, room);

        if (status)
            return status;

        std::stringstream request;

        request << "SELECT tasks.* FROM tasks INNER JOIN (SELECT rooms.ID FROM profile_room INNER JOIN rooms ON profile_room.roomID = rooms.ID AND profile_room.profileID = "
                << manager.ID
                << ") rooms_with_access ON tasks.roomID = rooms_with_access.ID AND rooms_with_access.ID = "
                << room.ID;

        conn.execute(request.str(), result);

        auto tasks = this->convert_data_base_response_to_matrix(result.rows());

        for (auto task : tasks) {
            struct task curr_task;
            
            curr_task.ID = task.at(0).get_uint64();
            curr_task.room_id = task.at(1).get_uint64();
            curr_task.label = task.at(2).get_string();
            curr_task.creator_id = task.at(3).get_uint64();

            struct profile task_creator;

            status = this->get_profile_by_ID(curr_task.creator_id, task_creator);

            if (status)
                return status;

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
        std::stringstream request;

        request << "SELECT * FROM "
                << name;

        conn.execute(request.str(), result);
        auto table = this->convert_data_base_response_to_matrix(result.rows());

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
        std::stringstream request;

        request << "SET FOREIGN_KEY_CHECKS=0";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);

        request.str(std::string());

        request <<" DROP TABLE "
                << name;

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);

        request.str(std::string());

        request << "SET FOREIGN_KEY_CHECKS=1";

        conn.query(request.str(), result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;
    }
}

auto data_base_manager::update_tables() -> void
{
    try {
        std::stringstream request;

        request << "SELECT * FROM profiles";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        this->create_profiles_table();
    }

    try {
        std::stringstream request;

        request << "SELECT * FROM rooms";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        this->create_rooms_table();
    }

    try {
        std::stringstream request;

        request << "SELECT * FROM profile_room";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        this->create_profile_room_table();
    }

    try {
        std::stringstream request;

        request << "SELECT * FROM tasks";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        this->create_tasks_table();
    }
}