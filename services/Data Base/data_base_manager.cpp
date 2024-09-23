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
#include <jwt/algorithm.hpp>
#include <openssl/x509.h>
#include <jwt/jwt.hpp>
#include <boost/format.hpp>
#include <boost/mysql/tcp_ssl.hpp>

#include "data_base_manager.h"

using namespace boost::mysql;

data_base_manager::data_base_manager(const std::string &host, const std::string &port, const std::string &username, const std::string &password, const std::string &db) :  
                                                                                                                                                                        ssl_ctx(boost::asio::ssl::context(boost::asio::ssl::context::tls_client)),
                                                                                                                                                                        conn(boost::mysql::tcp_ssl_connection(ctx.get_executor(), ssl_ctx)),
                                                                                                                                                                        resolver(boost::asio::ip::tcp::resolver(ctx.get_executor()))
{
    auto endpoints = resolver.resolve(host, port);

    boost::mysql::handshake_params params(
        username ,
        password,
        db
    );

    conn.connect(*endpoints.begin(), params);
}

data_base_manager::~data_base_manager()
{
    conn.close();
}

auto data_base_manager::drop_table(const std::string &name) -> void
{
    try {
        conn.query("DROP TABLE " + name, result);
    } catch (boost::mysql::error_with_diagnostics &exception) {}
}

auto data_base_manager::login_in_profile(const std::string &login, const std::string &password) -> bool
{
    try {
        conn.start_query("SELECT login, password FROM users WHERE login = '" + login + "'AND password = '" + password + "'", state);

        auto result = conn.read_some_rows(state);

        if (!result.size())
            return false;
    } catch(boost::mysql::error_with_diagnostics &exception) {
        return false;
    }
   
    return true;
}

auto data_base_manager::create_profile(const std::string &login, const std::string &password) -> bool
{
    try {
        conn.start_query("SELECT login, password FROM users WHERE login = '" + login + "'", state);

        if (conn.read_some_rows(state).size())
            return false;
    } catch(boost::mysql::error_with_diagnostics &exception) {
        conn.query(
            R"%(
                CREATE TABLE users (
                    id INT PRIMARY KEY AUTO_INCREMENT,
                    login TEXT,
                    password TEXT
                )
            )%",
            result
        );
    }

    conn.query(
        "INSERT INTO users (login, password) VALUES ('" + login + "', '" + password + "')",
        result
    );

    return true;
}

auto data_base_manager::delete_profile(const std::string &login, const std::string &password) -> void
{
    try {
        conn.start_query("DELETE FROM users WHERE login = '" + login + "'AND password = '" + password + "'", state);
    } catch(boost::mysql::error_with_diagnostics &exception) {}
}