#pragma once

#include <dak/dak.h>
#include <map>
#include <set>

#include "protocol.h"

namespace dak
{
	class server_connection;

	// A server of dak can forward messages & subscriptions of a center to every client connection.
	// A simple server is server with a local center.
	// A agent server is server with a client center.
	// A router server is a server with a sharding client center.
	class server
	{
	public:
		server(boost::asio::io_context& ioc, center *center, boost::asio::ip::tcp::endpoint endpoint);
		~server();

	private:
		friend class server_connection;

		void start_accept();

		boost::asio::io_context& ioc_;
		center* center_;
		boost::asio::ip::tcp::acceptor acceptor_;

		std::set<std::shared_ptr<server_connection>> connections_;
		std::shared_ptr<server_connection> accepting_connection_;
	};

	class server_connection : protected std::enable_shared_from_this<server_connection> {
	public:
		server_connection(server* server, boost::asio::io_context& ioc, center *center);
		~server_connection();

		void close();

	private:
		const message_header& header() {
			return *reinterpret_cast<const message_header*>(read_buffer_.c_str());
		};

		char* data_start() {
			return (&read_buffer_[0]) + sizeof(message_header);
		}

		void read_header();
		void read_message();
		void process_message();

		void send_ack(uint16_t ack, error_codes ec);
		void send_message(const std::string& topic, const std::string& message);

		void start_send();

		friend class server;

		server* server_;
		center* center_;
		boost::asio::io_context& ioc_;
		boost::asio::ip::tcp::socket socket_;

		std::string read_buffer_;

		std::string send_buffer_;
		std::string sending_buffer_;

		bool closed_;

		std::map<std::string, local_subscription> subscriptions_;
	};
}
