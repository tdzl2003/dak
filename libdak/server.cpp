#include "server.h"
#include "protocol.h"

using namespace dak;

#define MINIMAL_BUFFER_SIZE ((size_t)256)
#define MAX_PACKAGE_SIZE ((size_t)(1 + 2 * 4 + 65535 * 3))

server::server(boost::asio::io_context& ioc, center *center, boost::asio::ip::tcp::endpoint endpoint)
	: ioc_(ioc), center_(center), acceptor_(ioc, endpoint)
{
	start_accept();
}

server::~server()
{
	acceptor_.close();

	// Make center unavailable for every connection to keep life-cycle safe.
	for (auto itor = connections_.begin(); itor != connections_.end(); ++itor)
	{
		(*itor)->center_ = nullptr;
	}
}

void server::start_accept()
{
	accepting_connection_.reset(new server_connection(this, ioc_, center_));
	acceptor_.async_accept(accepting_connection_->socket_, [=](boost::system::error_code ec) {
		if (ec)
		{
			if (ec != boost::asio::error::operation_aborted)
			{
				fprintf(stderr, "%s", ec.message().c_str());
				abort();
			}
			return;
		}
		accepting_connection_->on_connected();
		connections_.insert(std::move(accepting_connection_));
		start_accept();
	});
}

server_connection::server_connection(server* server, boost::asio::io_context& ioc, center *center)
	: server_(server), center_(center), ioc_(ioc), socket_(ioc), closed_(false)
{
	read_buffer_.resize(MINIMAL_BUFFER_SIZE);
}

server_connection::~server_connection()
{
}

void server_connection::close()
{
	if (!closed_) {
		closed_ = true;

		socket_.close();
		subscriptions_.clear();
		send_buffers_.clear();
	}
}
