#pragma once

#include <dak/dak.h>

namespace dak {
	namespace impl {
		class weak_subscription;
		class subscription_manager;

		// Base subscription class for any subscription.
		// It will create and reference a weak subscription, which is also kept by center.
		class base_subscription
			: public subscription
		{
		public:
			weak_subscription* get_weak()
			{
				return weak_subscription_;
			}

			// Cancel this subscription and notice referenced weak subscription.
			virtual void cancel();

		protected:
			base_subscription(subscription_manager* manager);
			virtual ~base_subscription() = 0;

			// Override me:
			// The extended class can override this method for extra operation.
			virtual void release();

		private:
			weak_subscription * weak_subscription_;
		};

		// Weak subscription which connects subscription and manager.
		// Will be deleted after subscription was canceled and manager knows that.
		class weak_subscription
			: private boost::noncopyable
		{
		public:
			weak_subscription(
				base_subscription* subscription,
				subscription_manager* manager
			);
			~weak_subscription();

			base_subscription* get_subscription()
			{
				return subscription_;
			}

			subscription_manager* get_manager()
			{
				return manager_;
			}

		private:
			friend class base_subscription;
			friend class weak_subscription_list;

			// Notice when subscription was canceled.
			virtual void release_subscription();

			// Notice when manager was released or it knows that subscription was canceled.
			virtual void release_manager();

			base_subscription* subscription_;
			subscription_manager* manager_;
		};

		// Manage a list of subscription.
		class weak_subscription_list
			: private boost::noncopyable
		{
		public:
			weak_subscription_list();
			weak_subscription_list(weak_subscription_list&& other);
			~weak_subscription_list();

			size_t valid_count()
			{
				return valid_count_;
			}

			// Noticed when a subscription was canceled.
			// Will release the related weak_subscription later.
			// Either after half of subscriptions was canceled,
			// or a message was dispatched.
			void on_subscription_released();

			// Clean up
			void clean_up();

			void push_back(weak_subscription *subscription);

			auto begin() {
				return subscriptions_.begin();
			}
			auto end() {
				return subscriptions_.end();
			}
		private:
			std::vector<weak_subscription *> subscriptions_;
			size_t valid_count_;
		};

		class message_subscription
			: public base_subscription, private boost::noncopyable
		{
		public:
			message_subscription(subscription_manager* manager, on_message_callback&& on_message);
			virtual ~message_subscription();

			virtual void release();

			virtual void invoke(const std::string& message);
		private:
			on_message_callback on_message_;
		};

		class subscription_manager
			: private boost::noncopyable
		{
		public:
			subscription_manager();
			subscription_manager(subscription_manager&& other);
			~subscription_manager();

			subscription* subscribe(
				on_message_callback&& on_message
			);

			// Call every on_message
			void dispatch(const std::string& message);

			// Call every on_message on next tick.
			void post(boost::asio::io_context& io_context, const std::string& message);

			size_t valid_count()
			{
				return message_subscriptions_.valid_count();
			}

		private:
			void on_message_subscription_released()
			{
				message_subscriptions_.on_subscription_released();
			}

			friend class message_subscription;

			weak_subscription_list message_subscriptions_;
		};
	}
}

