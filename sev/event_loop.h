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

#ifndef SEV_EVENT_LOOP_H
#define SEV_EVENT_LOOP_H

#include "platform.h"
#ifdef __cplusplus

#ifdef _MSC_VER
#define SEV_EVENT_LOOP_CONCURRENT_QUEUE
#endif

#include <thread>
#include <mutex>
#include <condition_variable>
#include <variant>

#include <functional>

#include <queue>
#include <set>

#ifdef SEV_EVENT_LOOP_CONCURRENT_QUEUE
#	include <concurrent_queue.h>
#	include <concurrent_priority_queue.h>
#else
#	include "atomic_mutex.h"
#endif

#include "event_flag.h"

/*

TODO: IEventLoop
TODO: Try ring buffer?
TODO: Remove stl stuff from class definition...

Can we make an virtual anonymizer thing like std::function, but for stl classes so they can be popped through dll interfaces?

*/

namespace sev {

/*
struct EventLoopOptions
{
	bool EnableFibers = true;
	bool EnableTimers = true;
	bool EnableIO = true;
}
*/

typedef std::function<void()> EventFunction; // TODO: Can't use STL here due to DLL boundary :/ Need to make our own container
// typedef std::function<void(ptrdiff_t)> EventKernel;

class SEV_LIB IEventLoop;

class SEV_LIB IEventLoop
{
public:
	virtual ~IEventLoop() noexcept;

	//! Number of threads processing this event loop
	virtual int threads() = 0;

	//! Call from inside an interval function to prevent it from being called again
	virtual void cancel() = 0;

	//! Block call until the currently queued functions finished processing. Set empty flag to keep waiting until the queue is completely empty
	virtual void join(bool empty = false) = 0; // thread-safe

	//! Post a function, return immediately
	virtual void post(const EventFunction &f) = 0;
	virtual void post(EventFunction &&f) = 0;

	/*
	virtual void post(const EventKernel &f, ptrdiff_t from, ptrdiff_t to) = 0;
	virtual void post(EventKernel &&f, ptrdiff_t from, ptrdiff_t to) = 0;

	virtual void post(const EventKernel &f, ptrdiff_t from, ptrdiff_t to, const EventFunction &cb) = 0;
	virtual void post(EventKernel &&f, ptrdiff_t from, ptrdiff_t to, EventFunction &&cb) = 0;

	virtual void post(const EventKernel &f, ptrdiff_t from, ptrdiff_t to, EventFunction &&cb) = 0;
	virtual void post(EventKernel &&f, ptrdiff_t from, ptrdiff_t to, const EventFunction &cb) = 0;
	*/

	//! Post a function, block until processed
	virtual void invoke(const EventFunction &f) = 0;
	virtual void invoke(EventFunction &&f) = 0;

	/*
	virtual void invoke(const EventKernel &f, ptrdiff_t from, ptrdiff_t to) = 0;
	virtual void invoke(EventKernel &&f, ptrdiff_t from, ptrdiff_t to) = 0;
	*/

	//! Post a function that will be called after a timeout (TODO: Return a handle to cancel?)
	virtual void timeout(const EventFunction &f, int ms) = 0;
	virtual void timeout(EventFunction &&f, int ms) = 0;

	//! Post a function that will be called at an interval (TODO: Return a handle to cancel?)
	virtual void interval(const EventFunction &f, int ms) = 0;
	virtual void interval(EventFunction &&f, int ms) = 0;

	void setCurrent(bool current = true);
	bool current();

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
			std::function<void(ptrdiff_t)> f;
			EventFunction cb;
			EventFunction ef;
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
					std::function<void(ptrdiff_t)> &const f = d->f;
					EventFunction &const cb = d->cb;
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
				std::function<void(ptrdiff_t)> &const f = d->f;
				EventFunction &const cb = d->cb;
				EventFunction &const ef = d->ef;
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

class SEV_LIB EventLoop
{
public:
	EventLoop();
	virtual ~EventLoop() noexcept;

	void run()
	{
		stop();
		m_Running = true;
		m_Thread = std::move(std::thread(&EventLoop::loop, this));
	}

	void runSync()
	{
		stop();
		m_Running = true;
		loop();
	}

	void stop() // thread-safe
	{
		m_Running = false;
		poke();
		if (m_Thread.joinable())
			m_Thread.join();
	}

	void clear() // semi-thread-safe
	{
#ifdef SEV_EVENT_LOOP_CONCURRENT_QUEUE
		m_ImmediateConcurrent.clear();
		m_TimeoutConcurrent.clear();
#else
		std::unique_lock<AtomicMutex> lock(m_QueueLock);
		std::unique_lock<AtomicMutex> tlock(m_QueueTimeoutLock);
		m_Immediate = std::move(std::queue<EventFunction>());
		m_Timeout = std::move(std::priority_queue<timeout_func>());
#endif
	}

	//! Call from inside an interval function to prevent it from being called again
	void cancel()
	{
		m_Cancel = true;
	}

	//! Block call until the queued functions  finished processing. Set empty to repeat the wait until the queue is empty
	void join(bool empty = false) // thread-safe
	{
		EventFlag flag;
		EventFunction syncFunc = [this, &flag, &syncFunc, empty]() -> void {
#ifdef SEV_EVENT_LOOP_CONCURRENT_QUEUE
			if (empty && !m_ImmediateConcurrent.empty())
#else
			bool immediateEmpty;
			; {
				std::unique_lock<AtomicMutex> lock(m_QueueLock);
				immediateEmpty = m_Immediate.empty();
			}
			if (empty && !immediateEmpty)
#endif
			{
				immediate(syncFunc);
			}
			else
			{
				flag.set();
			}
		};
		immediate(syncFunc);
		flag.wait();
	}

	/*
private:
	template<typename T>
	inline void immediate_(T &&f) 
	{
#ifdef SEV_EVENT_LOOP_CONCURRENT_QUEUE
		m_ImmediateConcurrent.push(std::forward<T>(f));
#else
		std::unique_lock<AtomicMutex> lock(m_QueueLock);
		m_Immediate.push(std::forward<T>(f));
#endif
		poke();
	}

public:
	inline void immediate(const EventFunction &f) // thread-safe
	{
		immediate_(f);
	}

	void immediate(EventFunction &&f) // thread-safe
	{
		immediate_(f);
	}
	*/

	template<typename T = EventFunction>
	inline void immediate(T &&f)
	{
#ifdef SEV_EVENT_LOOP_CONCURRENT_QUEUE
		m_ImmediateConcurrent.push(std::forward<EventFunction>(f));
#else
		std::unique_lock<AtomicMutex> lock(m_QueueLock);
		m_Immediate.push(std::forward<EventFunction>(f));
#endif
		poke();
	}

	template<class rep, class period, typename T = EventFunction>
	void timeout(T &&f, const std::chrono::duration<rep, period>& delta) // thread-safe
	{
		timeout_func tf;
		tf.f = std::forward<EventFunction>(f);
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

	template<class rep, class period, typename T = EventFunction> 
	void interval(T &&f, const std::chrono::duration<rep, period>& interval) // thread-safe
	{
		timeout_func tf;
		tf.f = std::forward<EventFunction>(f);
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

	template<typename T = EventFunction>
	void timed(T &&f, const std::chrono::steady_clock::time_point &point) // thread-safe
	{
		timeout_func tf;
		tf.f = std::forward<EventFunction>(f);
		tf.time = point;
		tf.interval = std::chrono::steady_clock::duration::zero();
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

public:
	template<typename T = EventFunction>
	void thread(T &&f, T &&callback)
	{
		std::thread t([this, f = std::forward(f), callback = std::forward(f)]() mutable -> void {
			f();
			immediate(std::move(callback));
		});
		t.detach();
	}

private:
	void loop();
	inline void poke() { m_Flag.set(); }

private:
	struct timeout_func
	{
		EventFunction f;
		std::chrono::steady_clock::time_point time;
		std::chrono::steady_clock::duration interval;

		bool operator <(const timeout_func &o) const
		{
			return time > o.time;
		}

	};

private:
	bool m_Running;
	std::thread m_Thread;
	EventFlag m_Flag;

#ifdef SEV_EVENT_LOOP_CONCURRENT_QUEUE
	concurrency::concurrent_queue<EventFunction> m_ImmediateConcurrent;
	concurrency::concurrent_priority_queue<timeout_func> m_TimeoutConcurrent;
#else
	AtomicMutex m_QueueLock;
	std::queue<EventFunction> m_Immediate;
	AtomicMutex m_QueueTimeoutLock;
	std::priority_queue<timeout_func> m_Timeout;
#endif
	bool m_Cancel;

}; /* class EventLoop */

} /* namespace sev */

#endif /* #ifdef __cplusplus */

#endif /* #ifndef SEV_EVENT_LOOP_H */

/* end of file */
