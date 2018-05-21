#pragma once

#include <string>
#include <functional>
#include <memory>

#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>

namespace dak 
{
	enum class error_codes : int
	{
		EC_OK = 0,

		EC_CENTER_SHUTTED_DOWN = -1,
	};

	// Callback definitions

	// Callback when a operation was over. 
	// Callback will be always be yielded and run on a new callstack.
	// So do not reference to any local variables.

	// error_code: 0: OK; others: any errors.
	typedef std::function<void(error_codes error_code)> on_complete_callback;

	// Callback when receiving a message.
	typedef std::function<void(const std::string& data)> on_message_callback;

	// Interface for both subscription and set_meta command.
	class subscription
	{
	public:
		// Remove the subscription instance, will cancel the subscription.
		virtual ~subscription();

		// Cancel the subscription, but keep the subscription object.
		virtual void cancel() = 0;
	};

	// Auto release pointer of subscription.
	typedef std::auto_ptr<subscription> local_subscription;

	// Interface of a center.
	// A center can be a logic center (local, memory) 
	// or a client center which connects to a remote server(remote, via network)
	// or connections to serveral remote sharding servers.
	class center
	{
	public:
		// Release the center.
		// Will release connection self if it's a client center.
		// Won't shutdown remote servers or such things.
		virtual ~center();

		// Shut down the center gracefully.
		// Any incoming 'send' operation will fail.
		// Won't miss any remote message before shutdown operation.
		virtual void shutdown(on_complete_callback&& on_complete) = 0;

		// Send a message to a topic.
		// It's guaranteed that all subscription to the topic on same center
		// will be called before 'callback' was called.
		// Not guaranteed on different center with same server.
		virtual void send(
			const std::string& topic,
			const std::string& message,
			on_complete_callback&& on_complete = nullptr
		) = 0;

		// Subscribe a topic.
		// Won't receive any message before `callback` was called 
		// or after subscription was canceled.
		// Will return null if and only if the center was shutted down.
		virtual subscription* subscribe(
			const std::string& topic,
			on_message_callback&& on_message,
			on_complete_callback&& on_complete = nullptr
		) = 0;
	};

	// Create a local memory center
	center* create_local_center(
		boost::asio::io_context& io_context
	);

	// Create a client center which connects to a single remote server.
	center* create_client_center(
		boost::asio::io_context& io_context,
		boost::asio::ip::tcp::endpoint endpoint,
		on_complete_callback&& callback = nullptr
	);

	// Create a sharding center which connects to serveral remote sharding servers.
	center* create_sharding_center(
		boost::asio::io_context& io_context,
		std::vector<boost::asio::ip::tcp::endpoint> endpoints,
		on_complete_callback&& callback = nullptr
	);
}
