#pragma once

#include <type_traits>
#include <cstdint>
#include <memory>


struct handler_allocator
{
	template< typename T >
	struct rebind
	{	// convert this type to allocator<_Other>
		typedef T other;
	};

public:
	handler_allocator()
		: in_use_(false)
	{
	}

	void* allocate(std::uint32_t size)
	{
		if (!in_use_ && size < 1024)
		{
			in_use_ = true;
			return &storage_;
		}
		else
		{
			return ::operator new(size);
		}
	}

	void deallocate(void* pointer)
	{
		if (pointer == &storage_)
		{
			in_use_ = false;
		}
		else
		{
			::operator delete(pointer);
		}
	}

private:
	// Storage space used for handler-based custom memory allocation.
	std::aligned_storage<1024>::type storage_;

	// Whether the handler-based custom allocation storage has been used.
	bool in_use_;
};

template < typename HandlerT >
struct custom_handler_t
{
	handler_allocator &allocator_;
	HandlerT handler_;

	custom_handler_t(handler_allocator &allocator, HandlerT &&handler)
		: allocator_(allocator)
		, handler_(std::move(handler))
	{}

	template < typename T >
	void operator()(const T &t)
	{
		handler_(std::cref(t));
	}

	template < typename T1, typename T2 >
	void operator()(const T1 &t1, const T2 &t2)
	{
		handler_(std::cref(t1), std::cref(t2));
	}

	friend void *allocate_handler(std::uint32_t sz, custom_handler_t<HandlerT> *this_handler)
	{
		return this_handler->allocator_.allocate(sz);
	}

	friend void deallocate_handler(void* pointer, std::uint32_t /*size*/,
		custom_handler_t<HandlerT>* this_handler)
	{
		this_handler->allocator_.deallocate(pointer);
	}
};

template < typename HandlerT >
custom_handler_t<HandlerT> make_custom_handler(handler_allocator &allocator, HandlerT &&handler)
{
	return custom_handler_t<HandlerT>(allocator, std::forward<HandlerT>(handler));
}
