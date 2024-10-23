#include <boost/mysql/datetime.hpp>
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
    try {
        conn.query(
        R"%(
            CREATE TABLE profiles (
                ID BIGINT UNSIGNED PRIMARY KEY AUTO_INCREMENT NOT NULL UNIQUE,
                name VARCHAR(50) UNIQUE,
                login VARCHAR(50) NOT NULL UNIQUE,
                password VARCHAR(50) NOT NULL,

                email VARCHAR(50) NOT NULL,
                phone VARCHAR(50)
            )
        )%", result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;
    }
}

auto data_base_manager::create_rooms_table() -> void {
    try {
        conn.query(
        R"%(
            CREATE TABLE rooms (
                creator_ID BIGINT UNSIGNED NOT NULL,
                name VARCHAR(50) NOT NULL,

                description VARCHAR(50) NOT NULL,

                PRIMARY KEY (creator_ID , name)
            )
        )%", result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;
    }
}

auto data_base_manager::create_profile_room_table() -> void {
    try {
        conn.query(
        R"%(
            CREATE TABLE profile_room (
                profile_ID BIGINT UNSIGNED NOT NULL,
                room_creator_ID BIGINT UNSIGNED NOT NULL,
                room_name VARCHAR(50) NOT NULL,

                PRIMARY KEY (profile_ID, room_creator_ID, room_name),
                FOREIGN KEY (profile_ID) REFERENCES profiles(ID) ON DELETE CASCADE,
                FOREIGN KEY (room_creator_ID, room_name) REFERENCES rooms(creator_ID, name) ON DELETE CASCADE
            )
        )%", result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;
    }
}

auto data_base_manager::create_tasks_table() -> void {
    try {
        conn.query(
        R"%(
            CREATE TABLE tasks (
                room_creator_ID BIGINT UNSIGNED NOT NULL,
                room_name VARCHAR(50) NOT NULL,
                creator_ID BIGINT UNSIGNED,
                name VARCHAR(50) NOT NULL,

                label VARCHAR(50),
                status BIGINT UNSIGNED,

                creation_time DATETIME,
                deadline DATETIME,

                PRIMARY KEY (room_creator_ID, room_name, name),
                FOREIGN KEY (creator_ID) REFERENCES profiles(ID) ON DELETE SET NULL,
                FOREIGN KEY (room_creator_ID, room_name) REFERENCES rooms(creator_ID, name) ON DELETE CASCADE
            )
        )%", result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;
    }
}

auto data_base_manager::create_configs_table() -> void {
    try {
        conn.query(
        R"%(
            CREATE TABLE configs (
                profile_ID BIGINT UNSIGNED PRIMARY KEY AUTO_INCREMENT NOT NULL UNIQUE,
                avatar VARBINARY(1024),
                configuration VARBINARY(4096),

                FOREIGN KEY (profile_ID) REFERENCES profiles(ID) ON DELETE CASCADE
            )
        )%", result);
    }  catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;
    }
}

auto data_base_manager::create_assigneers_table() -> void {
    try {
        conn.query(
        R"%(
            CREATE TABLE assigneers (
                assignee_ID BIGINT UNSIGNED NOT NULL,
                room_creator_ID BIGINT UNSIGNED NOT NULL,
                room_name VARCHAR(50) NOT NULL,
                task_name VARCHAR(50) NOT NULL,

                PRIMARY KEY (assignee_ID, room_creator_ID, room_name, task_name),
                FOREIGN KEY (assignee_ID) REFERENCES profiles(ID),
                FOREIGN KEY (room_creator_ID, room_name, task_name) REFERENCES tasks(room_creator_ID, room_name, name) ON DELETE CASCADE
            )
        )%", result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;
    }
}

auto data_base_manager::create_viewers_table() -> void {
    try {
        conn.query(
        R"%(
            CREATE TABLE viewers (
                viewer_ID BIGINT UNSIGNED NOT NULL,
                room_creator_ID BIGINT UNSIGNED NOT NULL,
                room_name VARCHAR(50) NOT NULL,
                task_name VARCHAR(50) NOT NULL,

                PRIMARY KEY (viewer_ID, room_creator_ID, room_name, task_name),
                FOREIGN KEY (viewer_ID) REFERENCES profiles(ID),
                FOREIGN KEY (room_creator_ID, room_name, task_name) REFERENCES tasks(room_creator_ID, room_name, name) ON DELETE CASCADE
            )
        )%", result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;
    }
}

// Profile part

auto data_base_manager::create_profile(const std::string &name,
                                       const std::string &login,
                                       const std::string &password,
                                       const std::string &email,
                                       const std::string &phone,
                                       profile &result_profile) -> DATA_BASE_EXECUTION_STATUS {
    try {
        std::stringstream request;

        request << "SELECT * FROM profiles WHERE login = '"
                << login
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

        request << "INSERT INTO profiles (name, login, password, email, phone) VALUES ('"
                << name 
                << "', '"
                << login
                << "', '"
                << password
                << "', '"
                << email
                << "', '"
                << phone
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

        auto status = this->get_profile_by_ID(profile_ID, this->manager);

        result_profile = this->manager;

        return status;
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
            result_profile.email = profile.at(0).at(4).get_string();
            result_profile.phone = profile.at(0).at(5).get_string();
            
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

        request << "DELETE FROM profiles WHERE login = '"
                << this->manager.login
                << "' AND password = '"
                << this->manager.password
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
            curr_profile.email = profile.at(4).get_string();
            curr_profile.phone = profile.at(5).get_string();

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

auto data_base_manager::create_room(const std::string &room_name, const std::string &description, room &result_room) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        struct profile profile;

        auto status = this->get_profile_by_ID(this->manager.ID, profile);
        
        if (status)
            return status;

        std::stringstream request;

        request << "SELECT * FROM rooms WHERE creator_ID = "
                << this->manager.ID
                << " AND name = '"
                << room_name
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto rooms = this->convert_data_base_response_to_matrix(result.rows());

        if (rooms.size())
            return DATA_BASE_ROOM_WITH_SUCH_PARAMETERS_IS_ALREADY_EXIST;

        request.str(std::string());

        request << "INSERT INTO rooms (creator_ID, name, description) VALUES ("
                << this->manager.ID 
                << ", '" 
                << room_name
                << "', '" 
                << description
                << "')";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);
    
        request.str(std::string());

        request << "INSERT INTO profile_room (profile_ID, room_creator_ID, room_name) VALUES ("
                << this->manager.ID
                << ", "
                << this->manager.ID
                << ", '"
                << room_name
                << "')";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);

        return this->get_room(this->manager.ID, room_name, result_room);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_room(const u_int64_t room_creator_ID, const std::string &room_name, room &result_room) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        std::stringstream request;

        request << "SELECT * FROM rooms WHERE rooms.creator_ID = " 
                << room_creator_ID
                << " AND name = '"
                << room_name
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        std::vector<std::vector<field>> room_with_no_access = this->convert_data_base_response_to_matrix(result.rows());

        request.str(std::string());

        request << "SELECT * FROM (SELECT rooms.* FROM profile_room INNER JOIN rooms ON profile_room.room_creator_ID = rooms.creator_ID AND profile_room.room_name = rooms.name AND profile_room.profile_ID = "
                << manager.ID
                << ") rooms_with_access WHERE rooms_with_access.creator_ID = " 
                << room_creator_ID
                << " AND rooms_with_access.name = '"
                << room_name
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        std::vector<std::vector<field>> room = this->convert_data_base_response_to_matrix(result.rows());

        if (room.size() != room_with_no_access.size())
            return DATA_BASE_ROOM_ACCESS_ERROR;

        if (room.size()) {
            result_room.creator_ID = room.at(0).at(0).get_uint64();
            result_room.name = room.at(0).at(1).get_string();
            result_room.description = room.at(0).at(2).get_string();

            profile creator_profile;
            std::vector<task> tasks;

            auto status = this->get_profile_by_ID(result_room.creator_ID, creator_profile);
            
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

auto data_base_manager::delete_room(const u_int64_t room_creator_ID, const std::string &room_name) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        room room;
        
        auto status = this->get_room(room_creator_ID, room_name, room);

        if (status)
            return status;

        std::stringstream request;

        request << "DELETE FROM rooms WHERE creator_ID = "
                << room_creator_ID
                << " AND name = '"
                << room_name
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

auto data_base_manager::append_member_to_room(const u_int64_t member_ID, const u_int64_t creator_ID, const std::string &room_name) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        profile member;

        auto status = this->get_profile_by_ID(member_ID, member);

        if (status)
            return  status;

        room room;

        status = this->get_room(creator_ID, room_name, room);

        if (status)
            return  status;

        std::stringstream request;

        request << "SELECT * FROM profile_room WHERE profile_ID = "
                << member.ID 
                << " AND room_creator_ID = "
                << room.creator_ID
                << " AND room_name = '"
                << room.name
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto profile_room = this->convert_data_base_response_to_matrix(result.rows());

        if (profile_room.size())
            return DATA_BASE_THIS_PROFILE_IS_ALREADY_EXIST_IN_THIS_ROOM;

        request.str(std::string());

        request << "INSERT INTO profile_room (profile_ID, room_creator_ID, room_name) VALUES ("
                << member_ID
                << ", "
                << room.creator_ID
                << ", '"
                << room.name
                << "')";

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

auto data_base_manager::create_task(const u_int64_t room_creator_ID, 
                                    const std::string &room_name,
                                    const std::string &task_name,
                                    const std::string label,
                                    const u_int64_t status,
                                    const time_t &time_to_live,
                                    task &result_task) -> DATA_BASE_EXECUTION_STATUS{
    try {
        struct profile profile;

        auto status = this->get_profile_by_ID(this->manager.ID, profile);
        
        if (status)
            return status;

        room room;
        
        status = this->get_room(room_creator_ID, room_name, room);

        if (status)
            return status;

        std::stringstream request;

        request << "SELECT * FROM tasks WHERE room_creator_ID = "
                << room.creator_ID
                << " AND room_name = '"
                << room.name
                << "' AND name = '"
                << task_name
                << "'";

        conn.execute(request.str(), result);
        
        auto tasks = this->convert_data_base_response_to_matrix(result.rows());

        if (tasks.size())
            return DATA_BASE_TASK_WITH_SUCH_PARAMETERS_IS_ALREADY_EXIST;
        
        request.str(std::string());

        boost::mysql::datetime creation_time = boost::mysql::datetime::now();
        
        auto deadline_time_t = std::chrono::system_clock::to_time_t(creation_time.as_time_point() + std::chrono::minutes(time_to_live));

        auto deadline_tm = std::localtime(&deadline_time_t);

        boost::mysql::datetime deadline = {static_cast<uint16_t>(deadline_tm->tm_year),
                                                    static_cast<uint8_t>(deadline_tm->tm_mon),
                                                    static_cast<uint8_t>(deadline_tm->tm_mday),
                                                    static_cast<uint8_t>(deadline_tm->tm_hour),
                                                    static_cast<uint8_t>(deadline_tm->tm_min),
                                                    static_cast<uint8_t>(deadline_tm->tm_sec),
                                                    static_cast<uint32_t>(0)};


        request << "INSERT INTO tasks (room_creator_ID, room_name, creator_ID, name, label, status, creation_time, deadline) VALUES ("
                << room.creator_ID
                << ", '"
                << room.name
                << "', "
                << this->manager.ID
                << ", '"
                << task_name
                << "', '"
                << label
                << "', "
                << status
                << ", '"
                << creation_time
                << "', '"
                << deadline
                << "')";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);

        return this->get_task(room_creator_ID, room_name, task_name, result_task);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_task(const u_int64_t room_creator_ID, const std::string &room_name, const std::string &task_name, task &result_task) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        room room;
    
        auto status = this->get_room(room_creator_ID, room_name, room);

        if (status)
            return status;

        std::stringstream request;

        request << "SELECT * FROM tasks WHERE room_creator_ID = "
                << room.creator_ID
                << " AND room_name = '"
                << room.name
                << "' AND name = '"
                << task_name
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto task = this->convert_data_base_response_to_matrix(result.rows());

        if (task.size()) {
            result_task.room_creator_ID = task.at(0).at(0).get_uint64();
            result_task.room_name = task.at(0).at(1).get_string();
            result_task.creator_ID = task.at(0).at(2).get_uint64();
            result_task.name = task.at(0).at(3).get_string();
            result_task.label = task.at(0).at(4).get_string();
            result_task.status = task.at(0).at(5).get_uint64();
            result_task.creation_time = task.at(0).at(6).get_datetime();
            result_task.deadline = task.at(0).at(7).get_datetime();

            profile task_creator;

            status = this->get_profile_by_ID(result_task.creator_ID, task_creator);

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


auto data_base_manager::delete_task(const u_int64_t room_creator_ID, const std::string &room_name, const std::string &task_name) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        room room;
        
        auto status = this->get_room(room_creator_ID, room_name, room);

        if (status)
            return status;

        std::stringstream request;

        request << "SELECT * FROM tasks WHERE room_creator_ID = "
                << room.creator_ID
                << " AND room_name = '" 
                << room.name
                << "' AND name = '" 
                << task_name
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto task = this->convert_data_base_response_to_matrix(result.rows());

        if (!task.size())
            return DATA_BASE_THERE_IS_NO_TASK_WITH_SUCH_PARAMETERS;

        request.str(std::string());

        request << "DELETE FROM tasks WHERE room_creator_ID = "
                << room.creator_ID
                << " AND room_name = '"
                << room.name
                << "' AND name = '"
                << task_name
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

        request << "SELECT rooms.* FROM profile_room INNER JOIN rooms ON profile_room.room_creator_ID = rooms.creator_ID AND profile_room.room_name = rooms.name AND profile_room.profile_ID = "
                << manager.ID;
                
        conn.execute(request.str(), result);

        auto rooms = this->convert_data_base_response_to_matrix(result.rows());

        for (auto room : rooms) {
            struct room curr_room;

            curr_room.creator_ID = room.at(0).get_uint64();
            curr_room.name = room.at(1).get_string();
            curr_room.description = room.at(2).get_string();

            struct profile room_creator;

            auto status = this->get_profile_by_ID(curr_room.creator_ID, room_creator);

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

        request << "SELECT tasks.* FROM tasks INNER JOIN (SELECT profile_room.room_creator_ID, profile_room.room_name FROM profile_room INNER JOIN rooms ON profile_room.room_creator_ID = rooms.creator_ID AND profile_room.room_name = rooms.name AND profile_room.profile_ID = "
                << manager.ID
                << ") rooms_with_access ON rooms_with_access.room_creator_ID = tasks.room_creator_ID AND rooms_with_access.room_name = tasks.room_name";

        conn.execute(request.str(), result);

        auto tasks = this->convert_data_base_response_to_matrix(result.rows());

        for (auto task : tasks) {
            struct task curr_task;

            curr_task.room_creator_ID = task.at(0).get_uint64();
            curr_task.room_name = task.at(1).get_string();
            curr_task.creator_ID = task.at(2).get_uint64();
            curr_task.name = task.at(3).get_string();
            curr_task.label = task.at(4).get_string();
            curr_task.status = task.at(5).get_uint64();
            curr_task.creation_time = task.at(6).get_datetime();
            curr_task.deadline = task.at(7).get_datetime();

            struct profile task_creator;

            auto status = this->get_profile_by_ID(curr_task.creator_ID, task_creator);

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

auto data_base_manager::get_room_profiles(const u_int64_t room_creator_ID, const std::string &room_name, std::vector<profile> &result_profiles) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        struct room room;

        auto status = this->get_room(room_creator_ID, room_name, room);

        if (status)
            return status;

        std::stringstream request;

        request << "SELECT profiles.* FROM (SELECT * FROM profile_room WHERE profile_ID = "
                << this->manager.ID
                << ") profile_room_with_access INNER JOIN profiles ON profile_room_with_access.profile_ID = profiles.ID AND profile_room_with_access.room_creator_ID = "
                << room.creator_ID
                << " AND profile_room_with_access.room_name = '"
                << room.name
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto profiles = this->convert_data_base_response_to_matrix(result.rows());

        for (auto profile : profiles) {
            struct profile curr_profile;

            curr_profile.ID = profile.at(0).get_uint64();
            curr_profile.name = profile.at(1).get_string();
            curr_profile.login = profile.at(2).get_string();
            curr_profile.password = profile.at(3).get_string();
            curr_profile.email = profile.at(4).get_string();
            curr_profile.phone = profile.at(5).get_string();
            
            result_profiles.push_back(curr_profile);
        }

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_room_tasks(const u_int64_t room_creator_ID, const std::string &room_name, std::vector<task> &result_tasks) -> DATA_BASE_EXECUTION_STATUS
{
    try {
        room room;

        auto status = this->get_room(room_creator_ID, room_name, room);

        if (status)
            return status;

        std::stringstream request;

        request << "SELECT tasks.* FROM tasks INNER JOIN (SELECT rooms.creator_ID, rooms.name FROM profile_room INNER JOIN rooms ON profile_room.room_creator_ID = rooms.creator_ID AND profile_room.room_name = rooms.name AND profile_room.profile_ID = "
                << manager.ID
                << ") rooms_with_access ON tasks.room_creator_ID = rooms_with_access.creator_ID AND tasks.room_name = rooms_with_access.name AND rooms_with_access.creator_ID = "
                << room.creator_ID
                << " AND rooms_with_access.name = '"
                << room.name
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto tasks = this->convert_data_base_response_to_matrix(result.rows());

        for (auto task : tasks) {
            struct task curr_task;
            
            curr_task.room_creator_ID = task.at(0).get_uint64();
            curr_task.room_name = task.at(1).get_string();
            curr_task.creator_ID = task.at(2).get_uint64();
            curr_task.name = task.at(3).get_string();
            curr_task.label = task.at(4).get_string();
            curr_task.status = task.at(5).get_uint64();
            curr_task.creation_time = task.at(6).get_datetime();
            curr_task.deadline = task.at(7).get_datetime();

            struct profile task_creator;

            status = this->get_profile_by_ID(curr_task.creator_ID, task_creator);

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

    try {
        std::stringstream request;

        request << "SELECT * FROM tasks";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        this->create_tasks_table();
    }

    try {
        std::stringstream request;

        request << "SELECT * FROM configs";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        this->create_configs_table();
    }

    try {
        std::stringstream request;

        request << "SELECT * FROM assigneers";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        this->create_assigneers_table();
    }

    try {
        std::stringstream request;

        request << "SELECT * FROM viewers";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        this->create_viewers_table();
    }
}