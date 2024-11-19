#include <boost/exception/exception.hpp>
#include <boost/url.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <jwt/algorithm.hpp>
#include <openssl/x509.h>
#include <jwt/jwt.hpp>

#include "server/http_connection.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// namespace my_program_state
// {
//     std::size_t
//     request_count()
//     {
//         static std::size_t count = 0;
//         return ++count;
//     }

//     std::time_t
//     now()
//     {
//         return std::time(0);
//     }
// }

// "Loop" forever accepting new connections.
void http_server(tcp::acceptor& acceptor, tcp::socket& socket) {
    acceptor.async_accept(
                    socket,
                    [&](beast::error_code ec) {
                               if(!ec)
                                   std::make_shared<server::http_connection>(std::move(socket))->start();
                               http_server(acceptor, socket);
                           }
                         );
}

int main(int argc, char* argv[]) {
    try {
        // Check command line arguments.
        if(argc != 3) {
            std::cerr << "Usage: " << argv[0] << " <address> <port>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    receiver 0.0.0.0 80\n";
            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    receiver 0::0 80\n";
            return EXIT_FAILURE;
        }

        auto const address = net::ip::make_address(argv[1]);
        unsigned short port = static_cast<unsigned short>(std::atoi(argv[2]));

        net::io_context ioc{1};

        tcp::acceptor acceptor{ioc, {address, port}};
        tcp::socket socket{ioc};
        http_server(acceptor, socket);

        ioc.run();
    } catch(std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
