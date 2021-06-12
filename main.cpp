

#include "tcp_proxy.hpp"
#include "mariadb_listener.hpp"
#include "posix_file_logger.hpp"

#include <exception>
#include <iostream>

using boost::asio::ip::tcp;

int		main() {
	try {
		boost::asio::io_context	io;
		tcp_proxy	proxy(io, "mariadb", "3306", tcp::endpoint(tcp::v4(), 3306), boost::make_shared<mariadb_listener_factory>(boost::make_shared<posix_file_logger_factory>(io, "./")));
		io.run();
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}
