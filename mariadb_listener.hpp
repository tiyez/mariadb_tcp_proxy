

#ifndef MariaDB_Listener_Hpp
#define MariaDB_Listener_Hpp

#include "tcp_proxy.hpp"
#include "logger_interface.hpp"

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <string>

class mariadb_listener : public tcp_proxy_session_listener_interface {

	enum class connection_establishing_state {
		Initial_Handshake_From_Server,
		Handshake_Response_From_Client,
		Response_From_Server_With_Ok_Or_Auth_Data,
		Failure,
		Failure_SSL_Switch,
		Success,
	} m_connection_establishing_state;

	enum server_capabilities {
		NONE = 0,
		CLIENT_DEPRECATE_EOF = 1 << 24,
	};

	int			m_server_capabilities;

	std::string	m_username;
	int			m_connection_id;

	boost::shared_ptr<logger_interface>	m_logger;

	bool		m_log_response_from_server;

public:
	mariadb_listener(boost::shared_ptr<logger_interface> logger = nullptr)
		: m_connection_establishing_state(connection_establishing_state::Initial_Handshake_From_Server)
		, m_server_capabilities(server_capabilities::NONE)
		, m_connection_id(-1)
		, m_logger(logger)
		, m_log_response_from_server(false)
	{}
	~mariadb_listener() {
		if (m_logger && m_connection_establishing_state == connection_establishing_state::Success) {
			m_logger->close();
		}
	}

private:
	virtual void	received_from_client(const boost::asio::const_buffer &buffer);
	virtual void	received_from_server(const boost::asio::const_buffer &buffer);

	bool	parse_initial_handshake(const boost::asio::const_buffer &buffer);
	bool	parse_handshake_response_packet(const boost::asio::const_buffer &buffer);

	bool	is_ok_packet(const boost::asio::const_buffer &buffer);
	bool	is_err_packet(const boost::asio::const_buffer &buffer);
	bool	is_com_query(const boost::asio::const_buffer &buffer);

	void	parse_and_log_sql_query(const boost::asio::const_buffer &buffer);

	std::string	build_log_filename();
};

class mariadb_listener_factory : public tcp_proxy_listener_factory_interface {
	boost::shared_ptr<logger_factory_interface>	m_logger_factory;

public:
	mariadb_listener_factory(boost::shared_ptr<logger_factory_interface> logger_factory = nullptr): m_logger_factory(logger_factory) {}

	virtual boost::shared_ptr<tcp_proxy_session_listener_interface>	new_listener() {
		return (boost::make_shared<mariadb_listener>(m_logger_factory ? m_logger_factory->new_logger() : nullptr));
	}
};

#endif /* MariaDB_Listener_Hpp */
