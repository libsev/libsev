/*

Copyright (C) 2016-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#pragma once
#ifndef SEV_EVENT_LOOP_H
#define SEV_EVENT_LOOP_H

#include "platform.h"

#include "event_flag.h"
#include "functor_view.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SEV_EventLoopVt;
struct SEV_EventLoop
{
	SEV_EventLoopVt *Vt;

};

struct SEV_EventLoopVt
{
	void(*Destroy)(SEV_EventLoop *el);

	errno_t(*Post)(SEV_EventLoop *el, errno_t(*f)(void *ptr, SEV_EventLoop *el), void *ptr, ptrdiff_t size);
	void(*Invoke)(SEV_EventLoop *el, SEV_ExceptionHandle *eh, errno_t(*f)(void *ptr, SEV_EventLoop *el), void *ptr);
	errno_t(*Timeout)(SEV_EventLoop *el, errno_t(*f)(void *ptr, SEV_EventLoop *el), void *ptr, ptrdiff_t size, int timeoutMs);
	errno_t(*Interval)(SEV_EventLoop *el, errno_t(*f)(void *ptr, SEV_EventLoop *el), void *ptr, ptrdiff_t size, int intervalMs);

	errno_t(*PostFunctor)(SEV_EventLoop *el, const SEV_FunctorVt *vt, void *ptr, void(*forwardConstructor)(void *ptr, void *other));
	void(*InvokeFunctor)(SEV_EventLoop *el, SEV_ExceptionHandle *eh, const SEV_FunctorVt *vt, void *ptr);
	errno_t(*TimeoutFunctor)(SEV_EventLoop *el, const SEV_FunctorVt *vt, void *ptr, void(*forwardConstructor)(void *ptr, void *other), int timeoutMs);
	errno_t(*IntervalFunctor)(SEV_EventLoop *el, const SEV_FunctorVt *vt, void *ptr, void(*forwardConstructor)(void *ptr, void *other), int intervalMs);

	errno_t(*Join)(SEV_EventLoop *el, bool empty);

	errno_t(*Run)(SEV_EventLoop *el, const SEV_FunctorVt *onError, void *ptr, void(*forwardConstructor)(void *ptr, void *other)); // void(SEV_ExceptionHandle *eh)
	void(*Loop)(SEV_EventLoop *el, SEV_ExceptionHandle *eh);
	void(*Stop)(SEV_EventLoop *el);

	ptrdiff_t Reserved[32 - 13];

};

#ifdef __cplusplus
static_assert(sizeof(SEV_EventLoopVt) == 32 * sizeof(void *));
#endif

// Interface
SEV_LIB void SEV_EventLoop_destroy(SEV_EventLoop *el);

SEV_LIB errno_t SEV_EventLoop_post(SEV_EventLoop *el, errno_t(*f)(void *ptr, SEV_EventLoop *el), void *ptr, ptrdiff_t size); // Post functions failure is most likely ENOMEM, no other known errors. ptr is copied to the queue
SEV_LIB void SEV_EventLoop_invoke(SEV_EventLoop *el, SEV_ExceptionHandle *eh, errno_t(*f)(void *ptr, SEV_EventLoop *el), void *ptr); // TODO: Cast down eh
SEV_LIB errno_t SEV_EventLoop_timeout(SEV_EventLoop *el, errno_t(*f)(void *ptr, SEV_EventLoop *el), void *ptr, ptrdiff_t size, int timeoutMs);
SEV_LIB errno_t SEV_EventLoop_interval(SEV_EventLoop *el, errno_t(*f)(void *ptr, SEV_EventLoop *el), void *ptr, ptrdiff_t size, int intervalMs);

SEV_LIB errno_t SEV_EventLoop_postFunctor(SEV_EventLoop *el, const SEV_FunctorVt *vt, void *ptr, void(*forwardConstructor)(void *ptr, void *other));
SEV_LIB void SEV_EventLoop_invokeFunctor(SEV_EventLoop *el, SEV_ExceptionHandle *eh, const SEV_FunctorVt *vt, void *ptr); // TODO: Cast down eh
SEV_LIB errno_t SEV_EventLoop_timeoutFunctor(SEV_EventLoop *el, const SEV_FunctorVt *vt, void *ptr, void(*forwardConstructor)(void *ptr, void *other), int timeoutMs);
SEV_LIB errno_t SEV_EventLoop_intervalFunctor(SEV_EventLoop *el, const SEV_FunctorVt *vt, void *ptr, void(*forwardConstructor)(void *ptr, void *other), int intervalMs);

SEV_LIB errno_t SEV_EventLoop_join(SEV_EventLoop *el, bool empty);

SEV_LIB errno_t SEV_EventLoop_run(SEV_EventLoop *el, const SEV_FunctorVt *onError, void *ptr, void(*forwardConstructor)(void *ptr, void *other));
SEV_LIB void SEV_EventLoop_loop(SEV_EventLoop *el, SEV_ExceptionHandle *eh); // TODO: Cast down eh
SEV_LIB void SEV_EventLoop_stop(SEV_EventLoop *el);

// Generic implementations, work with all event loops
SEV_LIB errno_t SEV_IMPL_EventLoopBase_post(SEV_EventLoop *el, errno_t(*f)(void *ptr, SEV_EventLoop *el), void *ptr, ptrdiff_t size);
SEV_LIB void SEV_IMPL_EventLoopBase_invoke(SEV_EventLoop *el, SEV_ExceptionHandle *eh, errno_t(*f)(void *ptr, SEV_EventLoop *el), void *ptr);
SEV_LIB errno_t SEV_IMPL_EventLoopBase_timeout(SEV_EventLoop *el, errno_t(*f)(void *ptr, SEV_EventLoop *el), void *ptr, ptrdiff_t size, int timeoutMs);
SEV_LIB errno_t SEV_IMPL_EventLoopBase_interval(SEV_EventLoop *el, errno_t(*f)(void *ptr, SEV_EventLoop *el), void *ptr, ptrdiff_t size, int intervalMs);

SEV_LIB SEV_EventLoop *SEV_EventLoop_create();
SEV_LIB void SEV_IMPL_EventLoop_destroy(SEV_EventLoop *el);

SEV_LIB errno_t SEV_IMPL_EventLoop_postFunctor(SEV_EventLoop *el, const SEV_FunctorVt *vt, void *ptr, void(*forwardConstructor)(void *ptr, void *other));
SEV_LIB void SEV_IMPL_EventLoop_invokeFunctor(SEV_EventLoop *el, SEV_ExceptionHandle *eh, const SEV_FunctorVt *vt, void *ptr);
// SEV_LIB errno_t SEV_EventLoop_timeoutFunctor(SEV_EventLoop *el, const SEV_FunctorVt *vt, void *ptr, void(*forwardConstructor)(void *ptr, void *other), int timeoutMs);
// SEV_LIB errno_t SEV_EventLoop_intervalFunctor(SEV_EventLoop *el, const SEV_FunctorVt *vt, void *ptr, void(*forwardConstructor)(void *ptr, void *other), int intervalMs);

SEV_LIB errno_t SEV_IMPL_EventLoop_run(SEV_EventLoop *el, const SEV_FunctorVt *onError, void *ptr, void(*forwardConstructor)(void *ptr, void *other));
SEV_LIB void SEV_IMPL_EventLoop_loop(SEV_EventLoop *el, SEV_ExceptionHandle *eh);
SEV_LIB void SEV_IMPL_EventLoop_stop(SEV_EventLoop *el);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace sev {
typedef SEV_EventLoop EventLoop;
typedef FunctorVt<errno_t(EventLoop &el)> EventFunctorVt;
typedef Functor<errno_t(EventLoop &el)> EventFunctor;
typedef FunctorView<errno_t(EventLoop &el) > EventFunctorView;
}
#endif

/*

TODO: Class SingleLoop, MultiLoop, has virtual loop() = 0. Event loops can inherit from this to compose the thread start or synchronous run utility functions into the class.
Just have std::thread in a pointer, so we don't need to deal with weird stuff.
Do need a DLL-safe shared_ptr implementation?
Actually for our own types based on STL templates, we can enforce a DLL-exported template instance! As long as they're not exposed... So, better not...

*/

#ifdef __cplusplus

namespace sev {



















#if 0

class EventLoop
{
public:
	static SEV_FORCE_INLINE EventLoop *create()
	{
		EventLoop *el = (EventLoop *)SEV_EventLoop_create();
		if (!el)
			throw std::bad_alloc();
		return el;
	}

	static SEV_FORCE_INLINE EventLoop *create(nothrow_t)
	{
		return (EventLoop *)SEV_EventLoop_create();
	}

	SEV_FORCE_INLINE void destroy() // delete
	{
		SEV_EventLoop_destroy(this);
	}
	
	SEV_FORCE_INLINE void post(errno_t(*f)(void *capture, SEV_EventLoop *el), void *capture, ptrdiff_t size)
	{
		errno_t err = SEV_EventLoop_post(this, f, capture, size);
		if (!err) return;
		if (err == ENOMEM) throw std::bad_alloc();
		throw std::exception();
	}

	SEV_FORCE_INLINE void invoke(errno_t(*f)(void *capture, SEV_EventLoop *el), void *capture)
	{
		errno_t err = SEV_EventLoop_invoke(this, f, capture, size);
		if (!err) return;
		if (err == ENOMEM) throw std::bad_alloc();
		throw std::exception();
	}

	SEV_FORCE_INLINE void timeout(errno_t(*f)(void *capture, SEV_EventLoop *el), void *capture, ptrdiff_t size, int timeoutMs)
	{
		errno_t err = SEV_EventLoop_timeout(this, f, capture, size, timeoutMs);
		if (!err) return;
		if (err == ENOMEM) throw std::bad_alloc();
		throw std::exception();
	}

	SEV_FORCE_INLINE void interval(errno_t(*f)(void *capture, SEV_EventLoop *el), void *capture, ptrdiff_t size, int intervalMs)
	{
		errno_t err = SEV_EventLoop_interval(this, f, capture, size, intervalMs);
		if (!err) return;
		if (err == ENOMEM) throw std::bad_alloc();
		throw std::exception();
	}
	
	SEV_FORCE_INLINE void post(EventFunctorView &&f) = 0;
	SEV_FORCE_INLINE void invoke(EventFunctorView &&f) = 0;
	SEV_FORCE_INLINE void timeout(EventFunctorView &&f, int timeoutMs) = 0;
	SEV_FORCE_INLINE void interval(EventFunctorView &&f, int intervalMs)  = 0;

	SEV_FORCE_INLINE void loop() = 0;
	SEV_FORCE_INLINE void run() = 0;

	SEV_FORCE_INLINE IEventLoop(const IEventLoop &other) = delete;
	SEV_FORCE_INLINE IEventLoop &operator=(const IEventLoop &other) = delete;
	
	SEV_FORCE_INLINE IEventLoop(IEventLoop &&other) noexcept = delete;
	SEV_FORCE_INLINE IEventLoop &operator=(IEventLoop &&other) noexcept = delete;

};
#endif

#if 0

class SEV_LIB IEventLoop
{
public:
	//! Run a function on a thread, and proces a callback afterwards
	template<typename T>
	void thread(T &&f, T &&cb)
	{
		std::thread t([this, f = std::forward(f), cb = std::forward(cb)]() mutable -> void {
			f();
			post(std::forward(cb));
		});
		t.detach();
	}

	//! Post a kernel with signature `void(ptrdiff_t)`, return immediately. Calls callback when done. Involves one memory allocation. Option trickle posts each iteration, and the callback, separately to avoid blocking the event loop
	template<typename TFunc, typename TCb>
	void post(const TFunc &f, ptrdiff_t from, ptrdiff_t to, TCb &&cb = []() -> void {}, bool trickle = false)
	{
		struct TData
		{
			Functor<void(ptrdiff_t)> f;
			EventFunctorView cb;
			EventFunctorView ef;
			std::atomic_ptrdiff_t counter;
			ptrdiff_t to;
			int tc;
		};
		std::shared_ptr<TData> d = std::make_shared<TData>();
		int tc = (int)min((ptrdiff_t)threads(), to - from);
		d->f = f;
		d->cb = cb;
		d->counter = from;
		d->to = to;
		d->tc = tc;
		if (!trickle)
		{
			for (int t = 0; t < tc; ++tc)
			{
				post([d]() -> void {
					Functor<void(ptrdiff_t)> &const f = d->f;
					EventFunctorView &const cb = d->cb;
					std::atomic_ptrdiff_t &const counter = d->counter;
					const ptrdiff_t &const to = d->to;
					const int &const tc = d->tc;

					ptrdiff_t k = counter.fetch_add(1);
					while (k < to)
					{
						f(k);
						k = counter.fetch_add(1);
					}

					if (k - to == tc - 1) // Last round exiting
						cb();
				});
			}
		}
		else
		{
			d->ef = [d]() -> void {
				Functor<void(ptrdiff_t)> &const f = d->f;
				EventFunctorView &const cb = d->cb;
				EventFunctorView &const ef = d->ef;
				std::atomic_ptrdiff_t &const counter = d->counter;
				const ptrdiff_t &const to = d->to;
				const int &const tc = d->tc;

				ptrdiff_t k = counter.fetch_add(1);
				if (k < to)
				{
					f(k);
					post(ef); // Next round
				}
				else if (k - to == tc - 1)
				{
					post(cb); // Last round exiting
				}
			};

			for (int t = 0; t < tc; ++tc)
				d->ef();
		}
	}


	//! Post a kernel with signature `void(ptrdiff_t)`, block until done. This is ideal when the calling thread is running in the same eventloop
	template<typename TFunc, typename TCb>
	void invoke(const TFunc &f, ptrdiff_t from, ptrdiff_t to)
	{
		std::atomic_ptrdiff_t counter = from;
		int tc = (int)min((ptrdiff_t)threads(), to - from);
		if (l_EventLoop == this)
		{
			std::vector<EventFlag> flags(tc - 1);
			for (int t = 1; t < tc; ++tc)
			{
				post([&f, &counter, &flag = flags[t - 1]]() -> void {
					ptrdiff_t k = counter.fetch_add(1);
					while (k < to)
					{
						f(k);
						k = counter.fetch_add(1);
					}
					flag.set();
				});
			}
			ptrdiff_t k = counter.fetch_add(1);
			while (k < counter)
			{
				f(k);
				k = counter.fetch_add(1);
			}
			for (ptrdiff_t i = 0; i < (ptrdiff_t)flags.size(); ++i)
				flags[i].wait();
		}
		else
		{
			std::vector<EventFlag> flags(tc);
			for (int t = 1; t < tc; ++tc)
			{
				post([&f, &counter, &flag = flags[t]]() -> void {
					ptrdiff_t k = counter.fetch_add(1);
					while (k < to)
					{
						f(k);
						k = counter.fetch_add(1);
					}
					flag.set();
				});
			}
			for (ptrdiff_t i = 0; i < (ptrdiff_t)flags.size(); ++i)
				flags[i].wait();
		}
	}
};

#endif

#if 0

class SEV_LIB EventLoop
{
public:
	void clear() // semi-thread-safe
	{
#ifdef SEV_EVENT_LOOP_CONCURRENT_QUEUE
		m_ImmediateConcurrent.clear();
		m_TimeoutConcurrent.clear();
#else
		std::unique_lock<AtomicMutex> lock(m_QueueLock);
		std::unique_lock<AtomicMutex> tlock(m_QueueTimeoutLock);
		m_Immediate = std::move(std::queue<EventFunctorView>());
		m_Timeout = std::move(std::priority_queue<timeout_func>());
#endif
	}

	//! Block call until the queued functions  finished processing. Set empty to repeat the wait until the queue is empty
	void join(bool empty = false) // thread-safe
	{
		EventFlag flag;
		Functor<void(EventLoop &el)> syncFunc = [/*this,*/ &flag, &syncFunc, empty](EventLoop &el) -> void {
#ifdef SEV_EVENT_LOOP_CONCURRENT_QUEUE
			if (empty && !el.m_ImmediateConcurrent.empty())
#else
			bool immediateEmpty;
			; {
				std::unique_lock<AtomicMutex> lock(m_QueueLock);
				immediateEmpty = m_Immediate.empty();
			}
			if (empty && !immediateEmpty)
#endif
			{
				el.immediate(syncFunc);
			}
			else
			{
				flag.set();
			}
		};
		immediate(syncFunc);
		flag.wait();
	}

	template<class rep, class period, typename T = EventFunctorView>
	void timeout(T &&f, const std::chrono::duration<rep, period>& delta) // thread-safe
	{
		timeout_func tf;
		tf.f = std::forward<EventFunctorView>(f);
		tf.time = std::chrono::steady_clock::now() + delta;
		tf.interval = std::chrono::nanoseconds::zero();
		; {
#ifdef SEV_EVENT_LOOP_CONCURRENT_QUEUE
			m_TimeoutConcurrent.push(std::move(tf));
#else
			std::unique_lock<AtomicMutex> lock(m_QueueTimeoutLock);
			m_Timeout.push(std::move(tf));
#endif
		}
		poke();
	}

	template<class rep, class period, typename T = EventFunctorView> 
	void interval(T &&f, const std::chrono::duration<rep, period>& interval) // thread-safe
	{
		timeout_func tf;
		tf.f = std::forward<EventFunctorView>(f);
		tf.time = std::chrono::steady_clock::now() + interval;
		tf.interval = interval;
		; {
#ifdef SEV_EVENT_LOOP_CONCURRENT_QUEUE
			m_TimeoutConcurrent.push(std::move(tf));
#else
			std::unique_lock<AtomicMutex> lock(m_QueueTimeoutLock);
			m_Timeout.push(std::move(tf));
#endif
		}
		poke();
	}

private:
	struct timeout_func
	{
		EventFunctor f;
		std::chrono::steady_clock::time_point time;
		std::chrono::steady_clock::duration interval;

		bool operator <(const timeout_func &o) const
		{
			return time > o.time;
		}

	};

}; /* class EventLoop */

#endif

} /* namespace sev */

#endif /* #ifdef __cplusplus */

#endif /* #ifndef SEV_EVENT_LOOP_H */

/* end of file */
