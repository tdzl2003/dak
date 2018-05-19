#pragma once

#include <map>
#include <dak/dak.h>

namespace dak {
	namespace impl {

		class weak_subscription;
		class subscription_manager;

		class general_subscription
			: public base_subscription
		{
		public:
			weak_subscription * get_weak() { return weak_subscription_; }
		protected:
			general_subscription(subscription_manager* manager);
			virtual ~general_subscription();

			virtual void cancel();

			virtual void release() = 0;
		private:
			weak_subscription * weak_subscription_;
		};

		class message_subscription
			: public general_subscription
			, private boost::noncopyable
		{
		public:
			message_subscription(subscription_manager* manager,
				on_message_callback&& callback);
			virtual ~message_subscription();

			void invoke(int message_type, const std::string& message);
		private:
			on_message_callback callback_;
		};

		class weak_subscription
			: private boost::noncopyable
		{
		public:
			weak_subscription(base_subscription* subscription,
				subscription_manager *manager)
				: subscription_(subscription), manager_(manager) {}

			virtual void release_subscription();
			virtual void release_manager();

		private:
			base_subscription * subscription_;
			subscription_manager* manager_;
		};

		class logic_center;

		class weak_subscription_list
		{
		public:
			size_t valid_count()
			{
				return valid_count_;
			}
		private:
			std::vector<weak_subscription *> subscriptions_;
			size_t check_index_;
			size_t valid_count_;
		};

		class subscription_manager
			: boost::noncopyable
		{
		public:
			subscription_manager(const std::string &topic, logic_center *center);
			~subscription_manager();

			void subscribe(message_subscription *subscription);
			void dispatch(const std::string &message);

			size_t valid_count()
			{
				return message_subscriptions_.valid_count();
			}

		private:
			friend class weak_subscription;

			weak_subscription_list message_subscriptions_;

			std::string topic_;
			logic_center *center_;
		};

		class logic_center
			: public center_base
		{
		public:
			~logic_center();

			virtual void send(const std::string &topic, const std::string &message);
			virtual subscription subscribe(const std::string &topic,
				on_message_callback callback);
		private:
			friend class subscription_manager;

			std::map<std::string, subscription_manager*> topics_;
		};
	}
}
