#include "client_center.h"
#include "protocol.h"

using namespace dak;
using namespace dak::impl;

client_connection::client_connection(
	client_center* center,
	boost::asio::io_context& io_context,
	boost::asio::ip::tcp::endpoint endpoint
)
	: io_context_(io_context)
	, socket_(io_context)
{
	// Hack: pending all sent messages until connected.
	sending_buffer_.resize(1);

	socket_.async_connect(endpoint, [weak_self = weak_from_this()](boost::system::error_code ec) {
		auto self = weak_self.lock();
		if (!self || !self->center_) {
			// Center already released, ignore.
			return;
		}
		if (ec) {
			// Connect failed, raise a error and shutdown immediately.
			self->center_->close_with_error(error_codes::EC_CONNECTION_FAILED);
			return;
		}

		// Connection ready, start read data & send data.
		self->on_connected();
	});
}

void client_connection::send(
	unsigned short seqId,
	const std::string & topic,
	const std::string & message
)
{
	std::string tmp;
	write_encoded_uint32(tmp, (uint32_t)topic.length());

	message_header header(message_type::MT_MESSAGE, (uint32_t)(tmp.length() + topic.length() + message.length()));
	send_buffer_.append(reinterpret_cast<const char*>(&header), sizeof(header));
	send_buffer_.append(tmp);
	send_buffer_.append(topic);
	send_buffer_.append(message);

	start_send();
}

client_center::client_center(boost::asio::io_context& io_context, boost::asio::ip::tcp::endpoint endpoint)
	: base_center(io_context)
	, connection_(new client_connection(this, io_context, endpoint))
{
}

client_center::~client_center()
{
	release_connection();
}

int client_center::alloc_seq_id()
{
	if (free_seq_id_.size() > 0)
	{
		auto ret = free_seq_id_.back();
		free_seq_id_.pop_back();
		return ret;
	}

	if (callbacks_.size() < 65536)
	{
		unsigned short ret = (unsigned short)callbacks_.size();
		callbacks_.push_back(nullptr);
		return ret;
	}

	return -1;
}

void client_center::send(
	const std::string & topic,
	const std::string & message,
	on_complete_callback&& on_complete
)
{
	if (connection_state_ != error_codes::EC_OK) {
		ioc_.post([error_code = connection_state_, on_complete = std::move(on_complete)]() {
			on_complete(error_code);
		});
		return;
	}
	int seq_id = alloc_seq_id();
	if (seq_id < 0)
	{
		ioc_.post([on_complete = std::move(on_complete)]() {
			on_complete(error_codes::EC_TOO_MANY_OPERATIONS);
		});
		return;
	}
	callbacks_[seq_id].swap(on_complete);
	connection_->send((unsigned short)seq_id, topic, message);
}

void client_center::release_connection()
{
	if (connection_)
	{
		connection_->center_ = nullptr;
		connection_.reset();
	}
}

void client_center::close_with_error(error_codes ec)
{
	connection_state_ = ec;

	std::vector<on_complete_callback> tmp;
	tmp.swap(callbacks_);
	for (auto itor = tmp.begin(); itor != tmp.end(); ++itor)
	{
		if (*itor)
		{
			(*itor)(ec);
		}
	}

	free_seq_id_.clear();
}