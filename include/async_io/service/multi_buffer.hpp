#ifndef __ASYNC_SERVICE_MULTI_BUFFER_HPP
#define __ASYNC_SERVICE_MULTI_BUFFER_HPP

#include <array>
#include <tuple>
#include <string>
#include <vector>

#include "read_write_buffer.hpp"


namespace async { namespace service {

	template < typename T >
	struct arg_t;

	namespace details  
	{

		template < typename ...Args >
		struct args_count_t;

		template < typename T, typename ...Args >
		struct args_count_t<T, Args...> 
		{
			static const std::uint32_t value = 
				T::arg_size +
				args_count_t<Args...>::value;
		};

		template < typename T >
		struct args_count_t<T> 
		{
			static const std::uint32_t value = T::arg_size;
		};

		template < typename ...Args >
		struct args_count_t<std::tuple<Args...>>
			: args_count_t<Args...>
		{};


		template < std::uint32_t ArrayIdx, std::uint32_t TupleIdx >
		struct tuple_for_each_t
		{
			template < typename BuffersT, typename ...Args >
			static void run(BuffersT &buffers, const std::tuple<Args...> &val)
			{
				using type = typename std::tuple_element<TupleIdx, std::tuple<Args...>>::type;

				std::get<TupleIdx>(val).cast<ArrayIdx>(buffers);

				tuple_for_each_t<ArrayIdx - type::arg_size, TupleIdx - 1>::run(buffers, val);
			}
		};

		template < std::uint32_t ArrayIdx >
		struct tuple_for_each_t<ArrayIdx, 0> 
		{
			template < typename BuffersT, typename ...Args >
			static void run(BuffersT &buffers, const std::tuple<Args...> &val)
			{
				using type = typename std::tuple_element<0, std::tuple<Args...>>::type;

				std::get<0>(val).cast<ArrayIdx>(buffers);
			}
		};

	}
	

	template < std::size_t Idx, typename T, std::size_t N, typename U >
	void unpack(std::array<T, N> &buffers, const U &val)
	{
		auto buffer_val = buffer(val);
		buffers[Idx].buf = const_cast<char *>(buffer_val.data());
		buffers[Idx].len = buffer_val.size();
	}

	template < std::size_t Idx, typename T, std::size_t N, typename U, typename ...Args >
	void unpack(std::array<T, N> &buffers, const U &val, const Args &...args)
	{
		auto buffer_val = buffer(val);
		buffers[Idx].buf = const_cast<char *>(buffer_val.data());
		buffers[Idx].len = buffer_val.size();

		unpack<Idx + 1>(buffers, args...);
	}

	template < typename T, std::size_t N, typename ...Args >
	void unpack(std::array<T, N> &buffers, const Args &...args)
	{
		unpack<0>(buffers, args...);
	}
}}

#endif