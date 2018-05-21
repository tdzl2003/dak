#pragma once

#include <dak/dak.h>
#include <map>

namespace dak
{
	// A server of dak can forward messages & subscriptions of a center to every client connection.
	// A simple server is server with a local center.
	// A agent server is server with a client center.
	// A router server is a server with a sharding client center.
	class server
	{
	public:
		server(boost::asio::io_service& ios, center *center, boost::asio::ip::tcp::endpoint endpoint);
		~server();

		void shutdown();

	private:
		boost::asio::io_service& ios_;
		center* center_;
		boost::asio::ip::tcp::acceptor acceptor_;

		bool shutted_down_;
	};

	class server_connection : protected std::enable_shared_from_this<server_connection> {
	public:
		server_connection(boost::asio::io_service& ios, center *center);
		~server_connection();

		void close();

	private:
		std::map<std::string, local_subscription> subscriptions_;
	};
}
