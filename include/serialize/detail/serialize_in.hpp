#ifndef __SERIALIZE_IN_HPP
#define __SERIALIZE_IN_HPP

#include <cassert>
#include <type_traits>
#include <cstdint>

namespace serialize { namespace detail {


	template <
		typename CharT,
		template < typename > class BufferT
	>
	struct empty_in_t
	{
		typedef std::false_type is_need_in_t; 
		typedef std::false_type	is_need_length_t;

		empty_in_t(BufferT<CharT> &buffer)
		{}

		template < typename T >
		void push(const T &val)
		{
		}

		template < typename T, std::uint32_t N >
		void push_array(const T(&arr)[N])
		{
		}

		template < typename T >
		void push_pointer(const T * const ptr, std::uint32_t cnt = 1)
		{
		}
	};


	template <
		typename CharT,
		template < typename > class BufferT
	>
	class binary_in_t
	{
		typedef BufferT<CharT>		buffer_t;

	public:
		typedef std::true_type		is_need_in_t; 
		typedef std::true_type		is_need_length_t;

		typedef CharT				value_type;
		typedef CharT *				pointer;
		typedef value_type &		reference;
		typedef const CharT *		const_pointer;
		typedef const value_type &	const_reference;

	public:
		binary_in_t(buffer_t &buffer)
			: in_pos_(0)
			, buffer_(buffer)
		{}

	private:
		std::uint32_t in_pos_;
		buffer_t &buffer_;

	public:
		std::uint32_t in_length() const
		{
			return in_pos_;
		}

	public:
		template < typename T >
		void push(const T &val)
		{
			static_assert( std::is_pod<T>::value, "T must be a POD type" );

			// 检测T类型
			assert(sizeof( T ) + in_pos_ <= buffer_.buffer_length());
			if(sizeof( T ) + in_pos_ > buffer_.buffer_length())
				throw std::out_of_range("sizeof(T) + pos_ > bufLen_");

			const_pointer buf = reinterpret_cast< const_pointer >( &val );
			buffer_.write(buf, sizeof( T ), in_pos_);
			in_pos_ += sizeof( T );
		}

		template < typename T, std::uint32_t N >
		void push_array(const T(&arr)[N])
		{
			static_assert( std::is_pod<T>::value, "T must be a POD type" );

			const std::uint32_t len = sizeof( T ) * N;
			assert(len + in_pos_ <= buffer_.buffer_length());
			if(len + in_pos_ > buffer_.buffer_length())
				throw std::out_of_range("sizeof(T) + pos_ > bufLen_");

			const_pointer buf = reinterpret_cast< const_pointer >( &arr );
			buffer_.write(buf, len, in_pos_);
			in_pos_ += len;
		}

		template < typename T >
		void push_pointer(const T * const ptr, std::uint32_t cnt = 1)
		{
			static_assert( std::is_pod<T>::value, "T must be a POD type" );

			const std::uint32_t len = sizeof( T ) * cnt;
			assert(len + in_pos_ <= buffer_.buffer_length());
			if(len + in_pos_ > buffer_.buffer_length())
				throw std::out_of_range("sizeof(T) + pos_ > bufLen_");

			const_pointer buf = reinterpret_cast< const_pointer >( ptr );
			buffer_.write(buf, len, in_pos_);
			in_pos_ += len;
		}
	};


	template <
		typename CharT,
		template < typename > class BufferT
	>
	class text_in_t
	{
		typedef BufferT<CharT>		buffer_t;

	public:
		typedef std::true_type		is_need_in_t; 
		typedef std::false_type		is_need_length_t;
		typedef CharT				value_type;
		typedef CharT *				pointer;
		typedef value_type &		reference;
		typedef const CharT *		const_pointer;
		typedef const value_type &	const_reference;

	public:
		text_in_t(buffer_t &buffer)
			: in_pos_(0)
			, buffer_(buffer)
		{}

	private:
		std::uint32_t in_pos_;
		buffer_t &buffer_;

	public:
		std::uint32_t in_length() const
		{
			return in_pos_;
		}

	public:
		template < typename T >
		void push(const T &val)
		{
			static_assert( std::is_pod<T>::value, "T must be a POD type" );

			// 检测T类型
			assert(sizeof( T ) + in_pos_ <= buffer_.buffer_length());
			if(sizeof( T ) + in_pos_ > buffer_.buffer_length())
				throw std::out_of_range("sizeof(T) + pos_ > bufLen_");

			auto t = std::to_string(val);
			buffer_.write(t.data(), t.size(), in_pos_);
			in_pos_ += t.size();
		}

		void push(char val)
		{
			// 检测T类型
			assert(sizeof(val) + in_pos_ <= buffer_.buffer_length());
			if(sizeof(val) + in_pos_ > buffer_.buffer_length())
				throw std::out_of_range("sizeof(T) + pos_ > bufLen_");

			buffer_.write(&val, sizeof(val), in_pos_);
			in_pos_ += sizeof(val);
		}

		template < typename T >
		void push_pointer(const T * const ptr, std::uint32_t cnt = 1)
		{
			static_assert( std::is_pod<T>::value, "T must be a POD type" );

			const std::uint32_t len = sizeof( T ) * cnt;
			assert(len + in_pos_ <= buffer_.buffer_length());
			if(len + in_pos_ > buffer_.buffer_length())
				throw std::out_of_range("sizeof(T) + pos_ > bufLen_");

			buffer_.write(ptr, cnt, in_pos_);
			in_pos_ += cnt;
		}
	};
}
}






#endif