#ifndef PROMISE_HPP
#define PROMISE_HPP

#include <functional>
#include <exception>

template <typename T>
using promise_resolve_cb_t = std::function<void(const T &)>;

typedef std::function<void(const std::exception &reason)> promise_reject_cb_t;
typedef std::function<void()> promise_finish_cb_t;

enum PromiseState : uint8_t
{
	PROMISE_STATE_PENDING,
	PROMISE_STATE_FULFILLED,
	PROMISE_STATE_REJECTED,
};

template <typename T>
class Promise;

template <>
class Promise<void>
{
public:
	template <typename C>
	static Promise<void> *from(Promise<C> *promise, bool autoDelete = true)
	{
		Promise<void> *newPromise = new Promise<void>();
		promise->redirectTo(newPromise);
		if (autoDelete)
		{
			promise->freeOnFinish();
		}
		return newPromise;
	}

	static Promise<void> *sequence(const std::vector<std::function<Promise<void> *()>> &funcs);

	Promise<void> *onSuccess(const promise_finish_cb_t &cb);
	Promise<void> *onFail(const promise_reject_cb_t &cb);
	Promise<void> *onFinish(const promise_finish_cb_t &cb);
	void resolve();
	void reject(const std::exception &reason);
	Promise<void> *redirectTo(Promise<void> *promise);
	Promise<void> *redirectSuccessTo(Promise<void> *promise);
	template <typename C>
	Promise<void> *redirectRejectTo(Promise<C> *promise);
	void free();
	void freeOnFinish();

private:
	PromiseState state = PromiseState::PROMISE_STATE_PENDING;
	std::exception rejectReason;
	promise_finish_cb_t resolve_cb = nullptr;
	promise_reject_cb_t reject_cb = nullptr;
	promise_finish_cb_t finish_cb = nullptr;
	bool autoDelete = false;
};

template <typename T>
class Promise
{
public:
	template <typename C>
	static Promise<void> *from(Promise<C> *promise, bool autoDelete = true)
	{
		Promise<T> *newPromise = new Promise<T>();
		promise->redirectTo(newPromise);
		if (autoDelete) {
			promise->autoDelete();
		}
		return newPromise;
	}


	Promise<T> *onSuccess(const promise_resolve_cb_t<T> &cb)
	{
		if (this->state == PromiseState::PROMISE_STATE_FULFILLED)
		{
			cb(this->success);
			return this;
		}
		this->resolve_cb = cb;
		return this;
	}
	Promise<T> *onFail(const promise_reject_cb_t &cb)
	{
		if (this->state == PromiseState::PROMISE_STATE_REJECTED)
		{
			cb(this->rejectReason);
			return this;
		}
		this->reject_cb = cb;
		return this;
	}
	Promise<T> *onFinish(const promise_finish_cb_t &cb)
	{
		if (this->state != PromiseState::PROMISE_STATE_PENDING)
		{
			cb();
			return this;
		}
		this->finish_cb = cb;
		return this;
	}
	void resolve(const T &value)
	{
		assert(state == PromiseState::PROMISE_STATE_PENDING);
		state = PromiseState::PROMISE_STATE_FULFILLED;
		this->success = value;
		if (resolve_cb)
		{
			resolve_cb(this->success);
		}
		if (finish_cb)
		{
			finish_cb();
		}
		if (autoDelete)
		{
			this->free();
		}
	}
	void reject(const std::exception &reason)
	{
		assert(this->state == PromiseState::PROMISE_STATE_PENDING);
		state = PromiseState::PROMISE_STATE_REJECTED;
		this->rejectReason = reason;
		if (reject_cb)
		{
			reject_cb(this->rejectReason);
		}
		if (finish_cb)
		{
			finish_cb();
		}
		if (autoDelete)
		{
			this->free();
		}
	}


	template <typename C>
	Promise<T> *redirectTo(Promise<C> *promise)
	{
		this->redirectSuccessTo(promise);
		return this->redirectRejectTo(promise);
	}

	Promise<T> *redirectTo(Promise<void> *promise)
	{
		this->redirectSuccessTo(promise);
		return this->redirectRejectTo(promise);
	}

	template <typename C>
	Promise<T> *redirectSuccessTo(Promise<C> *promise)
	{
		return this->onSuccess(std::bind(&Promise<C>::resolve, promise, std::placeholders::_1));
	}

	Promise<T> *redirectSuccessTo(Promise<void> *promise)
	{
		return this->onSuccess(std::bind(&Promise<void>::resolve, promise));
	}

	template <typename C>
	Promise<T> *redirectRejectTo(Promise<C> *promise)
	{
		return this->onFail(std::bind(&Promise<C>::reject, promise, std::placeholders::_1));
	}
	void free() {
		delete this;
	}

	void freeOnFinish()
	{
		if (this->state != PromiseState::PROMISE_STATE_PENDING) {
			this->free();
			return;
		}
		this->autoDelete = true;
	}

private:
	PromiseState state = PromiseState::PROMISE_STATE_PENDING;
	T success;
	std::exception rejectReason;
	promise_resolve_cb_t<T> resolve_cb = nullptr;
	promise_reject_cb_t reject_cb = nullptr;
	promise_finish_cb_t finish_cb = nullptr;
	bool autoDelete = false;
};



#endif