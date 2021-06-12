

#include "posix_file_logger.hpp"

#include <boost/bind/bind.hpp>
#include <unistd.h>

void	posix_file_logger::open(const std::string &filename) {
	std::string	correct_path = m_path;

	if (correct_path.length() > 0 && correct_path.back() != '/') {
		correct_path += '/';
	}
	int		fd = ::open((correct_path + filename).c_str(), O_WRONLY | O_NONBLOCK | O_CREAT | O_TRUNC, 0644);

	if (fd >= 0) {
		m_output.assign(fd);
	} else {
		perror("posix_file_logger::open()");
	}
}

void	posix_file_logger::close() {
	if (m_output.is_open()) {
		m_output.close();
	}
}

void	posix_file_logger::log(std::string message) {
	if (m_output.is_open()) {
		boost::shared_ptr<std::string>	shared_message = boost::make_shared<std::string>();
		shared_message->swap(message);
		boost::asio::async_write(m_output, boost::asio::buffer(*shared_message), boost::bind(&posix_file_logger::handle_async_write, this, shared_message));
	}
}

