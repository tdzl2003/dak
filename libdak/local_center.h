#pragma once

#include <map>
#include "dak_impl.h"

namespace dak {
	namespace impl {
		class local_center
			: public base_center
		{
		public:
			local_center(boost::asio::io_context& io_context);
			virtual ~local_center();

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
			bool shutted_down_;
		};
	}
}
