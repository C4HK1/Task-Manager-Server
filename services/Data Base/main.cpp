#include "data_base_manager.h"
#include <cinttypes>
#include <iostream>
#include <mysql/mysql.h>


int main() {  
    data_base_manager manager("0.0.0.0", "3306", "myuser", "secret", "mydatabase");

    std::cout << "users: \n";
    manager.print_tabel("users");
    std::cout << "rooms: \n";
    manager.print_tabel("rooms");
    std::cout << "user_room: \n";
    manager.print_tabel("user_room");

    // std::cout << manager.create_profile("1", "1") << std::endl;
    // std::cout << manager.create_room("1", "room1") << std::endl;
    // manager.create_profile("1", "1");
    // manager.delete_room("1", "room1");
    // manager.delete_profile("1", "1");
    // std::cout << manager.create_profile("2", "1") << std::endl;
    // std::cout << manager.append_member_to_room("2", "1", "room1");

    std::cout << "users: \n";
    manager.print_tabel("users");
    std::cout << "rooms: \n";
    manager.print_tabel("rooms");
    std::cout << "user_room: \n";
    manager.print_tabel("user_room");
    // std::cout << manager.get_user_id("11", "1") << std::endl;
    // std::cout << manager.get_user_id("1", "1") << std::endl;

    return 0;
}