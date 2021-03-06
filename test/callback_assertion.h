#include <tuple>

template <typename R, typename... Args> class callback_assertion_impl;

template <typename... Args> class callback_assertion_impl<void, Args...> {
public:
	callback_assertion_impl() {}
	typedef callback_assertion_impl<void, Args...> _my;

	typedef std::tuple<typename std::remove_const<
		typename std::remove_reference<Args>::type>::type...>
		_tuple;

	callback_assertion_impl(std::initializer_list<_tuple> list) 
		: pending_(list) 
	{
		std::reverse(pending_.begin(), pending_.end());
	}
	callback_assertion_impl(callback_assertion_impl &&other)
		: pending_(std::move(other.pending_)) 
	{
	}
	~callback_assertion_impl() {
		BOOST_CHECK_MESSAGE(pending_.size() == 0, "Callback assertion failed: call times less than expected.");
	}
	void operator()(Args... args) {
		BOOST_CHECK_MESSAGE(!pending_.empty(), "Callback assertion failed: call times more than expected.");
		bool checkArg =
			std::make_tuple(std::forward<Args>(args)...) == pending_.front();
		BOOST_CHECK_MESSAGE(checkArg == true, "Callback assertion failed: arguments doesn't match");
		pending_.pop_back();
	}

private:
	std::vector<_tuple> pending_;
};

template <typename FuncType> class callback_assertion;

template <typename R, typename... Args>
class callback_assertion<R(Args...)>
	: public callback_assertion_impl<R, Args...> {
public:
	typedef callback_assertion_impl<R, Args...> _base;
	typedef typename _base::_tuple _tuple;
	callback_assertion(std::initializer_list<_tuple> list) : _base(list) {}

	callback_assertion(const callback_assertion &other) {
		BOOST_TEST_FAIL("Callback should not be copied.");
	}

	callback_assertion(callback_assertion &&other)
		: callback_assertion_impl(std::forward<callback_assertion>(other)) {}
};

template <typename R, typename... Args>
class callback_assertion<std::function<R(Args...)>>
	: public callback_assertion_impl<R, Args...> {
public:
	typedef callback_assertion_impl<R, Args...> _base;
	typedef typename _base::_tuple _tuple;

	callback_assertion(std::initializer_list<_tuple> list) : _base(list) {}

	callback_assertion(const callback_assertion &other) {
		BOOST_TEST_FAIL("Callback should not be copied.");
	}

	callback_assertion(callback_assertion &&other)
		: _base(std::forward<callback_assertion>(other)) {}
};
