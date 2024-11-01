#include "data_base_manager.h"
#include <boost/mysql/datetime.hpp>
#include <iostream>
#include <sys/types.h>

int main()
{
    data_base_manager manager("1", "1", "0.0.0.0", "3306", "myuser", "secret", "mydatabase");
    // data_base_manager manager;

    std::cout << manager.get_manager().to_json() << std::endl;

    std::cout << "profiles: \n";
    manager.print_tabel("profiles");
    std::cout << "rooms: \n";
    manager.print_tabel("rooms");
    std::cout << "profile_room: \n";
    manager.print_tabel("profile_room");
    std::cout << "tasks: \n";
    manager.print_tabel("tasks");
    std::cout << "configs: \n";
    manager.print_tabel("configs");
    std::cout << "assigneers: \n";
    manager.print_tabel("assignees");
    std::cout << "viewers: \n";
    manager.print_tabel("reviewers");

    std::cout << "=============================\n";

    struct profile profile;
    struct room room;
    struct task task;
    config config;
    std::vector<struct room> rooms;
    std::vector<struct task> tasks;
    std::vector<struct profile> profiles;

    // std::cout << manager.create_profile("C4H9", "123", "123", "s@mail.ru", "666", profile) << std::endl;
    // std::cout << manager.delete_profile() << std::endl;
    // std::cout << manager.get_profile_config(config) << std::endl;
    // std::cout << config.to_json() << std::endl;
    // std::cout << manager.update_profile_config("bebra", "color") << std::endl;
    // std::cout << manager.get_profile_config(config) << std::endl;
    // std::cout << config.to_json() << std::endl;
    // std::cout << manager.get_profile_config(config) << std::endl;
    // std::cout << config.to_json() << std::endl;
    // std::cout << manager.get_profile_by_ID(3, profile) << std::endl;
    // std::cout << manager.add_task_to_assignee(3, 3, "room1", "task1") << std::endl;
    // std::cout << manager.get_profile_assigned_tasks(tasks) << std::endl;
    // for (auto task : tasks) {
    //     std::cout << task.to_json() << std::endl;
    // }
    // std::cout << manager.remove_task_from_assignee(3, 3, "room1", "task1") << std::endl;
    // std::cout << manager.get_profile_assigned_tasks(tasks) << std::endl;
    // for (auto task : tasks) {
    //     std::cout << task.to_json() << std::endl;
    // }
    // std::cout << manager.add_task_to_reviewer(3, 3, "room1", "task1") << std::endl;
    // std::cout << manager.get_profile_reviewed_tasks(tasks) << std::endl;
    // for (auto task : tasks) {
    //     std::cout << task.to_json() << std::endl;
    // }
    // std::cout << manager.remove_task_from_reviewer(3, 3, "room1", "task1") << std::endl;
    // std::cout << manager.get_profile_reviewed_tasks(tasks) << std::endl;
    // for (auto task : tasks) {
    //     std::cout << task.to_json() << std::endl;
    // }
    // std::cout << profile.to_json() << std::endl;
    // std::cout << manager.create_room("room1", "description", room) << std::endl;
    // std::cout << manager.delete_room(3, "room1") << std::endl;
    // std::cout << manager.get_room(3, "room1", room) << std::endl;
    // std::cout << room.to_json() << std::endl;
    // std::cout << manager.create_task(1, "1", "5", "description", "in action", 1, 2, task) << std::endl;
    // std::cout << manager.delete_task(3, "room1", "task1") << std::endl;
    // std::cout << manager.get_task(1, "1", "5", task) << std::endl;
    // std::cout << task.to_json() << std::endl;
    // std::cout << manager.get_manager().to_json() << std::endl;
    // std::cout << manager.get_profile_rooms(rooms) << std::endl;
    // for (auto room : rooms) {
    //     std::cout << room.to_json() << std::endl;
    // }
    // std::cout << manager.get_profile_tasks(tasks) << std::endl;
    // for (auto task : tasks) {
    //     std::cout << task.to_json() << std::endl;
    // }
    // std::cout << manager.get_room_tasks(3, "room1", tasks) << std::endl;
    // for (auto task : tasks) {
    //     std::cout << task.to_json() << std::endl;
    // }
    // std::cout << manager.get_room_profiles(3, "room1", profiles) << std::endl;
    // for (auto profile : profiles) {
    //     std::cout << profile.to_json() << std::endl;
    // }
    // std::cout << manager.append_member_to_room(2, 3, "room1") << std::endl;

    std::cout << "=============================\n";

    std::cout << "profiles: \n";
    manager.print_tabel("profiles");
    std::cout << "rooms: \n";
    manager.print_tabel("rooms");
    std::cout << "profile_room: \n";
    manager.print_tabel("profile_room");
    std::cout << "tasks: \n";
    manager.print_tabel("tasks");
    std::cout << "configs: \n";
    manager.print_tabel("configs");
    std::cout << "assigneers: \n";
    manager.print_tabel("assignees");
    std::cout << "viewers: \n";
    manager.print_tabel("reviewers");

    return 0;
}