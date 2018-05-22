#include <algorithm>

#include "dak_impl.h"

using namespace dak;
using namespace dak::impl;

subscription::~subscription()
{
}

void subscription::cancel()
{
}

center::~center()
{
}

base_subscription::base_subscription(subscription_manager* manager)
	: weak_subscription_(new weak_subscription(this, manager))
{
}

base_subscription::~base_subscription()
{
}

void base_subscription::cancel()
{
	if (weak_subscription_ != NULL) {
		weak_subscription_->release_subscription();
		release();
		weak_subscription_ = NULL;
	}
}

void base_subscription::release()
{
}

weak_subscription::weak_subscription(
	base_subscription* subscription,
	subscription_manager *manager
)
	: subscription_(subscription), manager_(manager)
{
}

weak_subscription::~weak_subscription()
{
	assert(manager_ == NULL);
	assert(subscription_ == NULL);
}

void weak_subscription::release_subscription()
{
	assert(subscription_ != NULL);
	subscription_ = NULL;

	if (manager_ == NULL) {
		// This will occured when center was destroyed
		// or a set_meta subscription was replaced.
		delete this;
	}
}

void weak_subscription::release_manager()
{
	assert(manager_ != NULL);
	manager_ = NULL;

	if (subscription_ == NULL) {
		delete this;
	}
}

weak_subscription_list::weak_subscription_list()
	: valid_count_(0)
{

}

weak_subscription_list::weak_subscription_list(weak_subscription_list&& other)
	: valid_count_(other.valid_count_), subscriptions_(std::move(other.subscriptions_))
{
}

weak_subscription_list::~weak_subscription_list()
{
	for (
		auto itor = subscriptions_.begin();
		itor != subscriptions_.end();
		++itor
		)
	{
		(*itor)->release_manager();
	}
}

void weak_subscription_list::on_subscription_released()
{
	--valid_count_;

	// Half of subscriptions was canceled, do clean up.
	if (valid_count_ * 2 <= subscriptions_.size())
	{
		clean_up();
	}
}

void weak_subscription_list::clean_up()
{
	std::vector<weak_subscription *> copy;

	for (
		auto itor = subscriptions_.begin();
		itor != subscriptions_.end();
		++itor
		)
	{
		if ((*itor)->get_subscription() == NULL)
		{
			(*itor)->release_manager();
		}
		else {
			copy.push_back(*itor);
		}
	}

	copy.swap(subscriptions_);
	assert(valid_count_ == subscriptions_.size());
}

void weak_subscription_list::push_back(weak_subscription *subscription)
{
	subscriptions_.push_back(subscription);
	++valid_count_;
}

message_subscription::message_subscription(subscription_manager* manager, on_message_callback&& on_message)
	: base_subscription(manager), on_message_(std::move(on_message))
{
}

message_subscription::~message_subscription()
{
	cancel();
}

void message_subscription::release()
{
	auto manager = get_weak()->get_manager();
	if (manager != NULL)
	{
		manager->on_message_subscription_released();
	}
}

void message_subscription::invoke(const std::string& message)
{
	on_message_(message);
}

subscription_manager::subscription_manager(const std::string& topic, base_center* center)
	: topic_(topic), center_(center)
{
}

subscription_manager::subscription_manager(subscription_manager&& other)
	: message_subscriptions_(std::move(other.message_subscriptions_))
	, topic_(std::move(other.topic_))
	, center_(other.center_)
{
}

subscription_manager::~subscription_manager()
{
}

subscription* subscription_manager::subscribe(
	on_message_callback&& on_message
)
{
	base_subscription* sub = new message_subscription(this, std::move(on_message));

	message_subscriptions_.push_back(sub->get_weak());

	return sub;
}

void subscription_manager::dispatch(const std::string& message)
{
	message_subscriptions_.clean_up();

	for (
		auto itor = message_subscriptions_.begin();
		itor != message_subscriptions_.end();
		++itor
		)
	{
		message_subscription* sub = static_cast<message_subscription*>((*itor)->get_subscription());

		assert(sub != NULL);
		sub->invoke(message);
	}
}

void subscription_manager::post(boost::asio::io_context& io_context, const std::string& message)
{
	io_context.post([=]() {
		this->dispatch(message);
	});
}

void subscription_manager::on_message_subscription_released()
{
	message_subscriptions_.on_subscription_released();
	if (this->valid_count() == 0) {
		center_->release_topic(topic_);
	}
}

base_center::base_center(boost::asio::io_context& io_context)
	: ioc_(io_context)
{
}

base_center::~base_center()
{
}

void base_center::release_topic(const std::string& topic)
{
	size_t i = topics_.erase(topic);
	assert(i == 1);
}
