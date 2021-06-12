
#include "mariadb_listener.hpp"

#include <sstream>
#include <ctime>

void	mariadb_listener::received_from_client(const boost::asio::const_buffer &buffer) {
	if (m_connection_establishing_state == connection_establishing_state::Success) {
		if (is_com_query(buffer)) {
			parse_and_log_sql_query(buffer);
		}
	} else {
		switch (m_connection_establishing_state) {
			case connection_establishing_state::Handshake_Response_From_Client: {
				if (parse_handshake_response_packet (buffer)) {
					m_connection_establishing_state = connection_establishing_state::Response_From_Server_With_Ok_Or_Auth_Data;
				} else {
					m_connection_establishing_state = connection_establishing_state::Failure_SSL_Switch;
				}
			} break ;
		}
	}
}

void	mariadb_listener::received_from_server(const boost::asio::const_buffer &buffer) {
	if (m_connection_establishing_state == connection_establishing_state::Success) {
		if (m_log_response_from_server && m_logger) {
			if (is_ok_packet(buffer)) {
				m_logger->log(" [OK]\n");
			} else if (is_err_packet(buffer)) {
				m_logger->log(" [ERR]\n");
			} else {
				m_logger->log(" [DATA]\n");
			}
			m_log_response_from_server = false;
		}
	} else {
		switch (m_connection_establishing_state) {
			case connection_establishing_state::Initial_Handshake_From_Server: {
				if (parse_initial_handshake(buffer)) {
					m_connection_establishing_state = connection_establishing_state::Handshake_Response_From_Client;
				} else {
					m_connection_establishing_state = connection_establishing_state::Failure;
				}
			} break ;
			case connection_establishing_state::Response_From_Server_With_Ok_Or_Auth_Data: {
				if (is_ok_packet(buffer)) {
					m_connection_establishing_state = connection_establishing_state::Success;
					/* connection established */
					if (m_logger) {
						m_logger->open(build_log_filename());
					}
				} else if (is_err_packet(buffer)) {
					m_connection_establishing_state = connection_establishing_state::Failure;
				} else {
					/* auth data. do nothing */
				}
			} break ;
		}
	}
}

bool	mariadb_listener::parse_initial_handshake(const boost::asio::const_buffer &buffer) {
	const char	*data = static_cast<const char *>(buffer.data());
	size_t		index = 4; /* skip packet header */
	bool		success = false;

	index += 1; /* skip protocol version (maybe throw exception if version does not match) */
	while (index < buffer.size() && data[index] != 0) {
		/* skip server version (maybe check '5.5.5-' prefix to ensure that this is mariadb database) */
		index += 1;
	}
	index += 1; /* skip null-term */
	if (index + 4 <= buffer.size()) {
		static_assert(sizeof (int) == 4, "");
		m_connection_id = *reinterpret_cast<const int *>(data + index);

		index += 4;
		index += 8; /* skip scramble 1st part (authentication seed) */
		index += 1; /* skip reserved byte */

		if (index + 2 <= buffer.size()) {
			static_assert(sizeof (unsigned short) == 2, "");
			int	server_capabilities_part1 = (int) *reinterpret_cast<const unsigned short *>(data + index);
			m_server_capabilities = server_capabilities_part1;

			index += 2; /* skip server server_capabilities (1st part) */
			index += 1; /* skip server default collation */
			index += 2; /* skip status flags */

			if (index + 2 <= buffer.size()) {
				int	server_capabilities_part2 = (int) *reinterpret_cast<const unsigned short *>(data + index) << 16;
				m_server_capabilities |= server_capabilities_part2;
				success = true;
			} else {
			}
		} else {
		}
	} else {
		/* SSL switch */
	}
	return (success);
}

bool	mariadb_listener::parse_handshake_response_packet(const boost::asio::const_buffer &buffer) {
	bool		success = false;
	size_t		index = 4; /* skip packet header */

	if (index + 4 <= buffer.size()) {
		int client_server_capabilities = *reinterpret_cast<const int *>(static_cast<const char *>(buffer.data()) + index);

		m_server_capabilities &= client_server_capabilities;

		index += 4; /* skip client server_capabilities */
		index += 4; /* skip max packet size */
		index += 1; /* skip client character collation */
		index += 19; /* skip reserved */
		index += 4; /* skip extended client server_capabilities */
		if (index < buffer.size()) {
			m_username = std::string(static_cast<const char *>(buffer.data()) + index);
			success = true;
		} else {
		}
	} else {
	}
	return (success);
}

bool	mariadb_listener::is_ok_packet(const boost::asio::const_buffer &buffer) {
	bool	success = false;

	if (buffer.size() >= 4 + 1) {
		if (m_server_capabilities & server_capabilities::CLIENT_DEPRECATE_EOF) {
			success = (static_cast<const unsigned char *>(buffer.data())[4] == 0xFE);
		} else {
			success = (static_cast<const unsigned char *>(buffer.data())[4] == 0x00);
		}
	}
	return (success);
}

bool	mariadb_listener::is_err_packet(const boost::asio::const_buffer &buffer) {
	bool	success = false;

	if (buffer.size() >= 4 + 1) {
		success = (static_cast<const unsigned char *>(buffer.data())[4] == 0xFF);
	}
	return (success);
}

bool	mariadb_listener::is_com_query(const boost::asio::const_buffer &buffer) {
	bool	success = false;

	if (buffer.size() >= 4 + 1) {
		success = (static_cast<const unsigned char *>(buffer.data())[4] == 0x03); /* 0x03 - COM_QUERY header */
	}
	return (success);
}

void	mariadb_listener::parse_and_log_sql_query(const boost::asio::const_buffer &buffer) {
	size_t	index = 4; /* skip packet header */

	index += 1; /* skip COM_QUERY header */
	if (index < buffer.size()) {
		if (m_logger) {
			m_logger->log(std::string(static_cast<const char *>(buffer.data()) + index, buffer.size() - index));
			m_log_response_from_server = true;
		}
	}
}

std::string	mariadb_listener::build_log_filename() {
	std::ostringstream	stream;

	stream << std::time(0) << "_" << m_connection_id << "_" << m_username;
	return (stream.str());
}

