
#ifndef TCP_Proxy_Hpp
#define TCP_Proxy_Hpp

#include <string>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

class tcp_proxy_session_listener_interface {
public:
	virtual void	received_from_client(const boost::asio::const_buffer &buffer) = 0;
	virtual void	received_from_server(const boost::asio::const_buffer &buffer) = 0;
	virtual ~tcp_proxy_session_listener_interface() {};
};

class tcp_proxy_listener_factory_interface {
public:
	virtual boost::shared_ptr<tcp_proxy_session_listener_interface>	new_listener() = 0;
	virtual ~tcp_proxy_listener_factory_interface() {};
};

template <class Listener>
class simple_tcp_proxy_listener_factory : public tcp_proxy_listener_factory_interface {
public:
	virtual boost::shared_ptr<tcp_proxy_session_listener_interface>	new_listener() {
		return (boost::make_shared<Listener>());
	}
};

class tcp_proxy_session : public boost::enable_shared_from_this<tcp_proxy_session>
{
public:
	enum constants {
		Buffer_Size = 4 * 1024,
	};

private:
	boost::asio::ip::tcp::socket				m_client_socket;
	boost::asio::ip::tcp::socket				m_server_socket;
	boost::array<unsigned char, Buffer_Size>	m_client_buffer;
	boost::array<unsigned char, Buffer_Size>	m_server_buffer;

	boost::asio::ip::tcp::resolver::results_type					m_server_endpoints;

	boost::shared_ptr<tcp_proxy_session_listener_interface>	m_listener;

public:
	typedef boost::shared_ptr<tcp_proxy_session>	pointer;

	static pointer	create(boost::asio::io_context &io, const char *host, const char *service, boost::shared_ptr<tcp_proxy_session_listener_interface> listener = nullptr) {
		return pointer(new tcp_proxy_session(io, host, service, listener));
	}

	boost::asio::ip::tcp::socket	&client_socket() {
		return m_client_socket;
	}

	void	start();

private:
	tcp_proxy_session(boost::asio::io_context &io, const char *host, const char *service, boost::shared_ptr<tcp_proxy_session_listener_interface> listener)
		: m_client_socket(io)
		, m_server_socket(io)
		, m_server_endpoints(boost::asio::ip::tcp::resolver(io).resolve(host, service))
		, m_listener(listener)
	{
	}

	void	handle_client_read(const boost::system::error_code &error, size_t readed);
	void	handle_server_read(const boost::system::error_code &error, size_t readed);
	void	handle_client_write(const boost::system::error_code &error, size_t written);
	void	handle_server_write(const boost::system::error_code &error, size_t written);

};

class tcp_proxy {
	boost::asio::io_context			&m_io;
	boost::asio::ip::tcp::acceptor	m_acceptor;
	const char						*m_host;
	const char						*m_service;
	boost::shared_ptr<tcp_proxy_listener_factory_interface>	m_factory;

public:

	tcp_proxy(boost::asio::io_context &io, const char *host, const char *service, const boost::asio::ip::tcp::endpoint &endpoint, boost::shared_ptr<tcp_proxy_listener_factory_interface> factory = nullptr)
		: m_io(io)
		, m_acceptor(io, endpoint)
		, m_host(host)
		, m_service(service)
		, m_factory(factory)
	{
		start_accept();
	}

private:

	void	start_accept();
	void	handle_accept(tcp_proxy_session::pointer new_session, const boost::system::error_code &error);
};

#endif /* TCP_Proxy_Hpp */
