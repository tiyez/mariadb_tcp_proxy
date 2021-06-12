

#include "tcp_proxy.hpp"
#include "mariadb_listener.hpp"
#include "posix_file_logger.hpp"

#include <exception>
#include <iostream>

using boost::asio::ip::tcp;

int		main(int arg_count, char *args[]) {
	int		status = 0;

	if (arg_count != 2) {
		std::cout << "Usage: " << args[0] << " <mariadb_host>" << std::endl;
		status = 1;
	} else {
		try {
			boost::asio::io_context	io;
			tcp_proxy	proxy(io, args[1], "3306", tcp::endpoint(tcp::v4(), 3306), boost::make_shared<mariadb_listener_factory>(boost::make_shared<posix_file_logger_factory>(io, "./")));
			io.run();
		} catch (std::exception &e) {
			std::cerr << e.what() << std::endl;
			status = 1;
		}
	}
	return (status);
}
