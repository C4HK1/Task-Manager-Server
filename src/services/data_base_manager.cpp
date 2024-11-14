#include <boost/mysql/datetime.hpp>
#include <boost/mysql/field_view.hpp>
#include <boost/mysql/rows_view.hpp>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>

#include "models/profile.h" 
#include "models/room.h" 
#include "models/task.h" 
#include "models/config.h" 
#include "models/timer.h" 
#include "models/invite.h" 

#include "services/data_base_manager.h"

using namespace boost::mysql;

// Object part
data_base_manager::data_base_manager(
        const u_int64_t &manager_ID,
        const std::string &manager_login, 
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

    this->manager = new profile;
    this->manager->login = manager_login;
    this->manager->password = manager_password;

    try {
        std::stringstream request;

        request << "SELECT * FROM profiles WHERE ID = "
                << manager_ID
                << " AND login = '"
                << manager_login
                << "' AND password = '"
                << manager_password
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto profile = this->convert_data_base_response_to_matrix(result.rows());

        if (profile.size()) {
            this->manager->ID = profile.at(0).at(0).get_uint64();
            this->manager->name = profile.at(0).at(1).get_string();
            this->manager->email = profile.at(0).at(4).get_string();
            this->manager->phone = profile.at(0).at(5).get_string();
        }
    } catch(boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;
    }
}

data_base_manager::~data_base_manager() {
    this->clean_room_table();

    delete this->manager;
    conn.close();
}

auto data_base_manager::get_manager() -> profile {
    return *this->manager;
}

//Table part
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

                description VARCHAR(1024),

                label VARCHAR(50),
                status BIGINT UNSIGNED,

                creation_time DATETIME,
                deadline DATETIME,

                PRIMARY KEY (room_creator_ID, room_name, name),
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

auto data_base_manager::create_assignees_table() -> void {
    try {
        conn.query(
        R"%(
            CREATE TABLE assignees (
                assignee_ID BIGINT UNSIGNED NOT NULL,
                room_creator_ID BIGINT UNSIGNED NOT NULL,
                room_name VARCHAR(50) NOT NULL,
                task_name VARCHAR(50) NOT NULL,

                PRIMARY KEY (assignee_ID, room_creator_ID, room_name, task_name),
                FOREIGN KEY (assignee_ID) REFERENCES profiles(ID) ON DELETE CASCADE,
                FOREIGN KEY (room_creator_ID, room_name, task_name) REFERENCES tasks(room_creator_ID, room_name, name) ON DELETE CASCADE
            )
        )%", result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;
    }
}

auto data_base_manager::create_reviewers_table() -> void {
    try {
        conn.query(
        R"%(
            CREATE TABLE reviewers (
                reviewer_ID BIGINT UNSIGNED NOT NULL,
                room_creator_ID BIGINT UNSIGNED NOT NULL,
                room_name VARCHAR(50) NOT NULL,
                task_name VARCHAR(50) NOT NULL,

                PRIMARY KEY (reviewer_ID, room_creator_ID, room_name, task_name),
                FOREIGN KEY (reviewer_ID) REFERENCES profiles(ID) ON DELETE CASCADE,
                FOREIGN KEY (room_creator_ID, room_name, task_name) REFERENCES tasks(room_creator_ID, room_name, name) ON DELETE CASCADE
            )
        )%", result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;
    }
}

auto data_base_manager::create_invites_table() -> void {
    try {
        conn.query(
        R"%(
            CREATE TABLE invites (
                sender_ID BIGINT UNSIGNED NOT NULL,
                receiver_ID BIGINT UNSIGNED NOT NULL,
                room_creator_ID BIGINT UNSIGNED NOT NULL,
                room_name VARCHAR(50) NOT NULL,

                PRIMARY KEY (sender_ID, receiver_ID, room_creator_ID, room_name),
                FOREIGN KEY (sender_ID) REFERENCES profiles(ID) ON DELETE CASCADE,
                FOREIGN KEY (receiver_ID) REFERENCES profiles(ID) ON DELETE CASCADE,
                FOREIGN KEY (room_creator_ID, room_name) REFERENCES rooms(creator_ID, name) ON DELETE CASCADE
            )
        )%", result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;
    }
}

auto data_base_manager::clean_room_table() -> void {
    try {
    conn.query(R"%(
            DELETE rooms FROM rooms LEFT JOIN
            (SELECT rooms.creator_ID, rooms.name FROM rooms INNER JOIN
            (SELECT profile_room.room_creator_ID AS profile_room_room_creator_ID, profile_room.room_name AS profile_room_room_name, invites.room_creator_ID AS invites_room_creator_ID, invites.room_name AS invites_room_name 
            FROM profile_room
            LEFT JOIN invites ON profile_room.room_creator_ID = invites.room_creator_ID AND profile_room.room_name = invites.room_name
            UNION ALL
            SELECT profile_room.room_creator_ID AS profile_room_room_creator_ID, profile_room.room_name AS profile_room_room_name, invites.room_creator_ID AS invites_room_creator_ID, invites.room_name AS invites_room_name 
            FROM profile_room
            RIGHT JOIN invites ON profile_room.room_creator_ID = invites.room_creator_ID AND profile_room.room_name = invites.room_name
            WHERE profile_room.room_creator_ID IS NULL AND profile_room.room_name IS NULL) profile_room_invites ON (rooms.creator_ID = profile_room_invites.profile_room_room_creator_ID AND rooms.name = profile_room_invites.profile_room_room_name) OR (rooms.creator_ID = profile_room_invites.invites_room_creator_ID AND rooms.name = profile_room_invites.invites_room_name)) active_rooms
            ON rooms.creator_ID = active_rooms.creator_ID AND rooms.name = active_rooms.name  WHERE active_rooms.creator_ID IS NULL AND active_rooms.name IS NULL
        )%", result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;
    }
}

// Profile part
auto data_base_manager::create_profile(
        const std::string &name,
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
                << login
                << "' AND password = '"
                << password
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto IDs = this->convert_data_base_response_to_matrix(result.rows());

        if (!IDs.size())
            return DATA_BASE_THERE_IS_NO_PROFILE_WITH_SUCH_LOGIN_AND_PASSWORD;

        u_int64_t profile_ID = IDs.at(0).at(0).as_uint64();

        request.str(std::string());

        struct config default_config;

        request << "INSERT INTO configs (profile_ID, avatar, configuration) VALUES ("
                << profile_ID
                << ", '"
                << default_config.avatar
                << "', '"
                << default_config.configuration
                << "')";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto status = this->get_profile_by_ID(profile_ID, *this->manager);

        result_profile = *this->manager;

        return status;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_profile_by_ID(
        const u_int64_t profile_ID, 
        profile &result_profile) -> DATA_BASE_EXECUTION_STATUS {
    try {
        std::stringstream request;

        request << "SELECT * FROM profiles WHERE ID = " 
                << profile_ID;

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto profiles = this->convert_data_base_response_to_matrix(result.rows());

        if (profiles.size()) {
            result_profile = profiles.at(0);
            
            return DATA_BASE_COMPLETED_SUCCESSFULY;
        }

        return DATA_BASE_THERE_IS_NO_PROFILE_WITH_SUCH_ID;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::update_profile_config(const char *avatar, const char *configuration) -> DATA_BASE_EXECUTION_STATUS {
    try {
        struct config config;

        auto status = get_profile_config(config);

        if (status)
            return status;

        std::stringstream request;

        request << "UPDATE configs SET avatar = '"
                << avatar
                << "', configuration = '"
                << configuration
                << "' WHERE profile_ID = "
                << this->manager->ID;

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_profile_config(config &result_config) -> DATA_BASE_EXECUTION_STATUS {
    try {
        struct profile profile;

        auto status = get_profile_by_ID(this->manager->ID, profile);

        if (status)
            return status;

        
        std::stringstream request;

        request << "SELECT * FROM configs WHERE profile_ID = "
                << this->manager->ID;

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto configs = this->convert_data_base_response_to_matrix(result.rows());

        if (configs.size()) {
            result_config = configs.at(0);

            return DATA_BASE_COMPLETED_SUCCESSFULY;
        }

        return DATA_BASE_THERE_IS_NO_CONFIG_WITH_SUCH_PROFILE_ID;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}
        
auto data_base_manager::get_profile_assigned_tasks(std::vector<task> &result_tasks) -> DATA_BASE_EXECUTION_STATUS {
    try {
        struct profile profile;

        auto status = get_profile_by_ID(this->manager->ID, profile);

        if (status)
            return status;

        
        std::stringstream request;

        request << "SELECT tasks.* FROM (SELECT * FROM assignees WHERE assignee_ID = "
                << this->manager->ID
                << ") assignees_with_access INNER JOIN tasks ON assignees_with_access.room_creator_ID = tasks.room_creator_ID  AND assignees_with_access.room_name = tasks.room_name AND assignees_with_access.task_name = tasks.name";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto tasks = this->convert_data_base_response_to_matrix(result.rows());

        for (auto task : tasks) {
            struct task curr_task = task;

            struct profile task_creator;

            auto status = this->get_profile_by_ID(curr_task.creator_ID, task_creator);

            if (!status)
                curr_task.creator_name = task_creator.name;
            else
                curr_task.creator_name = "пользователся, создавшего данную задачу, более не существует";

            result_tasks.push_back(curr_task);
        }

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_profile_reviewed_tasks(std::vector<task> &result_tasks) -> DATA_BASE_EXECUTION_STATUS {
    try {
        struct profile profile;

        auto status = get_profile_by_ID(this->manager->ID, profile);

        if (status)
            return status;
        
        std::stringstream request;

        request << "SELECT tasks.* FROM (SELECT * FROM reviewers WHERE reviewer_ID = "
                << this->manager->ID
                << ") reviewers_with_access INNER JOIN tasks ON reviewers_with_access.room_creator_ID = tasks.room_creator_ID  AND reviewers_with_access.room_name = tasks.room_name AND reviewers_with_access.task_name = tasks.name";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto tasks = this->convert_data_base_response_to_matrix(result.rows());

        for (auto task : tasks) {
            struct task curr_task = task;

            struct profile task_creator;

            auto status = this->get_profile_by_ID(curr_task.creator_ID, task_creator);

            if (!status)
                curr_task.creator_name = task_creator.name;
            else
                curr_task.creator_name = "пользователся, создавшего данную задачу, более не существует";

            result_tasks.push_back(curr_task);
        }

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_profile_received_invites(std::vector<invite> &result_invites) -> DATA_BASE_EXECUTION_STATUS {
    try {
        std::stringstream request;

        request << "SELECT * FROM invites WHERE receiver_ID = "
                << this->manager->ID;

        std::cout << request.str();

        conn.execute(request.str(), result);

        auto invites = this->convert_data_base_response_to_matrix(result.rows());

        for (auto invite : invites) {
            struct invite curr_invite = invite;

            curr_invite.receiver_name = this->manager->name;

            struct profile sender;

            auto sender_ID = invite.at(0).get_uint64();

            auto status = this->get_profile_by_ID(sender_ID, sender);

            if (status)
                return status;

            curr_invite.sender_name = sender.name;

            result_invites.push_back(curr_invite);
        }

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_profile_sended_invites(std::vector<invite> &result_invites) -> DATA_BASE_EXECUTION_STATUS {
    try {
        std::stringstream request;

        request << "SELECT * FROM invites WHERE sender_ID = "
                << this->manager->ID;

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto invites = this->convert_data_base_response_to_matrix(result.rows());

        for (auto invite : invites) {
            struct invite curr_invite = invite;

            curr_invite.sender_name = this->manager->name;

            struct profile receiver;

            auto receiver_ID = invite.at(1).get_uint64();

            auto status = this->get_profile_by_ID(receiver_ID, receiver);

            if (status)
                return status;

            curr_invite.receiver_name = receiver.name;

            result_invites.push_back(curr_invite);
        }

        return DATA_BASE_COMPLETED_SUCCESSFULY;
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
                << this->manager->login
                << "' AND password = '"
                << this->manager->password
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto profile = this->convert_data_base_response_to_matrix(result.rows());

        if (!profile.size())
            return DATA_BASE_THERE_IS_NO_PROFILE_WITH_SUCH_LOGIN_AND_PASSWORD;

        request.str(std::string());

        request << "DELETE FROM profiles WHERE login = '"
                << this->manager->login
                << "' AND password = '"
                << this->manager->password
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

auto data_base_manager::loggin_profile(const std::string &login, const std::string &password, profile &result_profile) -> DATA_BASE_EXECUTION_STATUS
{
    try {     
        std::stringstream request;
        
        request << "SELECT * FROM profiles WHERE login = '"
                << login
                << "' AND password = '"
                << password
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto profiles = this->convert_data_base_response_to_matrix(result.rows());

        for (auto profile : profiles) {
            *this->manager = profile;
            result_profile = profile;

            return DATA_BASE_COMPLETED_SUCCESSFULY;
        }

        return DATA_BASE_THERE_IS_NO_PROFILE_WITH_SUCH_LOGIN_AND_PASSWORD;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::profile_authenticate(profile &result_profile) -> DATA_BASE_EXECUTION_STATUS {
    return this->loggin_profile(this->manager->login, this->manager->password, result_profile);
}

auto data_base_manager::get_profiles_with_substr_in_name(
        const std::string &substr, 
        const u_int64_t offset,
        std::vector<profile> &result_profiles) -> DATA_BASE_EXECUTION_STATUS {
    try {
        std::stringstream request;
        
        request << "SELECT * FROM (SELECT * FROM profiles WHERE name LIKE '%"
                << substr
                << "%' AND ID <> "
                << this->manager->ID
                << ") profiles_with_substr ORDER BY ID DESC LIMIT 10 OFFSET "
                << offset;

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto profiles = this->convert_data_base_response_to_matrix(result.rows());

        for (auto profile : profiles) {
            struct profile curr_profile = profile;

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
auto data_base_manager::create_room(
        const std::string &room_name, 
        const std::string &description, 
        room &result_room) -> DATA_BASE_EXECUTION_STATUS {
    try {
        struct profile profile;

        auto status = this->get_profile_by_ID(this->manager->ID, profile);
        
        if (status)
            return status;

        std::stringstream request;

        request << "SELECT * FROM rooms WHERE creator_ID = "
                << this->manager->ID
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
                << this->manager->ID 
                << ", '" 
                << room_name
                << "', '" 
                << description
                << "')";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);
    
        request.str(std::string());

        request << "INSERT INTO profile_room (profile_ID, room_creator_ID, room_name) VALUES ("
                << this->manager->ID
                << ", "
                << this->manager->ID
                << ", '"
                << room_name
                << "')";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);

        return this->get_room(this->manager->ID, room_name, result_room);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_room(
        const u_int64_t room_creator_ID, 
        const std::string &room_name, 
        room &result_room) -> DATA_BASE_EXECUTION_STATUS {
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
                << this->manager->ID
                << ") rooms_with_access WHERE rooms_with_access.creator_ID = " 
                << room_creator_ID
                << " AND rooms_with_access.name = '"
                << room_name
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        std::vector<std::vector<field>> rooms = this->convert_data_base_response_to_matrix(result.rows());

        if (rooms.size() != room_with_no_access.size())
            return DATA_BASE_ROOM_ACCESS_ERROR;

        if (rooms.size()) {
            result_room = rooms.at(0);

            struct profile creator_profile;

            auto status = this->get_profile_by_ID(result_room.creator_ID, creator_profile);
            
            if (!status)
                result_room.creator_name = creator_profile.name;
            else
                result_room.creator_name = "пользователя, создавшего данную комнату, более не существует";

            return DATA_BASE_COMPLETED_SUCCESSFULY;
        }

        return DATA_BASE_THERE_IS_NO_ROOM_WITH_SUCH_PARAMETERS;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message();
        std::cout << exception.get_diagnostics().client_message();

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::leave_from_room(
        u_int64_t room_creator_ID, 
        std::string room_name) -> DATA_BASE_EXECUTION_STATUS {
    try {
        std::stringstream request;

        request << "SELECT * FROM profile_room WHERE room_creator_ID = "
                << room_creator_ID
                << " AND room_name = '"
                << room_name
                << "' AND profile_ID = "
                << this->manager->ID;

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto profile_rooms = this->convert_data_base_response_to_matrix(result.rows());

        if (!profile_rooms.size())
            return DATA_BASE_ROOM_ACCESS_ERROR;

        request.str(std::string());

        request << "DELETE FROM profile_room WHERE room_creator_ID = "
                << room_creator_ID
                << " AND profile_ID = "
                << this->manager->ID
                << " AND room_name = '"
                <<  room_name
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message();
        std::cout << exception.get_diagnostics().client_message();

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::delete_room(
        const u_int64_t room_creator_ID, 
        const std::string &room_name) -> DATA_BASE_EXECUTION_STATUS {
    try {
        struct room room;
        
        auto status = this->get_room(room_creator_ID, room_name, room);

        if (status)
            return status;

        std::stringstream request;

        request << "DELETE FROM rooms WHERE creator_ID = "
                << room.creator_ID
                << " AND name = '"
                << room.name
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

// Task part
auto data_base_manager::create_task(
        const u_int64_t room_creator_ID, 
        const std::string &room_name,
        const std::string &task_name,
        const std::string &description,
        const std::string &label,
        const u_int64_t task_status,
        const time_t &time_to_live,
        task &result_task) -> DATA_BASE_EXECUTION_STATUS {
    try {
        struct profile profile;

        auto status = this->get_profile_by_ID(this->manager->ID, profile);
        
        if (status)
            return status;

        struct room room;
        
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
        
        struct timer task_creation_timer(time_to_live);

        request.str(std::string());

        request << "INSERT INTO tasks (room_creator_ID, room_name, creator_ID, name, description, label, status, creation_time, deadline) VALUES ("
                << room.creator_ID
                << ", '"
                << room.name
                << "', "
                << this->manager->ID
                << ", '"
                << task_name
                << "', '"
                << description
                << "', '"
                << label
                << "', "
                << task_status
                << ", '"
                << task_creation_timer.creation_time
                << "', '"
                << task_creation_timer.deadline
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

auto data_base_manager::add_task_to_assignee(
        const u_int64_t room_creator_ID,
        const std::string &room_name,
        const std::string &task_name,
        const u_int64_t assignee_ID) -> DATA_BASE_EXECUTION_STATUS {
    try {
        struct profile assignee;

        auto status = get_profile_by_ID(assignee_ID, assignee);

        if (status)
            return status;

        struct task task;

        status = get_task(room_creator_ID, room_name, task_name, task);

        if (status)
            return status;

        std::stringstream request;

        request << "SELECT * FROM assignees WHERE assignee_ID = "
                << assignee.ID
                << " AND room_creator_ID = '"
                << task.room_creator_ID
                << "' AND room_name = '"
                << task.room_name
                << "' AND task_name = '"
                << task.name
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto assignees = this->convert_data_base_response_to_matrix(result.rows());

        if (assignees.size())
            return DATA_BASE_THIS_PROFILE_IS_ALREADY_REVIEW_THIS_TASK;

        request.str(std::string());

        request << "INSERT INTO assignees (assignee_ID, room_creator_ID, room_name, task_name) VALUES ("
                << assignee.ID
                << ", "
                << task.room_creator_ID
                << ", '"
                << task.room_name
                << "', '"
                << task.name
                << "')";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::add_task_to_reviewer(
        const u_int64_t room_creator_ID,
        const std::string &room_name,
        const std::string &task_name,
        const u_int64_t reviewer_ID) -> DATA_BASE_EXECUTION_STATUS {
    try {
        struct profile reviewer;

        auto status = get_profile_by_ID(reviewer_ID, reviewer);

        if (status)
            return status;

        struct task task;

        status = get_task(room_creator_ID, room_name, task_name, task);

        if (status)
            return status;

        std::stringstream request;

        request << "SELECT * FROM reviewers WHERE reviewer_ID = "
                << reviewer.ID
                << " AND room_creator_ID = '"
                << task.room_creator_ID
                << "' AND room_name = '"
                << task.room_name
                << "' AND task_name = '"
                << task.name
                << "'";
            
        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto reviewers = this->convert_data_base_response_to_matrix(result.rows());

        if (reviewers.size())
            return DATA_BASE_THIS_PROFILE_IS_ALREADY_REVIEW_THIS_TASK;

        request.str(std::string());

        request << "INSERT INTO reviewers (reviewer_ID, room_creator_ID, room_name, task_name) VALUES ("
                << reviewer.ID
                << ", "
                << task.room_creator_ID
                << ", '"
                << task.room_name
                << "', '"
                << task.name
                << "')";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_task(
        const u_int64_t room_creator_ID, 
        const std::string &room_name, 
        const std::string &task_name, 
        task &result_task) -> DATA_BASE_EXECUTION_STATUS {
    try {
        struct room room;
    
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

        auto tasks = this->convert_data_base_response_to_matrix(result.rows());

        if (tasks.size()) {
            result_task = tasks.at(0);

            struct profile task_creator;

            status = this->get_profile_by_ID(result_task.creator_ID, task_creator);

            if (!status)
                result_task.creator_name = task_creator.name;
            else
                result_task.creator_name = "пользователя, создавшего данную задачу, более не существует";

            return DATA_BASE_COMPLETED_SUCCESSFULY;
        }
            
        return DATA_BASE_THERE_IS_NO_TASK_WITH_SUCH_PARAMETERS;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_task_assignees(
        const u_int64_t room_creator_ID,
        const std::string &room_name, 
        const std::string &task_name, 
        std::vector<profile> &result_assignees) -> DATA_BASE_EXECUTION_STATUS {
    try {
        struct task task;
        
        auto status = this->get_task(room_creator_ID, room_name, task_name, task);

        if (status)
            return status;

        std::stringstream request;

        request << "SELECT profiles.* FROM assignees INNER JOIN profiles ON assignees.profile_ID = profiles.ID AND assignees.room_creator_ID = "
                << task.room_creator_ID
                << " AND assignees.room_name = '"
                << task.room_name
                << "' AND assignees.task_name = '"
                << task.name
                << "'";
        
        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto profiles = this->convert_data_base_response_to_matrix(result.rows());

        for (auto profile : profiles) {
            struct profile curr_profile = profile;

            result_assignees.push_back(curr_profile);
        }

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    }  catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_task_reviewers(
        const u_int64_t room_creator_ID,
        const std::string &room_name, 
        const std::string &task_name, 
        std::vector<profile> &result_reviewers) -> DATA_BASE_EXECUTION_STATUS {
    try {
        struct task task;
        
        auto status = this->get_task(room_creator_ID, room_name, task_name, task);

        if (status)
            return status;

        std::stringstream request;

        request << "SELECT profiles.* FROM reviewers INNER JOIN profiles ON reviewers.profile_ID = profiles.ID AND reviewers.room_creator_ID = "
                << task.room_creator_ID
                << " AND reviewers.room_name = '"
                << task.room_name
                << "' AND reviewers.task_name = '"
                << task.name
                << "'";
        
        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto profiles = this->convert_data_base_response_to_matrix(result.rows());

        for (auto profile : profiles) {
            struct profile curr_profile = profile;

            result_reviewers.push_back(curr_profile);
        }

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    }  catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::delete_task(
        const u_int64_t room_creator_ID,
        const std::string &room_name,
        const std::string &task_name) -> DATA_BASE_EXECUTION_STATUS{
    try {
        struct room room;
        
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
    
auto data_base_manager::remove_task_from_assignee(
        const u_int64_t room_creator_ID,
        const std::string &room_name,
        const std::string &task_name,
        const u_int64_t assignee_ID) -> DATA_BASE_EXECUTION_STATUS {
    try {
        struct profile assignee;

        auto status = get_profile_by_ID(assignee_ID, assignee);

        if (status)
            return status;

        struct task task;

        status = get_task(room_creator_ID, room_name, task_name, task);

        if (status)
            return status;

        std::stringstream request;

        request << "SELECT * FROM assignees WHERE assignee_ID = "
                << assignee.ID
                << " AND room_creator_ID = '"
                << task.room_creator_ID
                << "' AND room_name = '"
                << task.room_name
                << "' AND task_name = '"
                << task.name
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto assignees = this->convert_data_base_response_to_matrix(result.rows());

        if (!assignees.size())
            return DATA_BASE_THIS_ID_IS_NOT_ASSIGNEE_THIS_TASK;

        request.str(std::string());

        request << "DELETE FROM assignees WHERE assignee_ID = "
                << assignee.ID
                << " AND room_creator_ID = '"
                << task.room_creator_ID
                << "' AND room_name = '"
                << task.room_name
                << "' AND task_name = '"
                << task.name
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

auto data_base_manager::remove_task_from_reviewer(
        const u_int64_t room_creator_ID,
        const std::string &room_name,
        const std::string &task_name,
        const u_int64_t reviewer_ID) -> DATA_BASE_EXECUTION_STATUS {
    try {
        struct profile reviewer;

        auto status = get_profile_by_ID(reviewer_ID, reviewer);

        if (status)
            return status;

        struct task task;

        status = get_task(room_creator_ID, room_name, task_name, task);

        if (status)
            return status;

        std::stringstream request;

        request << "SELECT * FROM reviewers WHERE reviewer_ID = "
                << reviewer.ID
                << " AND room_creator_ID = '"
                << task.room_creator_ID
                << "' AND room_name = '"
                << task.room_name
                << "' AND task_name = '"
                << task.name
                << "'";
            
        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto reviewers = this->convert_data_base_response_to_matrix(result.rows());

        if (!reviewers.size())
            return DATA_BASE_THIS_ID_IS_NOT_REVIEW_THIS_TASK;

        request.str(std::string());

        request << "DELETE FROM reviewers WHERE reviewer_ID = "
                << reviewer.ID
                << " AND room_creator_ID = '"
                << task.room_creator_ID
                << "' AND room_name = '"
                << task.room_name
                << "' AND task_name = '"
                << task.name
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


//Invite part
auto data_base_manager::create_invite(
        const u_int64_t receiver_ID, 
        const u_int64_t room_creator_ID, 
        const std::string room_name,
        invite &result_invite) -> DATA_BASE_EXECUTION_STATUS {
    try {
        struct profile member;

        auto status = this->get_profile_by_ID(receiver_ID, member);

        if (status)
            return  status;

        struct room room;

        status = this->get_room(room_creator_ID, room_name, room);

        if (status)
            return  status;

        status = this->get_invite(this->manager->ID,
                                  receiver_ID, 
                                  room.creator_ID, 
                                  room.name, 
                                  result_invite);

        if (!status)
            return DATA_BASE_INVITE_ALREADY_SENDED;

        std::vector<profile> result_profiles;

        status = this->get_room_profiles(room.creator_ID, room.name, result_profiles);

        if (status)
            return status;

        std::cout << result_profiles.size() << std::endl;

        for (auto profile : result_profiles) {
            if (receiver_ID == profile.ID)
                return DATA_BASE_THIS_PROFILE_IS_ALREADY_EXIST_IN_THIS_ROOM;
        }

        std::stringstream request;

        request << "INSERT INTO invites (sender_ID, receiver_ID, room_creator_ID, room_name) VALUES ("
            << this->manager->ID
            << ", "
            << receiver_ID
            << ", "
            << room.creator_ID
            << ", '"
            << room.name
            << "')";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);

        return this->get_invite(this->manager->ID,
                                receiver_ID, 
                                room.creator_ID, 
                                room.name, 
                                result_invite);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}    

auto data_base_manager::get_invite(
        const u_int64_t sender_ID, 
        const u_int64_t receiver_ID, 
        const u_int64_t room_creator_ID, 
        const std::string room_name,
        invite &result_invite) -> DATA_BASE_EXECUTION_STATUS {
    try {
        if (!((sender_ID == this->manager->ID) || (receiver_ID == this->manager->ID)))
            return DATA_BASE_THETE_IS_NO_ACCESS_FOR_THIS_INVITE;

        std::stringstream request;
        
        request << "SELECT * FROM invites WHERE sender_ID = "
            << sender_ID
            << " AND receiver_ID = "
            << receiver_ID
            << " AND room_creator_ID = "
            << room_creator_ID
            << " AND room_name = '"
            << room_name
            << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto invites = this->convert_data_base_response_to_matrix(result.rows());
    
        if (!invites.size())
            return DATA_BASE_THERE_IS_NO_INVITES_WITH_SUCH_PARAMETERS;

        result_invite = invites.at(0); 

        profile sender, receiver;

        auto status = this->get_profile_by_ID(sender_ID, sender);

        if (status)
            return status;

        result_invite.sender_name = sender.name;

        status = this->get_profile_by_ID(receiver_ID, receiver);

        if (status)
            return status;

        result_invite.receiver_name = receiver.name;

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::accept_invite(
        u_int64_t room_creator_ID, 
        std::string room_name,
        invite &accepted_invite) -> DATA_BASE_EXECUTION_STATUS {
    try {
        std::stringstream request;

        request << "SELECT * FROM invites WHERE receiver_ID = "
                << this->manager->ID 
                << " AND room_creator_ID = "
                << room_creator_ID
                << " AND room_name = '"
                << room_name
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto invites = this->convert_data_base_response_to_matrix(result.rows());

        if (!invites.size())
            return DATA_BASE_THERE_IS_NO_INVITES_WITH_SUCH_PARAMETERS;

        accepted_invite = invites.at(0);

        profile sender, receiver;

        auto status = this->get_profile_by_ID(invites.at(0).at(0).get_uint64(), sender);

        if (status)
            return status;

        accepted_invite.sender_name = sender.name;

        status = this->get_profile_by_ID(this->manager->ID, receiver);

        if (status)
            return status;

        accepted_invite.receiver_name = receiver.name;


        request.str(std::string());

        request << "INSERT INTO profile_room (profile_ID, room_creator_ID, room_name) VALUES ("
                << this->manager->ID
                << ", "
                << room_creator_ID
                << ", '"
                << room_name
                << "')";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);

        request.str(std::string());

        request << "DELETE FROM invites WHERE receiver_ID = "
                << this->manager->ID 
                << " AND room_creator_ID = "
                << room_creator_ID
                << " AND room_name = '"
                << room_name
                << "'";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::delete_sended_invite(
            u_int64_t receiver_ID, 
            u_int64_t room_creator_ID, 
            std::string room_name,
            invite &deleted_invite) -> DATA_BASE_EXECUTION_STATUS {
    return this->delete_invite(this->manager->ID, 
                               receiver_ID, 
                               room_creator_ID,
                               room_name,
                               deleted_invite);
}

auto data_base_manager::delete_received_invite(
        u_int64_t sender_ID, 
        u_int64_t room_creator_ID, 
        std::string room_name,
        invite &deleted_invite) -> DATA_BASE_EXECUTION_STATUS {
    return this->delete_invite(sender_ID,
                               this->manager->ID, 
                               room_creator_ID,
                               room_name,
                               deleted_invite);
}

auto data_base_manager::delete_invite(
        u_int64_t sender_ID,
        u_int64_t receiver_ID, 
        u_int64_t room_creator_ID, 
        std::string room_name,
        invite &deleted_invite) -> DATA_BASE_EXECUTION_STATUS {
    try {
        auto status = this->get_invite(sender_ID, receiver_ID, room_creator_ID, room_name, deleted_invite);
        
        if (status)
            return DATA_BASE_THERE_IS_NO_INVITES_WITH_SUCH_PARAMETERS;

        std::stringstream request;

        request.str(std::string());

        request << "DELETE FROM invites WHERE sender_ID = "
                << sender_ID
                << " AND receiver_ID = "
                << receiver_ID
                << " AND room_creator_ID = "
                << room_creator_ID
                << " AND room_name = '"
                << room_name
                << "'";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}


// Join part
auto data_base_manager::get_profile_rooms(std::vector<room> &result_rooms) -> DATA_BASE_EXECUTION_STATUS {
    try {
        struct profile profile;

        auto status = this->get_profile_by_ID(this->manager->ID, profile);

        if (status)
            return status;

        std::stringstream request;

        request << "SELECT rooms.* FROM profile_room INNER JOIN rooms ON profile_room.room_creator_ID = rooms.creator_ID AND profile_room.room_name = rooms.name AND profile_room.profile_ID = "
                << this->manager->ID;

        std::cout << request.str();
                
        conn.execute(request.str(), result);

        auto rooms = this->convert_data_base_response_to_matrix(result.rows());

        for (auto room : rooms) {
            struct room curr_room  = room;

            struct profile room_creator;

            auto status = this->get_profile_by_ID(curr_room.creator_ID, room_creator);

            if (!status)            
                curr_room.creator_name = room_creator.name;
            else
                curr_room.creator_name = "пользователя, создавшего данную комнату, более не существует";

            result_rooms.push_back(curr_room);
        }

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_profile_tasks(std::vector<task> &result_tasks) -> DATA_BASE_EXECUTION_STATUS {
    try {
        struct profile profile;

        auto status = this->get_profile_by_ID(this->manager->ID, profile);

        if (status)
            return status;
        
        std::stringstream request;

        request << "SELECT tasks.* FROM tasks INNER JOIN (SELECT profile_room.room_creator_ID, profile_room.room_name FROM profile_room INNER JOIN rooms ON profile_room.room_creator_ID = rooms.creator_ID AND profile_room.room_name = rooms.name AND profile_room.profile_ID = "
                << this->manager->ID
                << ") rooms_with_access ON rooms_with_access.room_creator_ID = tasks.room_creator_ID AND rooms_with_access.room_name = tasks.room_name";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto tasks = this->convert_data_base_response_to_matrix(result.rows());

        for (auto task : tasks) {
            struct task curr_task = task;

            struct profile task_creator;

            auto status = this->get_profile_by_ID(curr_task.creator_ID, task_creator);

            if (!status)
                curr_task.creator_name = task_creator.name;
            else
                curr_task.creator_name = "пользователся, создавшего данную задачу, более не существует";

            result_tasks.push_back(curr_task);
        }

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_room_profiles(const u_int64_t room_creator_ID, const std::string &room_name, std::vector<profile> &result_profiles) -> DATA_BASE_EXECUTION_STATUS {
    try {
        struct room room;

        auto status = this->get_room(room_creator_ID, room_name, room);

        if (status)
            return status;

        std::stringstream request;

        request << "SELECT profiles.* FROM (SELECT profile_ID FROM (SELECT room_creator_ID, room_name FROM profile_room WHERE profile_ID = "
                << this->manager->ID
                << ") rooms_with_access INNER JOIN profile_room ON profile_room.room_creator_ID = rooms_with_access.room_creator_ID AND profile_room.room_name = rooms_with_access.room_name AND profile_room.room_creator_ID = "
                << room_creator_ID
                << " AND profile_room.room_name = '"
                << room_name
                << "') room_profiles_ID INNER JOIN profiles ON room_profiles_ID.profile_ID = profiles.ID";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto profiles = this->convert_data_base_response_to_matrix(result.rows());

        for (auto profile : profiles) {
            struct profile curr_profile = profile;

            result_profiles.push_back(curr_profile);
        }

        return DATA_BASE_COMPLETED_SUCCESSFULY;
    } catch (boost::mysql::error_with_diagnostics &exception) {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;

        return DATA_BASE_FAILED;
    }
}

auto data_base_manager::get_room_tasks(const u_int64_t room_creator_ID, const std::string &room_name, std::vector<task> &result_tasks) -> DATA_BASE_EXECUTION_STATUS {
    try {
        struct room room;

        auto status = this->get_room(room_creator_ID, room_name, room);

        if (status)
            return status;

        std::stringstream request;

        request << "SELECT tasks.* FROM tasks INNER JOIN (SELECT rooms.creator_ID, rooms.name FROM profile_room INNER JOIN rooms ON profile_room.room_creator_ID = rooms.creator_ID AND profile_room.room_name = rooms.name AND profile_room.profile_ID = "
                << this->manager->ID
                << ") rooms_with_access ON tasks.room_creator_ID = rooms_with_access.creator_ID AND tasks.room_name = rooms_with_access.name AND rooms_with_access.creator_ID = "
                << room.creator_ID
                << " AND rooms_with_access.name = '"
                << room.name
                << "'";

        std::cout << request.str() << std::endl;

        conn.execute(request.str(), result);

        auto tasks = this->convert_data_base_response_to_matrix(result.rows());
        
        for (auto task : tasks) {
            struct task curr_task  = task;

            struct profile task_creator;

            status = this->get_profile_by_ID(curr_task.creator_ID, task_creator);

            if (!status)
                curr_task.creator_name = task_creator.name;
            else
                curr_task.creator_name = "пользователся, создавшего данную задачу, более не существует";

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

auto data_base_manager::convert_data_base_response_to_matrix(const rows_view &rows) -> std::vector<std::vector<boost::mysql::field>> {
    std::vector<std::vector<field>> matrix;

    for (auto row : rows) {
        matrix.push_back(row.as_vector());
    }

    return matrix;
}

auto data_base_manager::print_tabel(const std::string &name) -> void {
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

auto data_base_manager::drop_table(const std::string &name) -> void {
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

auto data_base_manager::update_tables() -> void {
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

        request << "SELECT * FROM configs";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        this->create_configs_table();
    }

    try {
        std::stringstream request;

        request << "SELECT * FROM assignees";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        this->create_assignees_table();
    }

    try {
        std::stringstream request;

        request << "SELECT * FROM reviewers";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        this->create_reviewers_table();
    }

    try {
        std::stringstream request;

        request << "SELECT * FROM invites";

        std::cout << request.str() << std::endl;

        conn.query(request.str(), result);
    } catch (boost::mysql::error_with_diagnostics &exception) {
        this->create_invites_table();
    }
}

auto data_base_manager::drop_tables() -> void {
    drop_table("profiles");
    drop_table("rooms");
    drop_table("profile_room");
    drop_table("tasks");
    drop_table("configs");
    drop_table("assignees");
    drop_table("reviewers");
    drop_table("invites");
}
