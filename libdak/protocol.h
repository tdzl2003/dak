#pragma once

#include <dak/dak.h>

namespace dak
{
	static const size_t MINIMAL_BUFFER_SIZE = 256;
	static const size_t MAX_TOPIC_SIZE = 2048;
	static const size_t MAX_MESSAGE_SIZE = 65535;
	static const size_t MAX_PACKET_SIZE = 2 * 2 + MAX_TOPIC_SIZE * 2 + MAX_MESSAGE_SIZE;

	enum class message_type : uint8_t
	{
		// UP: Send a message to a topic.
		// seqId(int) topic(string) message(string)
		// DOWN: Send a message for a subscribed topic.
		// topic(string) message(string)
		MT_MESSAGE = 0,

		// UP: Subscribe a topic.
		// seqId(int) topic(string)
		MT_SUBSCRIBE = 2,

		// DOWN: ACK for any request.
		// Header will became mtype(1 byte) error_no(1 byte) seqId(2 bytes)
		// And there will be no body.
		MT_ACK = 255,
	};

	struct message_header {
		uint32_t data;

		message_header(message_type type, uint32_t size)
			: data(
			(uint32_t)(message_type::MT_ACK) |
				(size << 8)
			)
		{
		}

		message_header(uint16_t ack, error_codes ec)
			: data(
			(uint32_t)(message_type::MT_ACK) |
				((uint32_t)(ec) << 8) |
				(ack << 16)
			)
		{
		}

		message_type mtype() const {
			return static_cast<message_type>(data & 0xff);
		}

		uint32_t packet_size() const {
			assert(mtype() != message_type::MT_ACK);
			return data >> 8;
		}

		error_codes ec() const {
			assert(mtype() == message_type::MT_ACK);
			return static_cast<error_codes>((data >> 8) & 0xff);
		}

		uint16_t seq_id() const {
			assert(mtype() == message_type::MT_ACK);
			return static_cast<uint16_t>(data >> 16);
		}
	};

	uint32_t read_encoded_uint32(std::string& read_buffer, uint32_t& offset, uint32_t packet_size, bool& ok);
	std::string read_string(std::string& read_buffer, uint32_t& offset, uint32_t packet_size, bool& ok);

	void write_encoded_uint32(std::string& send_buffer, uint32_t value);
}
