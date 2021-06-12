
#ifndef POSIX_File_Logger_Hpp
#define POSIX_File_Logger_Hpp

#include "logger_interface.hpp"

#include <boost/asio.hpp>
#include <boost/make_shared.hpp>

class posix_file_logger : public logger_interface {
	boost::asio::posix::stream_descriptor	m_output;
	std::string								m_path;

public:
	posix_file_logger(boost::asio::io_context &io, const std::string &path): m_output(io), m_path(path) {}
	~posix_file_logger() {
		close();
	}

	virtual void	open(const std::string &filename);
	virtual void	close();
	virtual void	log(std::string message);

private:
	void			handle_async_write(const boost::shared_ptr<std::string> &) {
	}

};

class posix_file_logger_factory : public logger_factory_interface {
	boost::asio::io_context	&m_io;
	std::string				m_path;

public:

	posix_file_logger_factory(boost::asio::io_context &io, const std::string &path): m_io(io), m_path(path) {}

	virtual boost::shared_ptr<logger_interface>	new_logger() {
		return (boost::make_shared<posix_file_logger>(m_io, m_path));
	}
};


#endif /* POSIX_File_Logger_Hpp */


