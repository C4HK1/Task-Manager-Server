#include "request_handler.h"
#include "request_handler.h"
#include <boost/beast/http/message.hpp>
#include <boost/mysql/datetime.hpp>
#include <ctime>
#include <nlohmann/json_fwd.hpp>
#include <vector>

#include "request_handler.h"

request_handler::request_handler(http::request<http::dynamic_body> &request, http::response<http::dynamic_body> &response) : request(request),
                                                                                                                             response(response)
{
    this->status.JWT_status = jwt.validate_jwt_token(this->request, this->request_data);
    
    try {
        this->login = this->request_data.at("login");
        this->password = this->request_data.at("password");
    } catch(nlohmann::json::exception &exeption) {
    }

    this->request_data.push_back(nlohmann::json::parse(beast::buffers_to_string(this->request.body().data())));
    
    this->data_base = new data_base_manager(this->login, this->password);
}

request_handler::~request_handler() {
    delete this->data_base;
}

auto request_handler::get_request_handler() -> void {
    if (!this->request.target().find("/ProfileLogining")) {
        this->status.data_base_status = this->data_base->login_in_profile();

        std::string token;

        this->status.JWT_status = jwt.create_jwt(this->login, this->password, token);

        if (this->status.data_base_status)
            token = "";

        beast::ostream(this->response.body()) << R"%({"status": )%" 
                                                       << this->status.get_status() 
                                                       << R"%(, "JWT": ")%" 
                                                       << token << R"%("})%";
    
        std::cout << "profile loggining\n";
    } else if (!this->request.target().find("/GetProfileInfo")) {

        std::cout << "get profile info\n";
    } else if (!this->request.target().find("/GetRoomInfo")) {
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");

        struct room room;

        this->status.data_base_status = this->data_base->get_room(room_creator_ID, room_name, room);

        beast::ostream(this->response.body()) << R"%({"status": )%" 
                                                       << this->status.get_status()  
                                                       << R"%(, "room": )%" 
                                                       << room.to_json() << R"%(})%";
    
        std::cout << "get room info\n";
    } else if (!this->request.target().find("/GetProfileRooms")) {
        std::vector<room> rooms;

        this->status.data_base_status = this->data_base->get_profile_rooms(rooms);
        
        beast::ostream(this->response.body()) << R"%({"status": )%" 
                                                       << this->status.get_status()  
                                                       << R"%(, "profile rooms count": )%" 
                                                       << rooms.size() 
                                                       << R"%(, "items": [)%";

        for (int i = 0; i < rooms.size(); ++i) {
            beast::ostream(this->response.body()) << rooms[i].to_json();

            if (i != rooms.size() - 1) {
                beast::ostream(this->response.body()) << R"%(, )%";
            }
        }

        beast::ostream(this->response.body()) << R"%(]})%";
            
        std::cout << "get profile rooms\n";
    } else if (!this->request.target().find("/GetProfileTasks")) {
        std::vector<task> tasks;

        this->status.data_base_status = this->data_base->get_profile_tasks(tasks);
        
        beast::ostream(this->response.body()) << R"%({"status": )%" 
                                                       << this->status.get_status()  
                                                       << R"%(, "profile tasks count": )%" 
                                                       << std::to_string(tasks.size())
                                                       << R"%(, "items": [)%";

        for (int i = 0; i < tasks.size(); ++i) {
            beast::ostream(this->response.body()) << tasks[i].to_json();

            if (i != tasks.size() - 1) {
                beast::ostream(this->response.body()) << R"%(, )%";
            }
        }

        beast::ostream(this->response.body()) << R"%(]})%";

        std::cout << "get profile tasks\n";
    } else if (!this->request.target().find("/GetProfileConfig")) {
        config config;

        this->status.data_base_status = this->data_base->get_profile_config(config);

        beast::ostream(this->response.body()) << R"%({"status": )%" 
                                                       << this->status.get_status()  
                                                       << R"%(, "config": )%"
                                                       << config.to_json()
                                                       << R"%(})%";

        std::cout << "get profile config" << std::endl;
    } else if (!this->request.target().find("/GetProfileAssignedTasks")) {
        std::vector<task> tasks;

        this->status.data_base_status = this->data_base->get_profile_assigned_tasks(tasks);
        
        beast::ostream(this->response.body()) << R"%({"status": )%" 
                                                       << this->status.get_status()  
                                                       << R"%(, "profile assigned tasks count": )%" 
                                                       << std::to_string(tasks.size())
                                                       << R"%(, "items": [)%";

        for (int i = 0; i < tasks.size(); ++i) {
            beast::ostream(this->response.body()) << tasks[i].to_json();

            if (i != tasks.size() - 1) {
                beast::ostream(this->response.body()) << R"%(, )%";
            }
        }

        beast::ostream(this->response.body()) << R"%(]})%";

        std::cout << "get profile assigned tasks\n";
    } else if (!this->request.target().find("/GetProfileReviewedTasks")) {
        std::vector<task> tasks;

        this->status.data_base_status = this->data_base->get_profile_reviewed_tasks(tasks);
        
        beast::ostream(this->response.body()) << R"%({"status": )%" 
                                                       << this->status.get_status()  
                                                       << R"%(, "profile reviewed tasks count": )%" 
                                                       << std::to_string(tasks.size())
                                                       << R"%(, "items": [)%";

        for (int i = 0; i < tasks.size(); ++i) {
            beast::ostream(this->response.body()) << tasks[i].to_json();

            if (i != tasks.size() - 1) {
                beast::ostream(this->response.body()) << R"%(, )%";
            }
        }

        beast::ostream(this->response.body()) << R"%(]})%";

        std::cout << "get profile reviewed tasks\n";
    } else if (!this->request.target().find("/GetRoomTasks")) {
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");

        std::vector<task> tasks;

        this->status.data_base_status = this->data_base->get_room_tasks(room_creator_ID, room_name, tasks);
        
        beast::ostream(this->response.body()) << R"%({"status": )%" 
                                                       << this->status.get_status()  
                                                       << R"%(, "room tasks count": )%"
                                                       << std::to_string(tasks.size())
                                                       << R"%(, "items": [)%";

        for (int i = 0; i < tasks.size(); ++i) {
            beast::ostream(this->response.body()) << tasks[i].to_json();

            if (i != tasks.size() - 1) {
                beast::ostream(this->response.body()) << R"%(, )%";
            }
        }

        beast::ostream(this->response.body()) << R"%(]})%";

        std::cout << "get room tasks\n";
    } else if (!this->request.target().find("/GetRoomProfiles")) {
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");

        std::vector<profile> profiles;

        this->status.data_base_status = this->data_base->get_room_profiles(room_creator_ID, room_name, profiles);
        
        beast::ostream(this->response.body()) << R"%({"status": )%" 
                                                       << this->status.get_status()  
                                                       << R"%({"room profiles count": )%"
                                                       << std::to_string(profiles.size())
                                                       << R"%(, "items": [)%";

        for (int i = 0; i < profiles.size(); ++i) {
            beast::ostream(this->response.body()) << profiles[i].to_json();

            if (i != profiles.size() - 1) {
                beast::ostream(this->response.body()) << R"%(, )%";
            }
        }

        beast::ostream(this->response.body()) << R"%(]})%";

        std::cout << "get room profiles\n";
    } else if (!request.target().find("/FindProfilesWithPrefix")) {
        auto prefix = this->request_data.at("prefix");

        std::vector<profile> profiles;

        this->status.data_base_status = this->data_base->find_profiles_with_prefix_in_name(prefix, profiles);

        beast::ostream(this->response.body()) << R"%({"status": )%" 
                                                       << this->status.get_status()  
                                                       << R"%({"profiles count": )%"
                                                       << std::to_string(profiles.size())
                                                       << R"%(, "items": [)%";

        for (int i = 0; i < profiles.size(); ++i) {
            beast::ostream(this->response.body()) << profiles[i].to_json();

            if (i != profiles.size() - 1) {
                beast::ostream(this->response.body()) << R"%(, )%";
            }
        }

        beast::ostream(this->response.body()) << R"%(]})%";

        std::cout << "find profile with prefix\n";
    } else if (!this->request.target().find("/ProfileAuthentication")) {
        this->status.data_base_status = this->data_base->login_in_profile();
        
        beast::ostream(this->response.body()) << R"%({"status": )%" 
                                                       << this->status.get_status()  
                                                       << R"%(})%";
    
        std::cout << "profile authentication\n";
    } else {
        this->response.result(http::status::not_found);
        this->response.set(http::field::content_type, "text/plain");
        beast::ostream(this->response.body()) << "File not found\r\n";
    }
}

auto request_handler::post_request_handler() -> void {
    if (!request.target().find("/ProfileCreation")) {
        auto name = this->request_data.at("name");
        auto login = this->request_data.at("login");
        auto password = this->request_data.at("password");
        auto email = this->request_data.at("email");
        auto phone = this->request_data.at("phone");

        profile profile;
        
        this->status.data_base_status = this->data_base->create_profile(name, login, password, email, phone, profile);

        std::string token;

        this->status.JWT_status = jwt.create_jwt(this->login, this->password, token);

        if (this->status.data_base_status)
            token = "";

        beast::ostream(response.body()) << R"%({"status": )%" 
                                                 << (this->status.data_base_status | (this->status.JWT_status 
                                                 << DATA_BASE_SHIFT)) 
                                                 << R"%(, "JWT": ")%" 
                                                 << token 
                                                 << R"%("})%";
    
        std::cout << "profile creation";
    } else if (!request.target().find("/RoomCreation")) {
        auto room_name = this->request_data.at("room name");
        auto description = this->request_data.at("description");

        struct room room;

        this->status.data_base_status = this->data_base->create_room(room_name, description, room);

        beast::ostream(response.body()) << R"%({"status": )%" 
                                                 << this->status.get_status()  
                                                 << R"%(, "room": )%" 
                                                 << room.to_json() 
                                                 << R"%(})%";
        
        std::cout << "room creating\n";
    } else if (!request.target().find("/AppendMemberToRoom")) {
        auto member_ID = this->request_data.at("member ID");
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");

        this->status.data_base_status = this->data_base->append_member_to_room(member_ID, room_creator_ID, room_name);

        beast::ostream(response.body()) << R"%({"status": )%" 
                                                 << this->status.get_status()
                                                 << R"%(})%";
        
        std::cout << "appending member to room\n";
    } else if (!request.target().find("/TaskCreation")) {
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");
        auto task_name = this->request_data.at("task name");
        auto label = this->request_data.at("label");
        int status = this->request_data.at("status");
        auto time_to_live = this->request_data.at("time_to_live");

        struct task task;

        this->status.data_base_status = this->data_base->create_task(room_creator_ID, room_name, task_name, label, status, time_to_live, task);
        
        beast::ostream(response.body()) << R"%({"status": )%" 
                                                 << this->status.get_status()  
                                                 << R"%(, "task": )%" 
                                                 << task.to_json() 
                                                 << R"%(})%";
        
        std::cout << "task creating\n";
    } else if (!request.target().find("/AddTaskToAssignee")) {
        auto assignee_ID = this->request_data.at("assigne ID");
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");
        auto task_name = this->request_data.at("task name");

        this->status.data_base_status = this->data_base->add_task_to_assignee(assignee_ID, room_creator_ID, room_name, task_name);

        beast::ostream(response.body()) << R"%({"status": )%" 
                                                 << this->status.get_status()  
                                                 << R"%(})%";
        
        std::cout << "task added to assignee\n";
    } else if (!request.target().find("/AddTaskToReviewer")) {
        auto reviewer_ID = this->request_data.at("reviewer ID");
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");
        auto task_name = this->request_data.at("task name");

        this->status.data_base_status = this->data_base->add_task_to_reviewer(reviewer_ID, room_creator_ID, room_name, task_name);

        beast::ostream(response.body()) << R"%({"status": )%" 
                                                 << this->status.get_status()  
                                                 << R"%(})%";
        
        std::cout << "task added to reviewer\n";
    } else if (!request.target().find("/RoomDeleting")) {
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");
        
        this->status.data_base_status = this->data_base->delete_room(room_creator_ID, room_name);

        beast::ostream(response.body()) << R"%({"status": )%" 
                                                 << this->status.get_status()  
                                                 << R"%(})%";

        std::cout << "room deleting\n";
    } else if (!request.target().find("/RemoveTaskFromAssignee")) {
auto assignee_ID = this->request_data.at("assigne ID");
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");
        auto task_name = this->request_data.at("task name");

        this->status.data_base_status = this->data_base->remove_task_from_assignee(assignee_ID, room_creator_ID, room_name, task_name);

        beast::ostream(response.body()) << R"%({"status": )%" 
                                                 << this->status.get_status()  
                                                 << R"%(})%";
        
        std::cout << "task removed from assignee\n";
    } else if (!request.target().find("/RemoveTaskFromReviewer")) {
auto reviewer_ID = this->request_data.at("reviewer ID");
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");
        auto task_name = this->request_data.at("task name");

        this->status.data_base_status = this->data_base->remove_task_from_reviewer(reviewer_ID, room_creator_ID, room_name, task_name);

        beast::ostream(response.body()) << R"%({"status": )%" 
                                                 << this->status.get_status()  
                                                 << R"%(})%";
        
        std::cout << "task remove from reviewer\n";
    } else if (!request.target().find("/TaskDeleting")) {
        auto room_creator_ID = this->request_data.at("room creator ID");
        auto room_name = this->request_data.at("room name");
        auto task_name = this->request_data.at("task name");
        
        this->status.data_base_status = this->data_base->delete_task(room_creator_ID, room_name, task_name);
        
        beast::ostream(response.body()) << R"%({"status": )%" 
                                                 << this->status.get_status()  
                                                 << R"%(})%";
        
        std::cout << "task deleting\n";
    } else {
        response.result(http::status::not_found);
        response.set(http::field::content_type, "text/plain");
        beast::ostream(response.body()) << "File not found\r\n";
    }
}

auto request_handler::delete_request_handler() -> void {
    if (!request.target().find("/ProfileDeleting")) {
        this->status.data_base_status = this->data_base->delete_profile();

        beast::ostream(response.body()) << R"%({"status": )%" 
                                                 << this->status.get_status()  
                                                 << R"%(})%";
    
        std::cout << "profile deleting\n";
    } else {
        response.result(http::status::not_found);
        response.set(http::field::content_type, "text/plain");
        beast::ostream(response.body()) << "File not found\r\n";
    }
}

auto request_handler::patch_request_handler() -> void {

}