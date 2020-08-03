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

#include "event_loop.h"

errno_t SEVIMPL_EventLoopBase_post(SEV_EventLoop *el, errno_t(*f)(void *capture, SEV_EventLoop *el), void *capture, ptrdiff_t size)
{
	// Generic unoptimized wrapper
	try
	{
		std::vector<uint8_t> v(size);
		memcpy(&v[0], capture, size);
		sev::EventFunctorView fv = std::move([f, v](sev::IEventLoop &el) -> void {
			errno_t err = f((void *)&v[0], &el);
			if (!err) return;
			if (err == ENOMEM) throw std::bad_alloc();
			throw std::exception();
		});
		const sev::EventFunctorVt *vt;
		void *ptr;
		bool movable;
		fv.extract(vt, ptr, movable, true);
		SEV_ASSERT(movable);
		return el->Vt->PostFunctor(el, vt->get(), ptr, vt->get()->MoveConstructor);
	}
	catch (std::bad_alloc)
	{
		return ENOMEM;
	}
	catch (...)
	{
		return EOTHER;
	}
	return 0;
}

errno_t SEVIMPL_EventLoopBase_invoke(SEV_EventLoop *el, errno_t(*f)(void *capture, SEV_EventLoop *el), void *capture, ptrdiff_t size)
{
	// Generic unoptimized wrapper
	try
	{
		std::vector<uint8_t> v(size);
		memcpy(&v[0], capture, size);
		sev::EventFunctorView fv = std::move([f, v](sev::IEventLoop &el) -> void {
			errno_t err = f((void *)&v[0], &el);
			if (!err) return;
			if (err == ENOMEM) throw std::bad_alloc();
			throw std::exception();
		});
		const sev::EventFunctorVt *vt;
		void *ptr;
		bool movable;
		fv.extract(vt, ptr, movable, true);
		SEV_ASSERT(movable);
		return el->Vt->InvokeFunctor(el, vt->get(), ptr, vt->get()->MoveConstructor);
	}
	catch (std::bad_alloc)
	{
		return ENOMEM;
	}
	catch (...)
	{
		return EOTHER;
	}
	return 0;
}

errno_t SEVIMPL_EventLoopBase_timeout(SEV_EventLoop *el, errno_t(*f)(void *capture, SEV_EventLoop *el), void *capture, ptrdiff_t size, int timeoutMs)
{
	// Generic unoptimized wrapper
	try
	{
		std::vector<uint8_t> v(size);
		memcpy(&v[0], capture, size);
		sev::EventFunctorView fv = std::move([f, v](sev::IEventLoop &el) -> void {
			errno_t err = f((void *)&v[0], &el);
			if (!err) return;
			if (err == ENOMEM) throw std::bad_alloc();
			throw std::exception();
		});
		const sev::EventFunctorVt *vt;
		void *ptr;
		bool movable;
		fv.extract(vt, ptr, movable, true);
		SEV_ASSERT(movable);
		return el->Vt->TimeoutFunctor(el, vt->get(), ptr, vt->get()->MoveConstructor, timeoutMs);
	}
	catch (std::bad_alloc)
	{
		return ENOMEM;
	}
	catch (...)
	{
		return EOTHER;
	}
	return 0;
}

errno_t SEVIMPL_EventLoopBase_interval(SEV_EventLoop *el, errno_t(*f)(void *capture, SEV_EventLoop *el), void *capture, ptrdiff_t size, int intervalMs)
{
	// Generic unoptimized wrapper
	try
	{
		std::vector<uint8_t> v(size);
		memcpy(&v[0], capture, size);
		sev::EventFunctorView fv = std::move([f, v](sev::IEventLoop &el) -> void {
			errno_t err = f((void *)&v[0], &el);
			if (!err) return;
			if (err == ENOMEM) throw std::bad_alloc();
			throw std::exception();
		});
		const sev::EventFunctorVt *vt;
		void *ptr;
		bool movable;
		fv.extract(vt, ptr, movable, true);
		SEV_ASSERT(movable);
		return el->Vt->IntervalFunctor(el, vt->get(), ptr, vt->get()->MoveConstructor, intervalMs);
	}
	catch (std::bad_alloc)
	{
		return ENOMEM;
	}
	catch (...)
	{
		return EOTHER;
	}
	return 0;
}

void SEV_EventLoop_destroy(SEV_EventLoop *el)
{
	el->Vt->Destroy(el);
}

errno_t SEV_EventLoop_post(SEV_EventLoop *el, errno_t(*f)(void *capture, SEV_EventLoop *el), void *capture, ptrdiff_t size)
{
	return el->Vt->Post(el, f, capture, size);
}

errno_t SEV_EventLoop_invoke(SEV_EventLoop *el, errno_t(*f)(void *capture, SEV_EventLoop *el), void *capture, ptrdiff_t size)
{
	return el->Vt->Invoke(el, f, capture, size);
}

errno_t SEV_EventLoop_timeout(SEV_EventLoop *el, errno_t(*f)(void *capture, SEV_EventLoop *el), void *capture, ptrdiff_t size, int timeoutMs)
{
	return el->Vt->Timeout(el, f, capture, size, timeoutMs);
}

errno_t SEV_EventLoop_interval(SEV_EventLoop *el, errno_t(*f)(void *capture, SEV_EventLoop *el), void *capture, ptrdiff_t size, int intervalMs)
{
	return el->Vt->Interval(el, f, capture, size, intervalMs);
}

errno_t SEV_EventLoop_postFunctor(SEV_EventLoop *el, const SEV_FunctorVt *vt, void *ptr, void(*forwardConstructor)(void *ptr, void *other))
{
	return el->Vt->PostFunctor(el, vt, ptr, forwardConstructor);
}

errno_t SEV_EventLoop_invokeFunctor(SEV_EventLoop *el, const SEV_FunctorVt *vt, void *ptr, void(*forwardConstructor)(void *ptr, void *other))
{
	return el->Vt->InvokeFunctor(el, vt, ptr, forwardConstructor);
}

errno_t SEV_EventLoop_timeoutFunctor(SEV_EventLoop *el, const SEV_FunctorVt *vt, void *ptr, void(*forwardConstructor)(void *ptr, void *other), int timeoutMs)
{
	return el->Vt->TimeoutFunctor(el, vt, ptr, forwardConstructor, timeoutMs);
}

errno_t SEV_EventLoop_intervalFunctor(SEV_EventLoop *el, const SEV_FunctorVt *vt, void *ptr, void(*forwardConstructor)(void *ptr, void *other), int intervalMs)
{
	return el->Vt->IntervalFunctor(el, vt, ptr, forwardConstructor, intervalMs);
}

void SEV_EventLoop_cancel(SEV_EventLoop *el)
{
	el->Vt->Cancel(el);
}

errno_t SEV_EventLoop_join(SEV_EventLoop *el, bool empty)
{
	return el->Vt->Join(el, empty);
}

void SEV_EventLoop_run(SEV_EventLoop *el, const SEV_FunctorVt *onError, void *ptr, void(*forwardConstructor)(void *ptr, void *other))
{
	el->Vt->Run(el, onError, ptr, forwardConstructor);
}

errno_t SEV_EventLoop_loop(SEV_EventLoop *el)
{
	return el->Vt->Loop(el);
}

void SEV_EventLoop_stop(SEV_EventLoop *el)
{
	el->Vt->Stop(el);
}




















namespace sev {

#if 0

namespace {

thread_local IEventLoop *l_EventLoop;

}

IEventLoop::~IEventLoop() noexcept
{

}

void IEventLoop::setCurrent(bool current)
{
	l_EventLoop = current ? this : null;
}

bool IEventLoop::current()
{
	return l_EventLoop == this;
}

EventLoop::EventLoop() : m_Running(false), m_Cancel(false)
{
	
}

EventLoop::~EventLoop() noexcept
{
	stop();
	clear();
}

void EventLoop::loop()
{
	while (m_Running)
	{
		for (;;)
		{
#ifdef SEV_EVENT_LOOP_CONCURRENT_QUEUE
			EventFunctor f;
			if (!m_ImmediateConcurrent.try_pop(f))
				break;
#else
			m_QueueLock.lock();
			if (!m_Immediate.size())
			{
				m_QueueLock.unlock();
				break;
			}
			EventFunctor f = m_Immediate.front();
			m_Immediate.pop();
			m_QueueLock.unlock();
#endif
			f(*this);
		}

		bool poked = false;
		for (;;)
		{
#ifdef SEV_EVENT_LOOP_CONCURRENT_QUEUE
			timeout_func tf;
			if (!m_TimeoutConcurrent.try_pop(tf))
				break;
			const timeout_func &tfr = tf;
#else
			m_QueueTimeoutLock.lock();
			if (!m_Timeout.size())
			{
				m_QueueTimeoutLock.unlock();
				break;
			}
			const timeout_func &tfr = m_Timeout.top();
#endif
			std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
			int64_t wt = std::chrono::duration_cast<std::chrono::milliseconds>(tfr.time - now).count();
			if (tfr.time > now) // Wait
			{
#ifdef SEV_EVENT_LOOP_CONCURRENT_QUEUE
				m_TimeoutConcurrent.push(tf);
#else
				m_QueueTimeoutLock.unlock();
#endif
				m_Flag.wait(wt & 0xFFFF); // Mask to 65 seconds, it's fine to break out earlier, the loop re-checks
				poked = true;
				break;
			}
#ifndef SEV_EVENT_LOOP_CONCURRENT_QUEUE
			timeout_func tf = tfr;
			m_Timeout.pop();
			m_QueueTimeoutLock.unlock();
#endif
			m_Cancel = false;
			tf.f(*this); // call
			if (!m_Cancel && (tf.interval > std::chrono::nanoseconds::zero())) // repeat
			{
				tf.time += tf.interval;
				; {
#ifdef SEV_EVENT_LOOP_CONCURRENT_QUEUE
					m_TimeoutConcurrent.push(std::move(tf));
#else
					std::unique_lock<AtomicMutex> lock(m_QueueTimeoutLock);
					m_Timeout.push(std::move(tf));
#endif
				}
			}
		}

		if (!poked)
		{
			m_Flag.wait();
		}
	}
}

} /* namespace sev */

namespace sev {

namespace /* anonymous */ {

void test()
{
	
}

} /* anonymous namespace */

#endif

} /* namespace sev */

/* end of file */
