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
		service::const_buffer_t translate_to_buffer(const T &val, 
			typename std::enable_if<std::is_pod<T>::value>::type * = nullptr)
		{
			return service::const_buffer_t(reinterpret_cast<const char *>(&val), sizeof(val));
		}

		template < typename T >
		const_array_buffer_t translate_to_buffer(const T &val, 
			typename std::enable_if<!std::is_pod<T>::value>::type * = nullptr)
		{
			return val.buffers();
		}

		template < typename CharT, typename CharTraits, typename AllocatorT >
		service::const_buffer_t translate_to_buffer(const std::basic_string<CharT, CharTraits, AllocatorT> &val)
		{
			return service::const_buffer_t(val.data(), val.size());
		}

		template < typename T, typename AllocatorT >
		service::const_buffer_t translate_to_buffer(const std::vector<T, AllocatorT> &val)
		{
			static_assert(std::is_pod<T>::value, "T must be a pod type");
			return service::const_buffer_t(val.data(), val.size() * sizeof(T));
		}


		template < std::uint32_t N >
		struct static_tuple_buffer_t
		{
			template < typename TupleT, typename ArrayT >
			static void make(const TupleT &tuple_val, ArrayT &array_val)
			{
				array_val[N - 1] = translate_to_buffer(std::get<N - 1>(tuple_val));
				static_tuple_buffer_t<N - 1>::make(tuple_val, array_val);
			}
		};

		template <>
		struct static_tuple_buffer_t<1>
		{
			template < typename TupleT, typename ArrayT >
			static void make(const TupleT &tuple_val, ArrayT &array_val)
			{
				array_val[0] = translate_to_buffer(std::get<0>(tuple_val));
			}
		};


		template < std::uint32_t N >
		struct dynamic_tuple_buffer_t
		{
			template < typename TupleT >
			static void make(const TupleT &tuple_val, const_array_buffer_t &array_val)
			{
				array_val << std::move(translate_to_buffer(std::get<N - 1>(tuple_val)));
				dynamic_tuple_buffer_t<N - 1>::make(tuple_val, array_val);
			}
		};

		template <>
		struct dynamic_tuple_buffer_t<1>
		{
			template < typename TupleT >
			static void make(const TupleT &tuple_val, const_array_buffer_t &array_val)
			{
				array_val << std::move(translate_to_buffer(std::get<0>(tuple_val)));
			}
		};

		
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
	

	
	/*template < typename HandlerT, typename ...T >
	auto make_param(HandlerT &&handler, T &&...val)->
	param_holder_t<decltype(std::make_tuple(std::forward<T>(val)...)), HandlerT>
	{
	auto val = std::make_tuple(std::forward<T>(val)...);
	typedef decltype(val) tuple_t;

	return param_holder_t<tuple_t, HandlerT>(std::forward<tuple_t>(val), std::forward<HandlerT>(handler));
	}

	auto param = make_param(std::move([](){}), std::move(10), std::move('a'));*/

	template < typename TupleT, typename HandlerT >
	struct static_param_holder_t
		: HandlerT
	{
		enum { PARAM_SIZE = std::tuple_size<TupleT>::value };

		typedef TupleT			tuple_t;
		typedef HandlerT		handler_t;
		typedef std::array<service::const_buffer_t, PARAM_SIZE> buffers_t;
		typedef std::true_type  is_static_param_t;

		tuple_t val_;

		static_param_holder_t(tuple_t &&val, HandlerT &&handler)
			: HandlerT(std::move(handler))
			, val_(std::move(val))
		{}

		static_param_holder_t(static_param_holder_t &&rhs)
			: HandlerT(std::move(static_cast<HandlerT>(rhs)))
			, val_(std::move(rhs.val_))
		{}

		buffers_t buffers() const
		{
			buffers_t buffer;

			details::static_tuple_buffer_t<PARAM_SIZE>::make(val_, buffer);

			return buffer;
		}

	private:
		static_param_holder_t(const static_param_holder_t &);
		static_param_holder_t &operator=(const static_param_holder_t &);
	};

	template < typename TupleT, typename HandlerT >
	static_param_holder_t<TupleT, HandlerT> make_static_param(TupleT &&tuple, HandlerT &&handler)
	{
		return static_param_holder_t<TupleT, HandlerT>(std::forward<TupleT>(tuple), std::forward<HandlerT>(handler));
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

		dynamic_param_holder_t(tuple_t &&tuple, HandlerT &&handler)
			: HandlerT(std::move(handler))
			, tuple_(std::move(val))
		{}

		dynamic_param_holder_t(dynamic_param_holder_t &&rhs)
			: HandlerT(std::move(static_cast<HandlerT>(rhs)))
			, tuple_(std::move(rhs.tuple_))
		{}

		buffers_t buffers() const
		{
			buffers_t buffer;
			
			//details::dynamic_tuple_buffer_t<std::tuple_size<tuple_t>::value>::make(tuple_, buffer);

			details::for_each<0, std::tuple_size<TupleT>::value>(tuple_, buffer);
			return buffer;
		}
	};

	template < typename TupleT, typename HandlerT >
	dynamic_param_holder_t<HandlerT, TupleT> make_dynamic_param(TupleT &&tuple, HandlerT &&handler)
	{
		return dynamic_param_holder_t<HandlerT, TupleT>(
			std::forward<HandlerT>(handler), 
			std::forward<TupleT>(tuple));	
	}
	
}}

#endif