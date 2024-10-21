#include "request_handler.h"
#include "request_handler.h"
#include <boost/beast/http/message.hpp>

#include "../Request Handler/request_handler.h"

request_handler::request_handler(http::request<http::dynamic_body> &request, http::response<http::dynamic_body> &response) : request(request),
                                                                                                                             response(response)
{
    this->jwt_status = jwt.validate_jwt_token(this->request, this->request_data);
    
    this->request_data.push_back(nlohmann::json::parse(beast::buffers_to_string(this->request.body().data())));
    
    this->login = this->request_data.at("login");
    this->password = this->request_data.at("password");

    this->data_base = new data_base_manager(this->login, this->password);
}

request_handler::~request_handler() {
    delete this->data_base;
}

auto request_handler::get_request_handler() -> void {
    if (!this->request.target().find("/ProfileLogining")) {
        this->data_base_status = this->data_base->login_in_profile();

        std::string token;

        this->jwt_status = jwt.create_jwt(this->login, this->password, 60 * 60 * 24 * 7, token);

        if (this->data_base_status)
            token = "";

        beast::ostream(this->response.body()) << R"%({"status": )%" << (this->data_base_status | (this->jwt_status << DATA_BASE_SHIFT)) << R"%(, "JWT": ")%" << token << R"%("})%";
    
        std::cout << "profile loggining\n";
    } else if (!this->request.target().find("/GetProfileInfo")) {

        std::cout << "get profile info\n";
    } else if (!this->request.target().find("/GetRoomInfo")) {
        auto room_creator_id = this->request_data.at("room creator id");
        auto room_label = this->request_data.at("room label");

        struct room room;

        this->data_base_status = this->data_base->get_room(room_creator_id, room_label, room);

        beast::ostream(this->response.body()) << R"%({"status": )%" << (this->data_base_status | (this->jwt_status << DATA_BASE_SHIFT)) << R"%(, "room": )%" << room.to_json() << R"%(})%";
    
        std::cout << "get room info\n";
    } else if (!this->request.target().find("/GetProfileRooms")) {
        std::vector<room> rooms;

        this->data_base_status = this->data_base->get_profile_rooms(rooms);
        
        beast::ostream(this->response.body()) << R"%({"status": )%" << (this->data_base_status | (this->jwt_status << DATA_BASE_SHIFT)) << R"%(, "profile rooms count": )%" << rooms.size() << R"%(, "items": [)%";

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

        this->data_base_status = this->data_base->get_profile_tasks(tasks);
        
        beast::ostream(this->response.body()) << R"%({"status": )%" << (this->data_base_status | (this->jwt_status << DATA_BASE_SHIFT)) << R"%(, "profile tasks count": )%" << std::to_string(tasks.size()) + R"%(, "items": [)%";

        for (int i = 0; i < tasks.size(); ++i) {
            beast::ostream(this->response.body()) << tasks[i].to_json();

            if (i != tasks.size() - 1) {
                beast::ostream(this->response.body()) << R"%(, )%";
            }
        }

        beast::ostream(this->response.body()) << R"%(]})%";

        std::cout << "get profile tasks\n";
    } else if (!this->request.target().find("/GetRoomTasks")) {
        auto room_creator_id = this->request_data.at("room creator id");
        auto room_label = this->request_data.at("room label");

        std::vector<task> tasks;

        this->data_base_status = this->data_base->get_room_tasks(room_creator_id, room_label, tasks);
        
        beast::ostream(this->response.body()) << R"%({"status": )%" << (this->data_base_status | (this->jwt_status << DATA_BASE_SHIFT)) << R"%(, "room tasks count": )%" + std::to_string(tasks.size()) + R"%(, "items": [)%";

        for (int i = 0; i < tasks.size(); ++i) {
            beast::ostream(this->response.body()) << tasks[i].to_json();

            if (i != tasks.size() - 1) {
                beast::ostream(this->response.body()) << R"%(, )%";
            }
        }

        beast::ostream(this->response.body()) << R"%(]})%";

        std::cout << "get room tasks\n";
    } else if (!this->request.target().find("/GetRoomProfiles")) {
        auto room_creator_id = this->request_data.at("room creator id");
        auto room_label = this->request_data.at("room label");

        std::vector<profile> profiles;

        this->data_base_status = this->data_base->get_room_profiles(room_creator_id, room_label, profiles);
        
        beast::ostream(this->response.body()) << R"%({"status": )%" << (this->data_base_status | (this->jwt_status << DATA_BASE_SHIFT)) << R"%({"room profiles count": )%" + std::to_string(profiles.size()) + R"%(, "items": [)%";

        for (int i = 0; i < profiles.size(); ++i) {
            beast::ostream(this->response.body()) << profiles[i].to_json();

            if (i != profiles.size() - 1) {
                beast::ostream(this->response.body()) << R"%(, )%";
            }
        }

        beast::ostream(this->response.body()) << R"%(]})%";

        std::cout << "get room profiles\n";
    } else if (!this->request.target().find("/ProfileAuthentication")) {
        this->data_base_status = this->data_base->login_in_profile();
        
        beast::ostream(this->response.body()) << R"%({"status": )%" << (this->data_base_status | (this->jwt_status << DATA_BASE_SHIFT)) << R"%(})%";
    
        std::cout << "profile authentication\n";
    } else {
        this->response.result(http::status::not_found);
        this->response.set(http::field::content_type, "text/plain");
        beast::ostream(this->response.body()) << "File not found\r\n";
    }
}

auto request_handler::post_request_handler() -> void {
    if (!request.target().find("/ProfileCreation")) {
        std::string name = request_data.at("name");

        struct profile profile;
        
        this->data_base_status = this->data_base->create_profile(name, this->login, this->password, profile);

        std::string token;

        this->jwt_status = jwt.create_jwt(this->login, this->password, 60 * 60 * 24 * 7, token);

        if (this->data_base_status)
            token = "";

        beast::ostream(response.body()) << R"%({"status": )%" << (this->data_base_status | (this->jwt_status << DATA_BASE_SHIFT)) << R"%(, "JWT": ")%" << token << R"%("})%";
    
        std::cout << "profile creation";
    } else if (!request.target().find("/RoomCreation")) {
        std::string room_label = request_data.at("room label");

        struct room room;

        this->data_base_status = this->data_base->create_room(room_label, room);

        beast::ostream(response.body()) << R"%({"status": )%" << (this->data_base_status | (this->jwt_status << DATA_BASE_SHIFT)) << R"%(, "room": )%" << room.to_json() << R"%(})%";
        
        std::cout << "room creating\n";
    } else if (!request.target().find("/TaskCreation")) {
        std::string room_creator_id = request_data.at("room creator id");
        std::string room_label = request_data.at("room label");
        std::string task_label = request_data.at("task label");

        struct task task;

        this->data_base_status = this->data_base->create_task(room_creator_id, room_label, task_label, task);
        
        beast::ostream(response.body()) << R"%({"status": )%" << (this->data_base_status | (this->jwt_status << DATA_BASE_SHIFT)) << R"%(, "task": )%" << task.to_json() << R"%(})%";
        
        std::cout << "task creating\n";
    } else if (!request.target().find("/RoomDeleting")) {
        auto room_creator_id = request_data.at("room creator id");
        auto room_label = request_data.at("room label");
        
        this->data_base_status = this->data_base->delete_room(room_creator_id, room_label);

        beast::ostream(response.body()) << R"%({"status": )%" << (this->data_base_status | (this->jwt_status << DATA_BASE_SHIFT)) << R"%(})%";

        std::cout << "room deleting\n";
    } else if (!request.target().find("/TaskDeleting")) {
        auto room_creator_id = request_data.at("room creator id");
        auto room_label = request_data.at("room label");
        auto task_label = request_data.at("task label");
        
        this->data_base_status = this->data_base->delete_task(room_creator_id, room_label, task_label);
        
        beast::ostream(response.body()) << R"%({"status": )%" << (this->data_base_status | (this->jwt_status << DATA_BASE_SHIFT)) << R"%(})%";
        
        std::cout << "task deleting\n";
    } else {
        response.result(http::status::not_found);
        response.set(http::field::content_type, "text/plain");
        beast::ostream(response.body()) << "File not found\r\n";
    }
}

auto request_handler::delete_request_handler() -> void {
    if (!request.target().find("/ProfileDeleting")) {
        this->data_base_status = this->data_base->delete_profile();

        beast::ostream(response.body()) << R"%({"status": )%" << (this->data_base_status | (this->jwt_status << DATA_BASE_SHIFT)) << R"%(})%";
    
        std::cout << "profile deleting\n";
    } else {
        response.result(http::status::not_found);
        response.set(http::field::content_type, "text/plain");
        beast::ostream(response.body()) << "File not found\r\n";
    }
}

auto request_handler::patch_request_handler() -> void {

}