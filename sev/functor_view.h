/*

Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#ifndef SEV_FUNCTOR_VIEW_H
#define SEV_FUNCTOR_VIEW_H

#include "platform.h"
#include "exception.h"

#ifdef __cplusplus

namespace sev {

template<class TFn>
struct FunctorView;
template<class TRes, class... TArgs>
struct FunctorView<TRes(TArgs...)>
{
private:
	using TVt = FunctorVt<TRes(TArgs...)>;

public:
#pragma warning(push)
#pragma warning(disable: 26495)
#ifdef __INTELLISENSE__
#pragma diag_suppress 2398
#endif
	FunctorView()
	{
		static const auto vtable = TVt();
		m_Vt = vtable;
	}
#pragma warning(pop)

	template<class TFn>
	FunctorView(const TFn &fn)
	{
		static const auto vtable = TVt(fn);
		m_Vt = &vtable;
		m_Ptr = reinterpret_cast<void *>(const_cast<TFn *>(&fn));
		m_Movable = false;
	}

	template<class TFn>
	FunctorView(TFn &fn)
	{
		static const auto vtable = TVt(fn);
		m_Vt = &vtable;
		m_Ptr = reinterpret_cast<void *>(&fn);
		m_Movable = false;
	}

	template<class TFn>
	FunctorView(TFn &&fn)
	{
		static const auto vtable = TVt(fn);
		m_Vt = &vtable;
		m_Ptr = reinterpret_cast<void *>(&fn);
		m_Movable = true;
	}

	// TODO: Specialized constructor from Functor
	// TODO: Specialized toFunctor

	inline TRes operator()(TArgs... value)
	{
		return m_Vt->Invoke(m_Ptr, value...);
	}

	inline bool movable()
	{
		return m_Movable;
	}

	FunctorView(const FunctorView &other)
		: m_Vt(other.m_Vt), m_Ptr(other.m_Ptr)
	{
		// When copying a movable, it's no longer movable
		other.m_Movable = false;
	}

	FunctorView(FunctorView &other)
		: m_Vt(other.m_Vt), m_Ptr(other.m_Ptr)
	{
		// When copying a movable, it's no longer movable
		other.m_Movable = false;
	}

	FunctorView(FunctorView &&other)
		: m_Vt(other.m_Vt), m_Ptr(other.m_Ptr), m_Movable(other.m_Movable)
	{
		// When moving a movable, the other vtable gets reset to the default
		static const auto vtable = TVt();
		other.m_Vt = &vtable;
	}

	FunctorView &operator= (const FunctorView &other)
	{
		if (*this != other)
		{
			~FunctorView();
			new (this) FunctorView(other);
		}
	}

	FunctorView &operator= (FunctorView &&other)
	{
		if (*this != other)
		{
			~FunctorView();
			new (this) FunctorView(move(other));
		}
	}

private:
	// friend struct Functor<TRes(TArgs...)>;
	const TVt *m_Vt;
	void *m_Ptr;
	mutable bool m_Movable;

};

}

#endif

#endif /* #ifndef SEV_FUNCTOR_VIEW_H */

/* end of file */