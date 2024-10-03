#include "data_base_manager.h"
#include <cinttypes>
#include <iostream>

int main()
{
    data_base_manager manager("0.0.0.0", "3306", "myuser", "secret", "mydatabase");

    std::cout << "users: \n";
    manager.print_tabel("users");
    std::cout << "rooms: \n";
    manager.print_tabel("rooms");
    std::cout << "user_room: \n";
    manager.print_tabel("user_room");
    std::cout << "tasks_room: \n";
    manager.print_tabel("tasks");

    std::cout << "=============================\n";

    // std::cout << manager.create_profile("1", "1") << std::endl;
    // std::cout << manager.create_room("1", "room1") << std::endl;
    // std::cout << manager.create_task("1", "task1", "1") << std::endl;
    
    // manager.delete_room("1", "room1");
    // manager.delete_profile("1", "1");
    // std::cout << manager.delete_task("1", "task1", "2") << std::endl;
    
    // std::cout << manager.append_member_to_room("2", "1", "room1");

    // auto res = manager.get_room_users("2");

    // auto res = manager.find_users_with_prefix_in_name("1");

    // for (auto i : res) {
    //     for (auto j : i) {
    //         std::cout << j << std::endl;
    //     }
    // }

    std::cout << "=============================\n";

    std::cout << "users: \n";
    manager.print_tabel("users");
    std::cout << "rooms: \n";
    manager.print_tabel("rooms");
    std::cout << "user_room: \n";
    manager.print_tabel("user_room");
    std::cout << "tasks_room: \n";
    manager.print_tabel("tasks");
    // std::cout << manager.get_user_id("11", "1") << std::endl;
    // std::cout << manager.get_user_id("1", "1") << std::endl;

    return 0;
}