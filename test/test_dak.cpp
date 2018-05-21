#define BOOST_TEST_MODULE dak_test_local
#include <boost/test/unit_test.hpp>
#include <tuple>
#include <queue>
#include <dak/dak.h>
#include "callback_assertion.h"

namespace test = boost::unit_test;

using namespace dak;

// Factory of a local center.
class local_center_factory
{
public:
	boost::asio::io_context ioc;

	center* create_center()
	{
		return create_local_center(ioc);
	}

	void run()
	{
		ioc.run();
	}
};

typedef std::tuple<local_center_factory> factory_types;

BOOST_AUTO_TEST_SUITE(test_local_center,
	*test::label("test_local_center")
)

// Test a normal message with one subscriber.
BOOST_AUTO_TEST_CASE_TEMPLATE(recv_test_positive, factory, factory_types)
{
	factory f;

	std::auto_ptr<center> center(f.create_center());

	local_subscription sub(
		center->subscribe("Topic",
			std::move(callback_assertion<on_message_callback>({
				std::make_tuple("Hello")
			})),
			std::move(callback_assertion<on_complete_callback>({
				std::make_tuple(error_codes::EC_OK)
			}))
		)
	);

	center->send("Topic", "Hello", std::move(callback_assertion<on_complete_callback>({
		std::make_tuple(error_codes::EC_OK)
	})));

	f.run();
}

// Test send and subscribe after center was shutted down.
BOOST_AUTO_TEST_CASE_TEMPLATE(center_closed, factory, factory_types)
{
	factory f;

	std::auto_ptr<center> center(f.create_center());

	center->shutdown(
		std::move(callback_assertion<on_complete_callback>({
			std::make_tuple(error_codes::EC_OK)
		}))
	);

	local_subscription sub(
		center->subscribe(
			"Topic", 
			std::move(callback_assertion<on_message_callback>({
			})),
			std::move(callback_assertion<on_complete_callback>({
				std::make_tuple(error_codes::EC_CENTER_SHUTTED_DOWN)
			}))
		)
	);

	center->send("Topic", "Hello", std::move(callback_assertion<on_complete_callback>({
		std::make_tuple(error_codes::EC_CENTER_SHUTTED_DOWN)
	})));

	f.run();
}

BOOST_AUTO_TEST_SUITE_END()
