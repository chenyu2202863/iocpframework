// handler_allocator_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <cstdint>
#include <type_traits>
#include <functional>

class handler_allocator
{
public:
	handler_allocator()
		: in_use_(false)
	{
	}

	void* allocate(std::size_t size)
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

	void operator()()
	{
		handler_();
	}

	friend void *allocate_handler(std::uint32_t sz, custom_handler_t<HandlerT> *this_handler)
	{
		return this_handler->allocator_.allocate(sz);
	}

	friend void *deallocate_handler(void* pointer, std::uint32_t /*size*/,
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


template < typename HandlerT >
struct second_handler_t
{
	HandlerT handler_;
	std::string msg_;

	second_handler_t(HandlerT &&handler, std::string &&msg)
		: handler_(std::move(handler))
		, msg_(std::move(msg))
	{}

	friend void *allocate_handler(std::uint32_t sz, second_handler_t<HandlerT> *this_handler)
	{
		return allocate_handler(sz, this_handler->handler_);
	}

	friend void *deallocate_handler(void* pointer, std::uint32_t /*size*/,
		second_handler_t<HandlerT>* this_handler)
	{
		return deallocate_handler(this, sz, this_handler->handler_);
	}
};

template < typename HandlerT >
second_handler_t<HandlerT> make_second_handler(HandlerT &&handler)
{
	return second_handler_t<HandlerT>(std::forward<HandlerT>(handler), "123");
}


void *allocate_handler(std::uint32_t sz, ...)
{
	return ::operator new(sz);
}

template < typename HandlerT >
void *allocate_handler(std::uint32_t sz, HandlerT &handler)
{
	return allocate_handler(sz, std::addressof(handler));
}



template < typename HandlerT >
struct my_handler_t
{
	HandlerT handler_;

	int n_;
	double m_;

	my_handler_t(HandlerT &&handler, int n, double m)
		: handler_(std::move(handler))
		, n_(n)
		, m_(m)
	{}
};

template < typename HandlerT >
void construct(HandlerT &handler)
{
	void *p = allocate_handler(sizeof(my_handler_t<HandlerT>), handler);
	::new (p) my_handler_t<HandlerT>(std::forward<HandlerT>(handler), 1, 10.0);
}


void test()
{
	struct op_t
	{
		void print()
		{
			std::cout << __FUNCTION__ << std::endl;
		}
	}op;

	auto bind_handler = std::bind(&op_t::print, std::ref(op));

	handler_allocator allocator;
	auto handler = make_custom_handler(allocator, std::move(bind_handler));

	auto second_handler = make_second_handler(handler);

	construct(bind_handler);
	construct(handler);
	construct(second_handler);
}

int _tmain(int argc, _TCHAR* argv[])
{
	test();


	return 0;
}

