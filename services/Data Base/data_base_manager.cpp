#include <iostream>
#include <string>

#include "data_base_manager.h"

using namespace boost::mysql;

// Object part

data_base_manager::data_base_manager(const std::string &host, const std::string &port, const std::string &profilename, const std::string &password, const std::string &db) : ssl_ctx(boost::asio::ssl::context(boost::asio::ssl::context::tls_client)),
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

    // drop_table("profiles");
    // drop_table("rooms");
    // drop_table("profile_room");
    // drop_table("tasks");
}

data_base_manager::~data_base_manager()
{
    conn.close();
}

// Profile part

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
        )%",
        result);
}

auto data_base_manager::create_profile(const std::string &name, const std::string &login, const std::string &password) -> bool
{
    try
    {
        conn.start_query("SELECT login, password FROM profiles WHERE login = '" + login + "'", state);

        if (conn.read_some_rows(state).size())
            return false;
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
    }

    conn.query(
        "INSERT INTO profiles (name, login, password) VALUES ('" + name +  "', '" + login + "', '" + password + "')",
        result);

    return true;
}

auto data_base_manager::get_profile_id(const std::string &login, const std::string &password) -> std::string
{
    try
    {
        conn.start_query("SELECT ID FROM profiles WHERE login = '" + login + "'AND password = '" + password + "'", state);

        auto result = conn.read_some_rows(state);

        return result.size() ? std::to_string(result.at(0).at(0).get_uint64()) : "0";
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
        return 0;
    }
}

auto data_base_manager::get_profile(const std::string &ID) -> profile
{
    try {
        conn.start_query("SELECT ID, name FROM profiles WHERE ID = '" + ID + "'",
                         state);

        auto profile_names = conn.read_some_rows(state);

        if (profile_names.size() > 0)
            return {std::to_string(profile_names.at(0).at(0).get_uint64()), profile_names.at(0).at(1).get_string()};
    } catch (boost::mysql::error_with_diagnostics &exception) {
    }

    return {};
}

auto data_base_manager::delete_profile(const std::string &login, const std::string &password) -> void
{
    try
    {
        conn.start_query("DELETE FROM profiles WHERE login = '" + login + "'AND password = '" + password + "'", state);
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;
    }
}

auto data_base_manager::login_in_profile(const std::string &login, const std::string &password) -> bool
{
    try
    {      
        conn.start_query("SELECT login, password FROM profiles WHERE login = '" + login + "' AND password = '" + password + "'", state);

        auto result = conn.read_some_rows(state);

        if (!result.size())
            return false;
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
        return false;
    }

    return true;
}

auto data_base_manager::find_profiles_with_prefix_in_name(const std::string &prefix) -> std::vector<std::vector<std::string>>
{
    std::vector<std::vector<std::string>> res;

    try
    {
        conn.start_query("SELECT * FROM profiles WHERE login LIKE '" + prefix + "%'", state);

        for (auto i : conn.read_some_rows(state))
        {
            res.push_back({i.at(1).get_string(), i.at(2).get_string()});
        }
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
    }

    return res;
}

// Room part

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
        )%",
        result);
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
        )%",
        result);
}

auto data_base_manager::create_room(const std::string &creator_id, const std::string &label) -> bool
{
    try
    {
        conn.start_query("SELECT * FROM rooms WHERE creatorID = '" + creator_id + "' AND label = '" + label + "'", state);

        if (conn.read_some_rows(state).size())
            return false;
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
    }

    conn.query(
        "INSERT INTO rooms (creatorID, label) VALUES ('" + creator_id + "', '" + label + "')",
        result);

    conn.query(
        "INSERT INTO profile_room (profileID, roomID) VALUES ('" + creator_id + "', '" + this->get_room_id(creator_id, label) + "')",
        result);

    return true;
}

auto data_base_manager::get_room_id(const std::string &creator_id, const std::string &label) -> std::string
{
    try
    {
        conn.start_query("SELECT ID FROM rooms WHERE creatorID = '" + creator_id + "' AND label = '" + label + "'", state);

        auto result = conn.read_some_rows(state);

        return result.size() ? std::to_string(result.at(0).at(0).get_int64()) : "0";
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
        return "0";
    }
}

auto data_base_manager::delete_room(const std::string &creator_id, const std::string &label) -> void
{
    try
    {
        conn.start_query("DELETE FROM rooms WHERE creatorID = '" + creator_id + "'AND label = '" + label + "'", state);
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
    }
}

auto data_base_manager::append_member_to_room(const std::string &member_id, const std::string &creator_id, const std::string &label) -> bool
{
    std::string room_id = this->get_room_id(creator_id, label);

    try
    {
        conn.start_query("SELECT * FROM profile_room WHERE creatorID = '" + member_id + "' AND label = '" + room_id + "'", state);

        if (conn.read_some_rows(state).size())
            return false;
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
    }

    conn.query(
        "INSERT INTO profile_room (profileID, roomID) VALUES ('" + member_id + "', '" + room_id + "')",
        result);

    return true;
}

// Task part

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
        )%",
        result);
}

auto data_base_manager::create_task(const std::string &room_id, const std::string &label, const std::string &creator_id) -> bool
{
    try
    {
        conn.start_query("SELECT * FROM tasks WHERE roomID = '" + room_id + "' AND label = '" + label + "'", state);

        if (conn.read_some_rows(state).size())
            return false;
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
    }

    conn.query(
        "INSERT INTO tasks (roomID, label, creatorID) VALUES ('" + room_id + "', '" + label + "', '" + creator_id + "')",
        result);

    return true;
}

auto data_base_manager::get_task_id(const std::string &room_id, const std::string &label) -> std::string
{
    try
    {
        conn.start_query("SELECT ID FROM tasks WHERE roomID = '" + room_id + "' AND label = '" + label + "'", state);

        auto result = conn.read_some_rows(state);

        return result.size() ? std::to_string(result.at(0).at(0).get_int64()) : "0";
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
        return "0";
    }
}

auto data_base_manager::delete_task(const std::string &room_id, const std::string &label) -> bool
{
    try
    {
        conn.start_query("SELECT FROM tasks WHERE roomID = '" + room_id + "'AND label = '" + label + "'", state);

        if (!conn.read_some_rows(state).size())
            return false;
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
        return false;
    }

    try
    {
        conn.start_query("DELETE FROM tasks WHERE roomID = '" + room_id + "'AND label = '" + label + "'", state);
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
        return false;
    }

    return true;
}

// Join part

auto data_base_manager::get_profile_rooms(const std::string &profile_id) -> std::vector<room>
{
    std::vector<room> res;

    try
    {
        conn.start_query("SELECT * FROM profile_room INNER JOIN rooms ON profile_room.roomID = rooms.ID AND profile_room.profileID = '" + profile_id + "'",
                         state);

        auto rooms = conn.read_some_rows(state);

        for (auto room : rooms)
        {
            res.push_back({std::to_string(room.at(3).get_int64()), this->get_profile(std::to_string(room.at(3).get_int64())).name, room.at(4).get_string(), {}});
        }
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
    }

    return res;
}

auto data_base_manager::get_profile_tasks(const std::string &profile_id) -> std::vector<task>
{
    std::vector<task> res;

    try
    {
        conn.start_query("SELECT * FROM tasks INNER JOIN (SELECT profile_room.roomID FROM profile_room INNER JOIN rooms ON profile_room.roomID = rooms.ID AND profile_room.profileID = '" + profile_id + "') t2 ON t2.roomID = tasks.roomID",
                         state);

        auto tasks = conn.read_some_rows(state);

        for (auto task : tasks)
        {
            res.push_back({std::to_string(task.at(3).get_int64()), this->get_profile(std::to_string(task.at(3).get_int64())).name, task.at(2).get_string()});
        }
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
    }

    return res;
}

auto data_base_manager::get_room_profiles(const std::string &room_id) -> std::vector<profile>
{
    std::vector<profile> res;

    try
    {
        conn.start_query("SELECT * FROM profile_room INNER JOIN profiles ON profile_room.profileID = profiles.ID",
                         state);

        auto profiles = conn.read_some_rows(state);

        for (auto profile : profiles)
        {
            res.push_back({std::to_string(profile.at(2).get_uint64()), profile.at(3).get_string(), {}});
        }
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
    }

    return res;
}

auto data_base_manager::get_room_tasks(const std::string &room_id) -> std::vector<task>
{
    std::vector<task> res;

    try
    {
        conn.start_query("SELECT * FROM tasks INNER JOIN rooms ON tasks.roomID = rooms.ID AND rooms.ID = '" + room_id + "'", state);

        auto tasks = conn.read_some_rows(state);

        for (auto task : tasks)
        {
            res.push_back({std::to_string(task.at(3).get_int64()), this->get_profile(std::to_string(task.at(3).get_int64())).name, task.at(2).get_string()});
        }
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
    }

    return res;
}

// Data base table management part

auto data_base_manager::print_tabel(const std::string &name) -> void
{
    try
    {
        conn.start_query("SELECT * FROM " + name, state);
        auto res = conn.read_some_rows(state);

        for (auto i : res)
        {
            for (auto j : i)
            {
                std::cout << j << " ";
            }
            std::cout << std::endl;
        }
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
    }
}

auto data_base_manager::drop_table(const std::string &name) -> void
{
    try
    {
        conn.query("SET FOREIGN_KEY_CHECKS=0", result);
        conn.query("DROP TABLE " + name, result);
        conn.query("SET FOREIGN_KEY_CHECKS=1;", result);
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
        std::cout << exception.get_diagnostics().server_message() << std::endl;
        std::cout << exception.get_diagnostics().client_message() << std::endl;
    }
}

auto data_base_manager::update_tables() -> void
{
    try
    {
        conn.query("SELECT * FROM profiles", result);
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
        this->create_profiles_table();
    }

    try
    {
        conn.query("SELECT * FROM rooms", result);
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
        this->create_rooms_table();
    }

    try
    {
        conn.query("SELECT * FROM profile_room", result);
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
        this->create_profile_room_table();
    }

    try
    {
        conn.query("SELECT * FROM tasks", result);
    }
    catch (boost::mysql::error_with_diagnostics &exception)
    {
        this->create_tasks_table();
    }
}