#include "Promise.hpp"

void onRejectSeq(size_t * index, Promise<void> *promise, const std::exception &reason)
{
	delete index;
	promise->reject(reason);
}

void callNextPromiseInSeq(const std::vector<std::function<Promise<void> *()>> &funcs, size_t *index, Promise<void> *promise)
{
	if ((*index) == funcs.size())
	{
		delete index;
		promise->resolve();
		return;
	}
	Promise<void> *p = funcs[(*index)++]();
	p->freeOnFinish();
	p->onSuccess(std::bind(&callNextPromiseInSeq, funcs, index, promise));
	p->onFail(std::bind(&onRejectSeq, index, promise, std::placeholders::_1));
}

Promise<void> *Promise<void>::sequence(const std::vector<std::function<Promise<void> *()>> &funcs)
{
	assert(!funcs.empty());
	Promise<void> *promise = new Promise<void>();
	size_t *index = new size_t{0};
	callNextPromiseInSeq(funcs, index, promise);
	return promise;
}
Promise<void> *Promise<void>::onSuccess(const promise_finish_cb_t &cb)
{
	if (state == PromiseState::PROMISE_STATE_FULFILLED)
	{
		cb();
		return this;
	}
	resolve_cb = cb;
	return this;
}
Promise<void> *Promise<void>::onFail(const promise_reject_cb_t &cb)
{
	if (state == PromiseState::PROMISE_STATE_REJECTED)
	{
		cb(this->rejectReason);
		return this;
	}
	reject_cb = cb;
	return this;
}
Promise<void> *Promise<void>::onFinish(const promise_finish_cb_t &cb)
{
	if (state != PromiseState::PROMISE_STATE_PENDING)
	{
		cb();
		return this;
	}
	finish_cb = cb;
	return this;
}
void Promise<void>::resolve()
{
	assert(state == PromiseState::PROMISE_STATE_PENDING);
	state = PromiseState::PROMISE_STATE_FULFILLED;
	if (resolve_cb)
	{
		resolve_cb();
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
void Promise<void>::reject(const std::exception &reason)
{
	assert(state == PromiseState::PROMISE_STATE_PENDING);
	state = PromiseState::PROMISE_STATE_REJECTED;
	rejectReason = reason;
	if (reject_cb)
	{
		reject_cb(rejectReason);
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

Promise<void> *Promise<void>::redirectTo(Promise<void> *promise)
{
	this->redirectSuccessTo(promise);
	return this->redirectRejectTo(promise);
}

Promise<void> *Promise<void>::redirectSuccessTo(Promise<void> *promise)
{
	return this->onSuccess(std::bind(&Promise<void>::resolve, promise));
}

void Promise<void>::free()
{
	delete this;
}

void Promise<void>::freeOnFinish()
{
	if (this->state != PromiseState::PROMISE_STATE_PENDING)
	{
		this->free();
		return;
	}
	this->autoDelete = true;
}