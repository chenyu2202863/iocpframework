#ifndef __SERIALIZE_OUT_HPP
#define __SERIALIZE_OUT_HPP


#include <cassert>
#include <type_traits>
#include <cstdint>

namespace serialize { namespace detail {


	template <
		typename CharT,
		template < typename > class BufferT
	>
	struct empty_out_t
	{
		typedef std::false_type		is_need_out_t; 
		typedef std::false_type		is_need_length_t;

		empty_out_t(BufferT<CharT> &buffer)
		{}

		template < typename T >
		void pop(const T &val)
		{
		}

		template < typename T, std::uint32_t N >
		void pop_array(const T(&arr)[N])
		{
		}

		template < typename T >
		void pop_pointer(const T * const ptr, std::uint32_t cnt = 1)
		{
		}
	};

	template <
		typename CharT,
		template < typename > class BufferT
	>
	class binary_out_t
	{
		typedef BufferT<CharT>		buffer_t;

	public:
		typedef std::true_type		is_need_out_t; 
		typedef CharT				value_type;
		typedef CharT *				pointer;
		typedef value_type &		reference;
		typedef const CharT *		const_pointer;
		typedef const value_type &	const_reference;

	private:
		std::uint32_t out_pos_;
		buffer_t &buffer_;

	public:
		binary_out_t(buffer_t &buffer)
			: out_pos_(0)
			, buffer_(buffer)
		{}

	public:
		std::uint32_t out_length() const
		{
			return out_pos_;
		}

	public:
		template < typename T >
		void pop(T &val)
		{
			static_assert( std::is_pod<T>::value, "T must POD type" );

			assert(sizeof( T ) + out_pos_ <= buffer_.buffer_length());
			if(sizeof( T ) + out_pos_ > buffer_.buffer_length())
				throw std::out_of_range("sizeof(T) + pos_ > bufLen_");

			pointer buf = reinterpret_cast< pointer >( &val );
			buffer_.read(buf, sizeof( T ), out_pos_);
			out_pos_ += sizeof( T );
		}

		template < typename T, std::uint32_t N >
		void pop_array(T(&arr)[N])
		{
			static_assert( std::is_pod<T>::value, "T must POD type" );

			const std::uint32_t len = sizeof( T ) * N;
			assert(len + out_pos_ <= buffer_.buffer_length());
			if(len + out_pos_ > buffer_.buffer_length())
				throw std::out_of_range("sizeof(T) + pos_ > bufLen_");

			pointer buf = reinterpret_cast< pointer >( &arr );
			buffer_.read(buf, len, out_pos_);
			out_pos_ += len;
		}

		template < typename T >
		void pop_pointer(T *ptr, std::uint32_t cnt = 1)
		{
			static_assert( std::is_pod<T>::value, "T must POD type" );

			const std::uint32_t len = sizeof( T ) * cnt;
			assert(len + out_pos_ <= buffer_.buffer_length());
			if(len + out_pos_ > buffer_.buffer_length())
				throw std::out_of_range("sizeof(T) + pos_ > bufLen_");

			pointer buf = reinterpret_cast< pointer >( ptr );
			buffer_.read(buf, len, out_pos_);
			out_pos_ += len;
		}
	};
}
}




#endif