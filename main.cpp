#include "Polyweb/polyweb.hpp"
#include <iostream>
#include <string.h>

std::string sockaddr_to_string(const struct sockaddr* addr) {
    std::string ret;
    switch (addr->sa_family) {
    case AF_INET: {
        struct sockaddr_in inet_addr;
        memcpy(&inet_addr, addr, sizeof inet_addr);
        pn::inet_ntop(AF_INET, &inet_addr.sin_addr, ret);
        break;
    }

    case AF_INET6: {
        struct sockaddr_in6 inet6_addr;
        memcpy(&inet6_addr, addr, sizeof inet6_addr);
        pn::inet_ntop(AF_INET6, &inet6_addr.sin6_addr, ret);
        break;
    }

    default:
        return "Unknown address family";
    }
    return ret;
}

int main(int argc, char* argv[]) {
    pn::init();

    pn::UniqueSocket<pw::Server> server;

    unsigned short port;
    if (argc >= 2) {
        port = atoi(argv[1]);
    } else {
        port = 80;
    }

    server->route("/",
        pw::HTTPRoute {
            [](const pw::Connection& conn, const pw::HTTPRequest& req, void* data) {
                std::cout << '[' << pw::build_date() << "] " << sockaddr_to_string(&conn.addr) << " - \"" << req.method << ' ' << req.target << ' ' << req.http_version << "\"" << std::endl;

                pw::URLInfo url_info;
                url_info.scheme = "https";
                url_info.host = req.headers.at("Host");
                url_info.path = req.target;
                url_info.query_parameters = req.query_parameters;

                return pw::HTTPResponse(301, {{"Location", url_info.build()}});
            },
            nullptr,
            true,
        });

    std::cout << "⚔️ Securitas is binding to port " << port << "..." << std::endl;
    if (server->bind("0.0.0.0", port) == PN_ERROR) {
        std::cerr << "❌ Error: " << pn::universal_strerror() << std::endl;
        return 1;
    }

    std::cout << "🛡️ Securitas is listening on port " << port << std::endl;
    if (server->listen() == PN_ERROR) {
        std::cerr << "❌ Error: " << pw::universal_strerror() << std::endl;
        return 1;
    }

    pn::quit();
    return 0;
}