// handler_allocator_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <cstdint>
#include <type_traits>
#include <functional>

#include <async_io/service/dispatcher.hpp>

template < typename T >
class handler_allocator
	: public std::allocator<handler_allocator<T>>
{
public:
	template<class _Other>
	struct rebind
	{
		typedef handler_allocator<_Other> other;
	};

public:
	handler_allocator()
	{
	}

	template < typename U >
	handler_allocator(const handler_allocator<U> &rhs)
	{}

	T* allocate(std::size_t size)
	{
		return reinterpret_cast<T *>(::operator new(sizeof(T) * size));
	}

	void deallocate(T* pointer, std::size_t sz)
	{
		::operator delete(pointer);
	}
	
};



int _tmain(int argc, _TCHAR* argv[])
{
	async::service::io_dispatcher_t io([](const std::string &msg){});
	io.post([](const std::error_code &e, std::uint32_t size)
	{
		std::cout << __FUNCTION__ << std::endl;
	}, handler_allocator<char>());

	system("pause");
	io.stop();
	return 0;
}

