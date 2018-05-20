#define BOOST_TEST_MODULE dak_test_local
#include <boost/test/unit_test.hpp>
#include <tuple>
#include <queue>
#include <dak/dak.h>

namespace test = boost::unit_test;

using namespace dak;

template <typename R, typename... Args>
class callback_assertion_impl;

template <typename... Args>
class callback_assertion_impl<void, Args...>
{
public:
	callback_assertion_impl() {}
	typedef callback_assertion_impl<void, Args...> _my;

	typedef std::tuple<
		typename std::remove_const<
		typename std::remove_reference<Args>::type
		>::type...
	> _tuple;

	callback_assertion_impl(std::initializer_list<_tuple> list)
		: pending_(list)
	{
		std::reverse(pending_.begin(), pending_.end());
	}
	callback_assertion_impl(callback_assertion_impl&& other)
		: pending_(std::move(other.pending_))
	{
	}
	~callback_assertion_impl() {
		BOOST_TEST(pending_.size() == 0);
	}
	void operator() (Args... args) {
		BOOST_TEST(!pending_.empty());
		bool checkArg = std::make_tuple(std::forward<Args>(args)...) == pending_.front();
		BOOST_CHECK_MESSAGE(checkArg == true, "Callback assertion failed.");
		pending_.pop_back();
	}
private:
	std::vector<_tuple> pending_;
};

template <typename FuncType>
class callback_assertion;

template <typename R, typename... Args>
class callback_assertion<R(Args...)>
	: public callback_assertion_impl<R, Args...>
{
public:
	typedef typename callback_assertion_impl<R, Args...>::_tuple _tuple;
	callback_assertion(std::initializer_list<_tuple> list)
		: callback_assertion_impl(list)
	{
	}

	callback_assertion(callback_assertion&& other)
		: callback_assertion_impl(std::forward<callback_assertion>(other))
	{
	}
};

template <typename R, typename... Args>
class callback_assertion<std::function<R(Args...)>>
	: public callback_assertion_impl<R, Args...>
{
public:
	typedef typename callback_assertion_impl<R, Args...>::_tuple _tuple;

	callback_assertion(std::initializer_list<_tuple> list)
		: callback_assertion_impl(list)
	{
	}
	callback_assertion(const callback_assertion& other)
	{
		BOOST_TEST_FAIL("Callback should not be copied.");
	}
	callback_assertion(callback_assertion&& other)
		: callback_assertion_impl(std::forward<callback_assertion>(other))
	{
	}
};


BOOST_AUTO_TEST_SUITE(test_local_center,
	*test::label("test_local_center")
)

BOOST_AUTO_TEST_CASE(recv_test_positive, *test::description("Should receive callback which sent to same topic"))
{
	boost::asio::io_context ioc;

	std::auto_ptr<center> center(create_local_center(ioc));

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

	ioc.run();
}

BOOST_AUTO_TEST_SUITE_END()
