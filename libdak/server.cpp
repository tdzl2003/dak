#include <algorithm>

#include "server.h"
#include "protocol.h"

#include <boost/static_assert.hpp>

using namespace dak;

#define MINIMAL_BUFFER_SIZE ((size_t)256)
#define MAX_TOPIC_SIZE ((size_t)2048)
#define MAX_MESSAGE_SIZE 65535
#define MAX_PACKET_SIZE ((size_t)(2 * 2 + MAX_TOPIC_SIZE * 2 + MAX_MESSAGE_SIZE))

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
		// Start read process for accepted connection.
		accepting_connection_->read_header();
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
		this->closed_ = true;

		this->socket_.close();
		this->subscriptions_.clear();
		this->send_buffer_.clear();
	}
}

void server_connection::read_header()
{
	boost::asio::async_read(
		this->socket_,
		boost::asio::buffer(&read_buffer_[0], sizeof(message_header)),
		[self = shared_from_this()](boost::system::error_code ec, std::size_t) {

		if (ec)
		{
			self->close();
			return;
		}
		self->read_message();
	});
}

void server_connection::read_message()
{
	uint32_t packet_size = this->header().packet_size();
	if (packet_size > MAX_PACKET_SIZE)
	{
		this->close();
		return;
	}
	size_t expected_size = std::max(MINIMAL_BUFFER_SIZE, packet_size + sizeof(message_header));
	if (expected_size != read_buffer_.size())
	{
		read_buffer_.resize(expected_size);
	}
	boost::asio::async_read(
		this->socket_,
		boost::asio::buffer(&read_buffer_[sizeof(message_header)], packet_size),
		[self = shared_from_this()](boost::system::error_code ec, std::size_t) {
		if (ec)
		{
			self->close();
			return;
		}
		self->process_message();
	});
}

void server_connection::process_message()
{
	message_type mtype = this->header().mtype();
	uint32_t packet_size = this->header().packet_size();
	uint32_t offset = sizeof(message_header);
	bool ok = true;

	switch (mtype)
	{
	case message_type::ET_MESSAGE:
	{
		uint16_t ack = (uint16_t)read_encoded_uint32(read_buffer_, offset, packet_size, ok);
		std::string topic = read_string(read_buffer_, offset, packet_size, ok);
		if (!ok) break;
		std::string message = read_buffer_.substr(offset);
		center_->send(topic, message, [ack, weak_self = weak_from_this()](error_codes error_code) {
			if (auto self = weak_self.lock()) {
				self->send_ack(ack, error_code);
			}
		});
		return;
	}
	case message_type::ET_SUBSCRIBE:
	{
		uint16_t ack = (uint16_t)read_encoded_uint32(read_buffer_, offset, packet_size, ok);
		if (!ok) break;
		std::string topic = read_buffer_.substr(offset);
		center_->subscribe(topic, [topic = std::move(topic), weak_self = weak_from_this()](const std::string& message) {
			if (auto self = weak_self.lock()) {
				self->send_message(topic, message);
			}
		}, [ack, weak_self = weak_from_this()](error_codes error_code) {
			if (auto self = weak_self.lock()) {
				self->send_ack(ack, error_code);
			}
		});

		return;
	}
	}
	// comes only ok = false
	this->close();
}

// The first byte contains info of total bytes with top '1's before '0'
// 0XXXXXXXX 0 ~ 127
// 10XXXXXXX XXXXXXXX 128 ~ 16383
uint32_t server_connection::read_encoded_uint32(std::string& read_buffer, uint32_t& offset, uint32_t packet_size, bool& ok)
{
	if (!ok) {
		return 0;
	}
	if (offset > packet_size)
	{
		ok = false;
		return 0;
	}
	uint8_t first_byte = read_buffer[offset++];
	uint8_t tail_bytes = 0;

	for (;; tail_bytes++)
	{
		if ((first_byte & (1 << (7 - tail_bytes))) == 0)
		{
			break;
		}
	}

	offset += tail_bytes;
	if (offset > packet_size)
	{
		ok = false;
		return 0;
	}

	uint32_t result = tail_bytes == 8 ? 0 : first_byte & (((uint8_t)0xff) >> tail_bytes);

	for (; tail_bytes > 0; --tail_bytes)
	{
		result <<= 8;
		result |= (uint8_t)read_buffer[offset++];
	}
	return result;
}

std::string server_connection::read_string(std::string& read_buffer, uint32_t& offset, uint32_t packet_size, bool& ok)
{
	if (!ok) {
		return std::string();
	}
	uint32_t size = read_encoded_uint32(read_buffer, offset, packet_size, ok);
	if (!ok) {
		return std::string();
	}
	uint32_t start = offset;
	offset += size;
	if (offset > packet_size)
	{
		ok = false;
		return std::string();
	}
	return read_buffer.substr(start, size);
}

void server_connection::send_ack(uint16_t ack, error_codes ec)
{
	message_header header(ack, ec);

	send_buffer_.append(reinterpret_cast<const char*>(&header), sizeof(header));

	start_send();
}

void server_connection::send_message(const std::string& topic, const std::string& message)
{
	std::string tmp;
	write_encoded_uint32(tmp, (uint32_t)topic.length());

	message_header header(message_type::ET_MESSAGE, (uint32_t)(tmp.length() + topic.length() + message.length()));
	send_buffer_.append(reinterpret_cast<const char*>(&header), sizeof(header));
	send_buffer_.append(tmp);
	send_buffer_.append(topic);
	send_buffer_.append(message);

	start_send();
}

void server_connection::write_encoded_uint32(std::string& send_buffer, uint32_t value)
{
	int tail_bytes = 0;

	while (tail_bytes < 4 && (value & (((uint32_t)0xffffffff) << (tail_bytes * 7))) != 0)
	{
		++tail_bytes;
	}

	uint8_t first_byte = tail_bytes == 4 ? 0 : (value >> (tail_bytes * 8));

	send_buffer.push_back((char)first_byte);

	for (--tail_bytes; tail_bytes >= 0; --tail_bytes)
	{
		send_buffer.push_back((char)(value >> (tail_bytes * 8)));
	}
}

void server_connection::start_send()
{
	if (sending_buffer_.length() > 0) {
		return;
	}
	std::swap(sending_buffer_, send_buffer_);

	boost::asio::async_write(socket_, boost::asio::buffer(sending_buffer_),
		[self = shared_from_this()](boost::system::error_code ec, std::size_t) {
		self->sending_buffer_.clear();
		if (ec)
		{
			self->close();
			return;
		}
		self->start_send();
	});
}
