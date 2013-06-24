#ifndef __ASYNC_SERVICE_ASYNC_RESULT_HPP
#define __ASYNC_SERVICE_ASYNC_RESULT_HPP



#include <functional>
#include <type_traits>
#include <system_error>
#include <memory>
#include <cstdint>
#include <cassert>

#include "../basic.hpp"
#include "object_factory.hpp"


namespace async { namespace service {


	extern std::_Ph<1> _Error;
	extern std::_Ph<2> _Size;



	// Allocate IO Callback

	struct async_callback_base_t;

	struct async_result_deallocate_t 
	{
		void operator()(async_callback_base_t *p);
	};

	typedef std::unique_ptr< async_callback_base_t, async_result_deallocate_t > async_callback_base_ptr;


	template < typename HandlerT >
	void *allocate_handler(std::uint32_t sz, HandlerT &handler);

	template < typename HandlerT >
	void deallocate_handler(void *p, std::uint32_t sz, HandlerT &handler);

	inline void *allocate_handler(std::uint32_t sz, ...)
	{
		return ::operator new(sz);
	}

	inline void deallocate_handler(void *p, std::uint32_t /*sz*/, ...)
	{
		::operator delete(p);
	}


	//---------------------------------------------------------------------------
	// struct async_callback_base

	struct async_callback_base_t
		: public OVERLAPPED
	{
		async_callback_base_t()
		{
			std::memset(static_cast<OVERLAPPED *>(this), 0, sizeof(OVERLAPPED));
		}

		virtual ~async_callback_base_t() {}
		virtual void invoke(const std::error_code &error, std::uint32_t size) = 0;
		virtual void deallocate() = 0;

	private:
		async_callback_base_t(const async_callback_base_t &);
		async_callback_base_t &operator=(const async_callback_base_t &);
	};

	template < typename OverlappedT >
	void call(OverlappedT *overlapped, std::uint32_t size, const std::error_code &error)
	{
		async_callback_base_ptr p(static_cast<async_callback_base_t *>(overlapped));

		p->invoke(error, size);
	}


	template < typename HandlerT >
	struct win_async_callback_t
		: async_callback_base_t
	{
		HandlerT handler_;

		explicit win_async_callback_t(HandlerT &&callback)
			: handler_(std::move(callback))
		{}

		virtual ~win_async_callback_t()
		{}

		virtual void invoke(const std::error_code &error, std::uint32_t size)
		{
			handler_(std::cref(error), size);
		}

		virtual void deallocate()
		{
			deallocate_handler(this, sizeof(*this), *this);
		}

		friend void *allocate_handler(std::uint32_t sz, win_async_callback_t<HandlerT> *this_handler)
		{
			allocate_handler(sz, this_handler->handler_);
		}

		friend void deallocate_handler(void *p, std::uint32_t sz, win_async_callback_t<HandlerT> *this_handler)
		{
			deallocate_handler(p, sz, this_handler->handler_);
		}
	};


	inline void async_result_deallocate_t::operator()(async_callback_base_t *p)
	{
		p->deallocate();
	}


	template < typename HandlerT >
	void *allocate_handler(std::uint32_t sz, HandlerT &handler)
	{
		return allocate_handler(sz, std::addressof(handler));
	}

	template < typename HandlerT >
	void deallocate_handler(void *p, std::uint32_t sz, HandlerT &handler)
	{
		deallocate_handler(p, sz, std::addressof(handler));
	}

	template < typename HandlerT >
	win_async_callback_t<HandlerT> *make_async_callback(HandlerT &&handler)
	{
		typedef win_async_callback_t<HandlerT> async_callback_t;

		void *p = allocate_handler(sizeof(async_callback_t), handler);
		return ::new(p) async_callback_t(std::forward<HandlerT>(handler));
	}
}
}


#endif