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
	
	template < typename T >
	struct arg_t
	{
		T val_;
		static const std::uint32_t arg_size = 1;

		arg_t(T &&val)
			: val_(std::move(val))
		{}
		arg_t(arg_t &&rhs)
			: val_(std::move(rhs.val_))
		{}

		arg_t(const arg_t &) = delete;
		arg_t &operator=(const arg_t &) = delete;

		template < std::uint32_t N, typename BuffersT >
		void cast(BuffersT &buffers) const
		{
			auto buffer_val = buffer(val_);
			buffers[N].buf = const_cast<char *>(buffer_val.data());
			buffers[N].len = buffer_val.size();
		}
	};

	template < typename T, typename AllocatorT >
	struct arg_t<std::vector<T, AllocatorT>>
	{
		using type = std::vector<T, AllocatorT>;
		static const std::uint32_t arg_size = 2;

		std::uint32_t size_;
		type val_;

		arg_t(type &&val)
			: size_(val.size())
			, val_(std::move(val))
		{}
		arg_t(arg_t &&rhs)
			: size_(rhs.size_)
			, val_(std::move(rhs.val_))
		{}

		arg_t(const arg_t &) = delete;
		arg_t &operator=(const arg_t &) = delete;

		template < std::uint32_t N, typename BuffersT >
		void cast(BuffersT &buffers) const
		{
			auto size_val = buffer(size_);
			buffers[N - 1].buf = const_cast<char *>(size_val.data());
			buffers[N - 1].len = size_val.size();

			auto buffer_val = buffer(val_);
			buffers[N].buf = const_cast<char *>(buffer_val.data());
			buffers[N].len = buffer_val.size();
		}
	};

	template < typename T, typename AllocatorT >
	struct arg_t<std::basic_string<T, std::char_traits<T>, AllocatorT>>
	{
		using type = std::basic_string<T, std::char_traits<T>, AllocatorT>;
		static const std::uint32_t arg_size = 2;

		std::uint32_t size_;
		type val_;

		arg_t(type &&val)
			: size_(val.size())
			, val_(std::move(val))
		{}
		arg_t(arg_t &&rhs)
			: size_(rhs.size_)
			, val_(std::move(rhs.val_))
		{}

		arg_t(const arg_t &) = delete;
		arg_t &operator=(const arg_t &) = delete;

		template < std::uint32_t N, typename BuffersT >
		void cast(BuffersT &buffers) const
		{
			auto size_val = buffer(size_);
			buffers[N - 1].buf = const_cast<char *>(size_val.data());
			buffers[N - 1].len = size_val.size();

			auto buffer_val = buffer(val_);
			buffers[N].buf = const_cast<char *>(buffer_val.data());
			buffers[N].len = buffer_val.size();
		}
	};


	template < typename HandlerT, typename ...Args >
	struct param_t
		: HandlerT 
	{
		using tuple_t = std::tuple<arg_t<typename std::remove_reference<Args>::type>...>;
		tuple_t params_;

		using buffers_t = std::array<WSABUF, details::args_count_t<tuple_t>::value>;
		buffers_t buffers_;

		param_t(HandlerT &&handler, Args &&...args)
			: HandlerT(std::forward<HandlerT>(handler))
			, params_(std::make_tuple(std::forward<Args>(args)...))
		{}

		param_t(param_t &&param)
			: HandlerT(std::move(param))
			, params_(std::move(param.params_))
		{}

		param_t(const param_t &) = delete;
		param_t &operator=(const param_t &) = delete;


		const buffers_t &buffers()
		{
			details::tuple_for_each_t<buffers_t::_EEN_SIZE - 1, std::tuple_size<tuple_t>::value - 1>::run(buffers_, params_);
			return buffers_;
		}
	};


	template < typename HandlerT, typename ...Args >
	param_t<HandlerT, Args...> make_param(HandlerT &&handler, Args &&...args)
	{
		return param_t<HandlerT, Args...>(std::forward<HandlerT>(handler), std::forward<Args>(args)...);
	}
}}

#endif