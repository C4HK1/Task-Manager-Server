#include <stdio.h>
#include <mysql/mysql.h>
#include <iostream>

MYSQL mysql;
MYSQL_RES *res;
MYSQL_ROW row;

void die(void){
   printf("%s\n", mysql_error(&mysql));
   exit(0);
}

int main() {
    // db.drop_table("users");
    // std::cout << db.create_profile("Andrew", "1234") << std::endl;
    // db.delete_profile("Andrew", "1234");
    // std::cout << db.login_in_profile("Andrew", "1234") << std::endl;

    unsigned int i = 0;
    if (!mysql_init(&mysql)) abort ();
    if (!(mysql_real_connect(&mysql,"0.0.0.0","myuser","secret", "mydatabase", 3306 , NULL , 0)))
       die();

    std::cout << "yapiyapi\n";

    mysql_close(&mysql);

    return 0;
}