#pragma once

#include <string>
#include <functional>
#include <memory>

#include <boost/asio.hpp>

namespace dak {

	// Callback definitions:
	typedef std::function<void(int error_code)> on_complete_callback;
	typedef std::function<void(const std::string& data)> on_message_callback;

	class base_subscription {
	public:
		virtual ~base_subscription() {}
		void cancel() {
			delete this;
		}
	};
	typedef base_subscription* subscription;

	typedef std::auto_ptr<base_subscription> local_subscription;

	class center_base {
	public:
		virtual ~center_base() {};
		virtual void send(const std::string& topic, const std::string& message, on_complete_callback callback = nullptr) = 0;
		virtual subscription subscribe(const std::string& topic, on_message_callback on_message, on_complete_callback callback = nullptr) = 0;
	};

	center_base* create_logic_center(boost::asio::io_context& io_context);
	center_base* connect_remote_server(boost::asio::io_context& io_context, boost::asio::ip::tcp::endpoint endpoint, on_complete_callback callback = nullptr);
}
