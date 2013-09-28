#ifndef __ASYNC_SERVICE_ASYNC_RESULT_HPP
#define __ASYNC_SERVICE_ASYNC_RESULT_HPP



#include <functional>
#include <type_traits>
#include <system_error>
#include <memory>
#include <cstdint>
#include <cassert>

#include "../basic.hpp"

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


	template < typename HandlerT, typename AllocatorT >
	struct win_async_callback_t
		: async_callback_base_t
	{
		typedef win_async_callback_t<HandlerT, AllocatorT> this_t;
		HandlerT handler_;
		AllocatorT allocator_;

		explicit win_async_callback_t(HandlerT &&callback, const AllocatorT &allocator)
			: handler_(std::move(callback))
			, allocator_(allocator)
		{}

		virtual ~win_async_callback_t()
		{}

		virtual void invoke(const std::error_code &error, std::uint32_t size)
		{
			handler_(error, size);
		}

		virtual void deallocate()
		{
			typedef typename AllocatorT::rebind<this_t>::other allocator_t;
			allocator_t alloc = allocator_;

			alloc.destroy(this);
			alloc.deallocate(this, 1);
		}
	};


	inline void async_result_deallocate_t::operator()(async_callback_base_t *p)
	{
		p->deallocate();
	}

	template < typename HandlerT, typename AllocatorT >
	win_async_callback_t<HandlerT, AllocatorT> *make_async_callback(HandlerT &&handler, const AllocatorT &allocator)
	{
		typedef win_async_callback_t<HandlerT, AllocatorT> async_callback_t;

		typedef typename AllocatorT::rebind<async_callback_t>::other allocator_t;
		allocator_t alloc = allocator;
		auto p = alloc.allocate(1);
		alloc.construct(p, std::forward<HandlerT>(handler), allocator);

		return p;
	}
}
}


#endif