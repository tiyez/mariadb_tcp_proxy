
#include "tcp_proxy.hpp"

#include <boost/bind/bind.hpp>
#include <iostream>

using boost::asio::ip::tcp;

void	tcp_proxy_session::start() {
	try {
		boost::asio::connect(m_server_socket, m_server_endpoints);

		m_client_socket.async_read_some(boost::asio::buffer(m_client_buffer), boost::bind(&tcp_proxy_session::handle_client_read, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		m_server_socket.async_read_some(boost::asio::buffer(m_server_buffer), boost::bind(&tcp_proxy_session::handle_server_read, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

void	tcp_proxy_session::handle_client_read(const boost::system::error_code &error, size_t readed) {
	if (!error) {
		boost::asio::const_buffer	buffer(boost::asio::buffer(m_client_buffer, readed));

		if (m_listener) {
			m_listener->received_from_client(buffer);
		}
		boost::asio::async_write(m_server_socket, buffer, boost::bind(&tcp_proxy_session::handle_client_write, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	} else {
		if (m_client_socket.is_open()) {
			m_client_socket.close();
		}
	}
}

void	tcp_proxy_session::handle_server_read(const boost::system::error_code &error, size_t readed) {
	if (!error) {
		boost::asio::const_buffer	buffer(boost::asio::buffer(m_server_buffer, readed));

		if (m_listener) {
			m_listener->received_from_server(buffer);
		}
		boost::asio::async_write(m_client_socket, buffer, boost::bind(&tcp_proxy_session::handle_server_write, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	} else {
		if (m_server_socket.is_open()) {
			m_server_socket.close();
		}
	}
}

void	tcp_proxy_session::handle_client_write(const boost::system::error_code &error, size_t written) {
	if (!error) {
		if (m_server_socket.is_open()) {
			m_client_socket.async_read_some(boost::asio::buffer(m_client_buffer), boost::bind(&tcp_proxy_session::handle_client_read, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
	} else {
		std::cerr << boost::system::system_error(error).what() << std::endl;
	}
}

void	tcp_proxy_session::handle_server_write(const boost::system::error_code &error, size_t written) {
	if (!error) {
		if (m_client_socket.is_open()) {
			m_server_socket.async_read_some(boost::asio::buffer(m_server_buffer), boost::bind(&tcp_proxy_session::handle_server_read, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		}
	} else {
		std::cerr << boost::system::system_error(error).what() << std::endl;
	}
}


void	tcp_proxy::start_accept() {
	tcp_proxy_session::pointer	new_session = tcp_proxy_session::create(m_io, m_host, m_service, m_factory ? m_factory->new_listener() : nullptr);
	m_acceptor.async_accept(new_session->client_socket(), boost::bind(&tcp_proxy::handle_accept, this, new_session, boost::asio::placeholders::error));
}

void	tcp_proxy::handle_accept(tcp_proxy_session::pointer new_session, const boost::system::error_code &error) {
	if (!error) {
		new_session->start();
	} else {
		std::cerr << boost::system::system_error(error).what() << std::endl;
	}
	start_accept();
}
