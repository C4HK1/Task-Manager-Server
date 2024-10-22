#include "data_base_manager.h"
#include <iostream>

int main()
{
    data_base_manager manager("123", "123", "0.0.0.0", "3306", "myuser", "secret", "mydatabase");

    std::cout << "profiles: \n";
    manager.print_tabel("profiles");
    std::cout << "rooms: \n";
    manager.print_tabel("rooms");
    std::cout << "profile_room: \n";
    manager.print_tabel("profile_room");
    std::cout << "tasks: \n";
    manager.print_tabel("tasks");

    std::cout << "=============================\n";

    struct profile profile;
    struct room room;
    struct task task;
    std::vector<struct room> rooms;
    std::vector<struct task> tasks;
    std::vector<struct profile> profiles;
    // std::cout << manager.create_profile("LENYA") << std::endl;
    // std::cout << manager.delete_profile() << std::endl;
    // std::cout << manager.get_profile_by_ID(1, profile) << std::endl;
    // std::cout << profile.to_json() << std::endl;
    // std::cout << manager.create_room("room1", room) << std::endl;
    // std::cout << manager.delete_room(1, "room1") << std::endl;
    // std::cout << manager.get_room(1, "room1", room) << std::endl;
    // std::cout << room.to_json() << std::endl;
    // std::cout << manager.create_task(1, "room1", "task1", task) << std::endl;
    // std::cout << manager.delete_task(1, "room1", "task1") << std::endl;
    // std::cout << manager.get_task(1, "room1", "task1", task) << std::endl;
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
    // std::cout << manager.get_room_tasks(1, "room1", tasks) << std::endl;
    // for (auto task : tasks) {
    //     std::cout << task.to_json() << std::endl;
    // }
    // std::cout << manager.get_room_profiles(1, "room1", profiles) << std::endl;
    // for (auto profile : profiles) {
    //     std::cout << profile.to_json() << std::endl;
    // }
    // std::cout << manager.append_member_to_room(2, 1, "room1") << std::endl;

    std::cout << "=============================\n";

    std::cout << "profiles: \n";
    manager.print_tabel("profiles");
    std::cout << "rooms: \n";
    manager.print_tabel("rooms");
    std::cout << "profile_room: \n";
    manager.print_tabel("profile_room");
    std::cout << "tasks: \n";
    manager.print_tabel("tasks");

    return 0;
}