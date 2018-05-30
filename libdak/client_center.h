#pragma once

#include "dak_impl.h"
#include <stack>
#include <vector>

namespace dak {
	namespace impl {
		class client_center;

		class client_connection
			: std::enable_shared_from_this<client_connection>
		{
		public:
			client_connection(
				client_center* center,
				boost::asio::io_context& io_context,
				boost::asio::ip::tcp::endpoint endpoint
			);

			void send(
				unsigned short seqId,
				const std::string & topic,
				const std::string & message
			);

		private:
			// Start read data & send data after connection ready.
			void on_connected();
			void start_send();

			friend class client_center;

			boost::asio::io_context& io_context_;

			boost::asio::ip::tcp::socket socket_;
			std::string read_buffer_;

			std::string send_buffer_;
			std::string sending_buffer_;

			client_center* center_;
		};

		class client_center
			: public base_center
		{
		public:
			client_center(boost::asio::io_context& io_context, boost::asio::ip::tcp::endpoint endpoint);
			virtual ~client_center();

			virtual void shutdown(on_complete_callback&& on_complete);

			virtual void send(
				const std::string & topic,
				const std::string & message,
				on_complete_callback&& on_complete
			);
			virtual subscription* subscribe(
				const std::string &topic,
				on_message_callback&& on_message,
				on_complete_callback&& callback
			);

		private:
			friend class client_connection;
			void close_with_error(error_codes ec);

			void release_connection();

			// alloc a available seq id.
			// 0 - 65535 as a valid seq id
			// -1 means no seq id available, EC_TOO_MANY_OPERATIONS should be thrown.
			int alloc_seq_id();

			// If connection state is not EC_OK, all following operation will fail with same error code.
			error_codes connection_state_;

			std::shared_ptr<client_connection> connection_;

			// Actually a map, seqid -> callback
			std::vector<on_complete_callback> callbacks_;

			// Seq ids that were free to use.
			std::vector<unsigned short> free_seq_id_;
		};
	}
}
