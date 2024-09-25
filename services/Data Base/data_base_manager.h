#pragma once
#include <boost/asio/ip/basic_resolver.hpp>
#include <boost/exception/exception.hpp>
#include <boost/mysql/error_code.hpp>
#include <boost/mysql/error_with_diagnostics.hpp>
#include <boost/url.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <jwt/algorithm.hpp>
#include <openssl/x509.h>
#include <jwt/jwt.hpp>
#include <boost/format.hpp>
#include <boost/mysql/tcp_ssl.hpp>
#include <stdlib.h>

class data_base_manager
{
    boost::asio::io_context ctx;
    boost::asio::ssl::context ssl_ctx;
    boost::mysql::tcp_ssl_connection conn;
    boost::asio::ip::tcp::resolver resolver;
    boost::mysql::results result;
    boost::mysql::execution_state state;
public:
    data_base_manager(const std::string &host = getenv("DATA_BASE_HOST"), const std::string &port = boost::mysql::default_port_string, const std::string &username = "myuser", const std::string &password = "secret", const std::string &db = "mydatabase");
    ~data_base_manager();

    auto login_in_profile(const std::string &login, const std::string &password) -> bool;
    auto create_profile(const std::string &login, const std::string &password) -> bool;
    auto delete_profile(const std::string &login, const std::string &password) -> void;
    auto drop_table(const std::string &name) -> void;
};