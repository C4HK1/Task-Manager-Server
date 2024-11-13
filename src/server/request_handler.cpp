#include "services/JWT_manager.h"
#include "services/data_base_manager.h"

#include "server/request_handler.h"
#include "server/server_status.h"

#include "models/profile.h" 
#include "models/room.h" 
#include "models/task.h" 
#include "models/config.h" 
#include "models/invite.h" 
#include <cstddef>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <sys/types.h>

request_handler::request_handler(http::request<http::dynamic_body> *request, http::response<http::dynamic_body> *response) : 
        request(request),
        response(response) {
    std::cout << "\nrequest on: " 
              << (*request).target() 
              << " accepted with body data: " 
              << beast::buffers_to_string((*this->request).body().data())
              << std::endl;

    this->jwt = new JWT_manager;
    server_status::JWT_status = jwt->validate_jwt_token(*this->request, this->request_data);
    
    std::cout << "\nwith header data: '"
              << this->request_data
              << "'\n";

    u_int64_t header_ID = 0;
    std::string header_login;
    std::string header_password;

    try {
        header_ID = std::stoi(std::string(this->request_data.at("ID")));
        header_login = this->request_data.at("login");
        header_password = this->request_data.at("password");
    } catch(nlohmann::json::exception &exception) {
        std::cout << exception.what() << std::endl;
    }

    this->data_base = new data_base_manager(header_ID, header_login, header_password);

    try {   
        this->request_data = nlohmann::json::parse(beast::buffers_to_string((*this->request).body().data()));
    } catch(nlohmann::json::exception &exeption) {
        std::cout << "body data is not valid for parsing" << std::endl;
    }
}

request_handler::~request_handler() {
    delete this->data_base;
    delete this->jwt;
}

auto request_handler::get_request_handler() -> void {
    (*this->response).result(http::status::ok);
    nlohmann::json response;
    
    if (!(*this->request).target().find("/LogginProfile/")) {
        try {
            auto login = this->request_data.at("login");
            auto password = this->request_data.at("password");
        
            struct profile profile;

            server_status::data_base_status = this->data_base->loggin_profile(login, password, profile);

            std::string result_jwt;

            server_status::JWT_status = jwt->create_jwt(profile.ID, profile.login, profile.password, result_jwt);

            if (server_status::data_base_status)
                result_jwt = "";
        
            response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
            response.push_back(nlohmann::json::object_t::value_type("JWT", result_jwt));
        } catch(boost::mysql::error_with_diagnostics &exception) {
            std::cout << exception.get_diagnostics().server_message() << std::endl;
            std::cout << exception.get_diagnostics().client_message() << std::endl;
            
            server_status::request_status = REQUEST_INVALID_REQUEST_BODY_DATA;
            response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
        }

        std::cout << "profile loggining response: " << response << std::endl;
    } else if (!(*this->request).target().find("/ProfileAuthentication/")) {
        profile result_profile;

        server_status::data_base_status = this->data_base->profile_authenticate(result_profile);
        
        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
    
        std::cout << "profile authentication response " << response << std::endl;
    } else if (!(*this->request).target().find("/GetProfile/")) {
        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
        response.push_back(nlohmann::json::object_t::value_type("profile", this->data_base->get_manager().to_json()));
 
        std::cout << "get profile response " << response << std::endl;
    } else if (!(*this->request).target().find("/GetProfilesWithSubstr/")) {
        auto prefix = this->request_data.at("substr");
        auto offset = this->request_data.at("offset");

        std::vector<profile> result_profiles;

        server_status::data_base_status = this->data_base->get_profiles_with_substr_in_name(prefix, offset, result_profiles);

        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));

        nlohmann::json profiles;

        for (auto profile : result_profiles) {
            profiles.push_back(profile.to_public_json());
        }

        response.push_back(nlohmann::json::object_t::value_type("profiles", profiles));

        std::cout << "find profiles with substr response " << response << std::endl;
    } else if (!(*this->request).target().find("/GetPublicProfile/")) {
        auto profile_ID = this->request_data.at("profile ID");

        profile result_profile;

        server_status::data_base_status = this->data_base->get_profile_by_ID(profile_ID, result_profile);
        
        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
        response.push_back(nlohmann::json::object_t::value_type("profile", result_profile.to_public_json()));
 
        std::cout << "get public profile response " << response << std::endl;
    } else if (!(*this->request).target().find("/GetProfileConfig/")) {
        config result_config;

        server_status::data_base_status = this->data_base->get_profile_config(result_config);

        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
        response.push_back(nlohmann::json::object_t::value_type("config", result_config.to_json()));

        std::cout << "get profile config" << response << std::endl;
    } else if (!(*this->request).target().find("/GetProfileRooms/")) {
        std::vector<room> result_rooms;

        server_status::data_base_status = this->data_base->get_profile_rooms(result_rooms);
        
        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
        
        nlohmann::json rooms;
        for (auto room : result_rooms) {
            rooms.push_back(room.to_json());
        }

        response.push_back(nlohmann::json::object_t::value_type("rooms", rooms));
            
        std::cout << "get profile rooms response " << response << std::endl;
    } else if (!(*this->request).target().find("/GetProfileTasks/")) {
        std::vector<task> result_tasks;

        server_status::data_base_status = this->data_base->get_profile_tasks(result_tasks);
        
        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));

        nlohmann::json tasks;
        for (auto task : result_tasks) {
            tasks.push_back(task.to_json());
        }

        response.push_back(nlohmann::json::object_t::value_type("tasks", tasks));

        std::cout << "get profile tasks response " << response << std::endl;
    } else if (!(*this->request).target().find("/GetProfileAssignedTasks/")) {
        std::vector<task> result_tasks;

        server_status::data_base_status = this->data_base->get_profile_assigned_tasks(result_tasks);
        
        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));

        nlohmann::json tasks;
        for (auto task : result_tasks) {
            tasks.push_back(task.to_json());
        }

        response.push_back(nlohmann::json::object_t::value_type("tasks", tasks));

        std::cout << "get profile assigned tasks reponse " << response << std::endl;         
    } else if (!(*this->request).target().find("/GetProfileReviewedTasks/")) {
        std::vector<task> result_tasks;

        server_status::data_base_status = this->data_base->get_profile_reviewed_tasks(result_tasks);
        
        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));

        nlohmann::json tasks;
        for (auto task : result_tasks) {
            tasks.push_back(task.to_json());
        }

        response.push_back(nlohmann::json::object_t::value_type("tasks", tasks));

        std::cout << "get profile reviewed tasks response " << response << std::endl;
    } else if (!(*this->request).target().find("/GetProfileReceivedInvites/")) {
        std::vector<invite> result_invites;

        server_status::data_base_status = this->data_base->get_profile_received_invites(result_invites);

        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));

        nlohmann::json invites;

        for (auto invite : result_invites) {
            invites.push_back(invite.to_json());
        }

        response.push_back(nlohmann::json::object_t::value_type("invites", invites));

        std::cout << "get profile received invites response " << response << std::endl;
    } else if (!(*this->request).target().find("/GetProfileSendedInvites/")) {
        std::vector<invite> result_invites;

        server_status::data_base_status = this->data_base->get_profile_sended_invites(result_invites);

        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));

        nlohmann::json invites;

        for (auto invite : result_invites) {
            invites.push_back(invite.to_json());
        }

        response.push_back(nlohmann::json::object_t::value_type("invites", invites));

        std::cout << "get profile sended invites response " << response << std::endl;
    } else if (!(*this->request).target().find("/GetRoom/")) {
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");

        struct room result_room;

        server_status::data_base_status = this->data_base->get_room(room_creator_ID, room_name, result_room);

        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
        response.push_back(nlohmann::json::object_t::value_type("room", result_room.to_json()));

        std::cout << "get room info response " << response << std::endl;
    } else if (!(*this->request).target().find("/GetRoomTasks/")) {
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");

        std::vector<task> result_tasks;

        server_status::data_base_status = this->data_base->get_room_tasks(room_creator_ID, room_name, result_tasks);
        
        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
        
        nlohmann::json tasks;
        for (auto task : result_tasks) {
            tasks.push_back(task.to_json());
        }

        response.push_back(nlohmann::json::object_t::value_type("tasks", tasks));

        std::cout << "get room tasks response " << response << std::endl;
    } else if (!(*this->request).target().find("/GetRoomProfiles/")) {
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");

        std::vector<profile> result_profiles;

        server_status::data_base_status = this->data_base->get_room_profiles(room_creator_ID, room_name, result_profiles);
        
        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));

        nlohmann::json profiles;
        for (auto profile : result_profiles) {
            profiles.push_back(profile.to_json());
        }

        response.push_back(nlohmann::json::object_t::value_type("profiles", profiles));

        std::cout << "get room profiles response " << response << std::endl;
    } else {
        (*this->response).result(http::status::not_found);
        (*this->response).set(http::field::content_type, "text/plain");
        
        response = "File not found\r\n";

        std::cout << "missed url response " << response << std::endl;
    }

    beast::ostream((*this->response).body()) << response;
}

auto request_handler::post_request_handler() -> void {
    (*this->response).result(http::status::ok);
    nlohmann::json response;

    if (!(*request).target().find("/CreateProfile/")) {
        auto name = this->request_data.at("name");
        auto login = this->request_data.at("login");
        auto password = this->request_data.at("password");
        auto email = this->request_data.at("email");
        auto phone = this->request_data.at("phone");

        profile profile;
        
        server_status::data_base_status = this->data_base->create_profile(name, login, password, email, phone, profile);
        
        std::string result_jwt;

        server_status::JWT_status = jwt->create_jwt(profile.ID, profile.login, profile.password, result_jwt);

        if (server_status::data_base_status)
            result_jwt = "";

        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
        response.push_back(nlohmann::json::object_t::value_type("JWT", result_jwt));
    
        std::cout << "profile creation response " << response << std::endl;
    } else if (!(*request).target().find("/CreateRoom/")) {
        auto room_name = this->request_data.at("room name");
        auto description = this->request_data.at("description");

        room result_room;

        server_status::data_base_status = this->data_base->create_room(room_name, description, result_room);

        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
        response.push_back(nlohmann::json::object_t::value_type("room", result_room.to_json()));
        
        std::cout << "room creating response " << response << std::endl;
    } else if (!(*request).target().find("/DeleteRoom/")) {
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");
        
        server_status::data_base_status = this->data_base->delete_room(room_creator_ID, room_name);

        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));

        std::cout << "room deleting response " << response << std::endl;
    } else if (!(*request).target().find("/CreateTask/")) {      
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");
        auto task_name = this->request_data.at("task name");
        auto description = this->request_data.at("description");
        auto label = this->request_data.at("label");
        int status = this->request_data.at("status");
        auto time_to_live = this->request_data.at("time to live");

        struct task task;

        server_status::data_base_status = this->data_base->create_task(room_creator_ID, room_name, task_name, description, label, status, time_to_live, task);
        
        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
        response.push_back(nlohmann::json::object_t::value_type("task", task.to_json()));
        
        std::cout << "task creating response " << response << std::endl;
    } else if (!(*request).target().find("/AddTaskToAssignee/")) {
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");
        auto task_name = this->request_data.at("task name");
        auto assignee_ID = this->request_data.at("assignee ID");

        server_status::data_base_status = this->data_base->add_task_to_assignee(room_creator_ID, room_name, task_name, assignee_ID);

        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));

        std::cout << "task added to assignee response " << response << std::endl;
    } else if (!(*request).target().find("/AddTaskToReviewer/")) {
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");
        auto task_name = this->request_data.at("task name");
        auto reviewer_ID = this->request_data.at("reviewer ID");

        server_status::data_base_status = this->data_base->add_task_to_reviewer(room_creator_ID, room_name, task_name, reviewer_ID);

        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
        
        std::cout << "task added to reviewer response " << response << std::endl;
    } else if (!(*request).target().find("/RemoveTaskFromAssignee/")) {
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");
        auto task_name = this->request_data.at("task name");
        auto assignee_ID = this->request_data.at("assigne ID");

        server_status::data_base_status = this->data_base->remove_task_from_assignee(room_creator_ID, room_name, task_name, assignee_ID);

        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
        
        std::cout << "task removed from assignee response " << response << std::endl;
    } else if (!(*request).target().find("/RemoveTaskFromReviewer/")) {
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");
        auto task_name = this->request_data.at("task name");
        auto reviewer_ID = this->request_data.at("reviewer ID");

        server_status::data_base_status = this->data_base->remove_task_from_reviewer(room_creator_ID, room_name, task_name, reviewer_ID);

        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
        
        std::cout << "task remove from reviewer response " << response << std::endl;
    } else if (!(*request).target().find("/DeleteTask/")) {
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");
        auto task_name = this->request_data.at("task name");
        
        server_status::data_base_status = this->data_base->delete_task(room_creator_ID, room_name, task_name);
        
        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
        
        std::cout << "task deleting response " << response << std::endl;
    }  else if (!(*request).target().find("/CreateInvite/")) {
        auto receiver_ID = this->request_data.at("receiver ID");
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");

        invite result_invite;

        server_status::data_base_status = this->data_base->create_invite(receiver_ID, room_creator_ID, room_name, result_invite);

        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
        response.push_back(nlohmann::json::object_t::value_type("invite", result_invite.to_json()));
        
        std::cout << "creating invite response " << response << std::endl;
    } else if (!(*request).target().find("/AcceptInvite/")) {
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");

        invite accepted_invite;

        server_status::data_base_status = this->data_base->accept_invite(room_creator_ID, room_name, accepted_invite);

        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
        response.push_back(nlohmann::json::object_t::value_type("invite", accepted_invite.to_json()));
        
        std::cout << "accepting invite response " << response << std::endl;
    } else if (!(*request).target().find("/DeleteSendedInvite/")) {
        auto receiver_ID = this->request_data.at("receiver ID");
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");

        invite deleted_invite;

        server_status::data_base_status = this->data_base->delete_sended_invite(receiver_ID, room_creator_ID, room_name, deleted_invite);
        
        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
        response.push_back(nlohmann::json::object_t::value_type("invite", deleted_invite.to_json()));
        
        std::cout << "deleting sended invite response " << response << std::endl;
    } else if (!(*request).target().find("/DeleteReceivedInvite/")) {
        auto sender_ID = this->request_data.at("sender ID");
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");

        invite deleted_invite;

        server_status::data_base_status = this->data_base->delete_received_invite(sender_ID, room_creator_ID, room_name, deleted_invite);

        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
        response.push_back(nlohmann::json::object_t::value_type("invite", deleted_invite.to_json()));

        std::cout << "deleting received invite response " << response << std::endl;
    } else {
        (*this->response).result(http::status::not_found);
        (*this->response).set(http::field::content_type, "text/plain");

        response = "File not found\r\n";
        
        std::cout << "missed url response " << response << std::endl;
    }

    beast::ostream((*this->response).body()) << response;
}

auto request_handler::delete_request_handler() -> void {
    (*this->response).result(http::status::ok);
    nlohmann::json response;

    if (!(*request).target().find("/DeleteProfile/")) {
        server_status::data_base_status = this->data_base->delete_profile();

        response.push_back(nlohmann::json::object_t::value_type("status", server_status::get_status()));
    
        std::cout << "profile deleting response " << response << std::endl;
    } else {
        (*this->response).result(http::status::not_found);
        (*this->response).set(http::field::content_type, "text/plain");
        
        response = "File not found\r\n";

        std::cout << "missed url response " << response << std::endl;
    }

    beast::ostream((*this->response).body()) << response;
}

auto request_handler::patch_request_handler() -> void {

}
