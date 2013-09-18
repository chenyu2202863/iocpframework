#ifndef __ASYNC_SERVICE_MULTI_BUFFER_HPP
#define __ASYNC_SERVICE_MULTI_BUFFER_HPP

#include <array>
#include <tuple>
#include <string>
#include <vector>

#include "read_write_buffer.hpp"


namespace async { namespace service {

	namespace details
	{
		template < typename T >
		struct pod_param_t
		{
			T val_;

			pod_param_t(T val)
				: val_(val)
			{}

			pod_param_t(pod_param_t && rhs)
				: val_(rhs.val_)
			{}

		private:
			pod_param_t(const pod_param_t &);
			pod_param_t &operator=( const pod_param_t & );
		};

		template < typename T >
		struct no_pod_param_t
		{
			std::uint32_t size_;
			T val_;

			no_pod_param_t(T && val)
				: val_(std::move(val))
				, size_(0)
			{}

			no_pod_param_t(no_pod_param_t && rhs)
				: val_(std::move(rhs.val_))
				, size_(rhs.size_)
			{}

		private:
			no_pod_param_t(const no_pod_param_t &);
			no_pod_param_t &operator=( const no_pod_param_t & );
		};

		template < typename T >
		pod_param_t<T> make_param(T && t, 
								  typename std::enable_if<std::is_pod<T>::value>::type * = nullptr)
		{
			return pod_param_t<T>(t);
		}

		template < typename T >
		no_pod_param_t<T> make_param(T && t,
								  typename std::enable_if<!std::is_pod<T>::value>::type * = nullptr)
		{
			return no_pod_param_t<T>(std::forward<T>(t));
		}


		template < typename T >
		struct buffer_traits_t
		{
			static void buffer(const_array_buffer_t &buffers, const T &val)
			{
				val.buffer(buffers);
			}
		};

		template < typename CharT, typename LessT, typename AllocatorT >
		struct buffer_traits_t<std::basic_string<CharT, LessT, AllocatorT>>
		{
			static void buffer(const_array_buffer_t &buffers, const std::basic_string<CharT, LessT, AllocatorT> &val)
			{
				buffers << service::buffer(val);
			}
		};

		template < typename T, typename AllocatorT >
		struct buffer_traits_t<std::vector<T, AllocatorT>>
		{
			static void buffer(const_array_buffer_t &buffers, const std::vector<T, AllocatorT> &val)
			{
				buffers << service::buffer(val);
			}
		};


		template < typename T >
		const_buffer_t translate_to_buffer(const pod_param_t<T> &val)
		{
			return buffer(reinterpret_cast<const char *>(&val.val_), sizeof(val.val_));;
		}

		template < typename T >
		const_array_buffer_t translate_to_buffer(const no_pod_param_t<T> &val)
		{
			const_array_buffer_t array_buffer;
			array_buffer << buffer(reinterpret_cast<const char *>(&val.size_), sizeof(val.size_));
			buffer_traits_t<T>::buffer(buffer, val.val_);

			return buffer;
		}

		
		template < std::uint32_t I, std::uint32_t N, typename TupleT, typename BuffersT >
		inline typename std::enable_if<I == N>::type for_each(const TupleT& t, BuffersT &buffers)
		{
			buffers << translate_to_buffer(std::get<I>(t));
		}

		template< std::uint32_t I, std::uint32_t N, typename TupleT, typename BuffersT >
		inline typename std::enable_if<I < N>::type for_each(const TupleT& t, BuffersT &buffers)
		{
			buffers << translate_to_buffer(std::get<I>(t));
			for_each<I + 1, std::tuple_size<TupleT>::value - 1>(t, buffers);
		}
	}


	template < typename HandlerT, typename TupleT >
	struct dynamic_param_holder_t
		: HandlerT
	{
		typedef TupleT					tuple_t;
		typedef HandlerT				handler_t;
		typedef const_array_buffer_t	buffers_t;
		typedef std::false_type			is_static_param_t;

		tuple_t tuple_;

		dynamic_param_holder_t(HandlerT &&handler, tuple_t &&tuple)
			: HandlerT(std::move(handler))
			, tuple_(std::move(tuple))
		{}

		dynamic_param_holder_t(dynamic_param_holder_t &&rhs)
			: HandlerT(std::move(static_cast<HandlerT>(rhs)))
			, tuple_(std::move(rhs.tuple_))
		{}

		buffers_t buffers() const
		{
			buffers_t buffer;
			enum { TUPLE_SIZE = std::tuple_size<tuple_t>::value };

			details::for_each<0, TUPLE_SIZE == 1 ? 0 : TUPLE_SIZE>(tuple_, buffer);
			return buffer;
		}

	private:
		dynamic_param_holder_t(const dynamic_param_holder_t &);
		dynamic_param_holder_t &operator=(const dynamic_param_holder_t &);
	};

	template < typename HandlerT, typename ...Args >
	auto make_dynamic_param(HandlerT && handler, Args && ...args)->
		dynamic_param_holder_t<HandlerT, decltype( std::make_tuple(details::make_param(std::forward<Args>(args))...))>
	{
		auto tuple = std::make_tuple(details::make_param(std::forward<Args>(args))...);

		return dynamic_param_holder_t<HandlerT, decltype(tuple)>(std::forward<HandlerT>(handler), std::move(tuple));
	}
	
}}

#endif