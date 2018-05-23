#include "local_center.h"

using namespace dak;
using namespace dak::impl;

center* dak::create_local_center(boost::asio::io_context& io_context) {
	return new local_center(io_context);
}

local_center::local_center(boost::asio::io_context& io_context)
	: base_center(io_context), shutted_down_(false)
{
}

local_center::~local_center()
{
}

void local_center::shutdown(on_complete_callback&& on_complete)
{
	shutted_down_ = true;
	topics_.clear();

	if (on_complete)
	{
		ioc_.post([on_complete = std::move(on_complete)]() {
			on_complete(error_codes::EC_OK);
		});
	}
}

void local_center::send(
	const std::string & topic,
	const std::string & message,
	on_complete_callback&& on_complete
)
{
	if (shutted_down_)
	{
		if (on_complete)
		{
			ioc_.post([on_complete = std::move(on_complete)]() {
				on_complete(error_codes::EC_CENTER_SHUTTED_DOWN);
			});
		}
		return;
	}

	auto itor = topics_.find(topic);

	if (itor != topics_.end())
	{
		itor->second.post(ioc_, message);
	}

	if (on_complete)
	{
		ioc_.post([on_complete = std::move(on_complete)]() {
			on_complete(error_codes::EC_OK);
		});
	}
}

subscription* local_center::subscribe(
	const std::string &topic,
	on_message_callback&& on_message,
	on_complete_callback&& on_complete
)
{
	if (shutted_down_)
	{
		if (on_complete)
		{
			ioc_.post([on_complete = std::move(on_complete)]() {
				on_complete(error_codes::EC_CENTER_SHUTTED_DOWN);
			});
		}
		return NULL;
	}

	auto itor = topics_.find(topic);

	if (itor == topics_.end())
	{
		topics_.insert(
			std::make_pair(
				topic,
				subscription_manager(topic, this)
			)
		);
		itor = topics_.find(topic);
	}

	auto ret = itor->second.subscribe(std::move(on_message));

	if (on_complete)
	{
		ioc_.post([on_complete = std::move(on_complete)]() {
			on_complete(error_codes::EC_OK);
		});
	}

	return ret;
}
