#include "protocol.h"

namespace dak {

	// The first byte contains info of total bytes with top '1's before '0'
	// 0XXXXXXXX 0 ~ 127
	// 10XXXXXXX XXXXXXXX 128 ~ 16383
	uint32_t read_encoded_uint32(std::string& read_buffer, uint32_t& offset, uint32_t packet_size, bool& ok)
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

	std::string read_string(std::string& read_buffer, uint32_t& offset, uint32_t packet_size, bool& ok)
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

	void write_encoded_uint32(std::string& send_buffer, uint32_t value)
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
}
