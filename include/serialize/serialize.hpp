#ifndef __SERIALIZE_SERIALIZE_HPP
#define __SERIALIZE_SERIALIZE_HPP

#include <cassert>

#include "detail/dispatch.hpp"
#include "detail/buffer.hpp"
#include "detail/serialize_in.hpp"
#include "detail/serialize_out.hpp"


/*
序列化,默认提供写向内存和文件

	mem_serialize\mem_wserialize
	file_serialize


*/

namespace serialize {

	// ------------------------------------
	// CharT 缓冲类型
	// OutT  输出目标

	template < 
		typename CharT, 
		template < typename > class BufferT,
		typename InT = detail::binary_in_t<CharT, BufferT>,
		typename OutT = detail::binary_out_t<CharT, BufferT>
	>
	class serialize_t
		: public InT
		, public OutT
	{
	public:
		typedef serialize_t<CharT, BufferT, InT, OutT>		this_t;
		typedef InT											in_t;
		typedef OutT										out_t;
		typedef BufferT<CharT>								buffer_t;
		typedef typename in_t::is_need_length_t				is_need_length_t;

		typedef typename BufferT<CharT>::value_type			value_type;
		typedef typename BufferT<CharT>::pointer			pointer;
		typedef typename BufferT<CharT>::reference			reference;
		typedef typename BufferT<CharT>::const_pointer		const_pointer;
		typedef typename BufferT<CharT>::const_reference	const_reference;

	private:
		buffer_t buffer_;

	public:
		template < typename ...Arg >
		serialize_t(Arg &&...arg)
			: in_t(buffer_)
			, out_t(buffer_)
			, buffer_(std::forward<Arg>(arg)...)
		{}

		template < std::uint32_t N >
		explicit serialize_t(value_type (&buf)[N])
			: in_t(buffer_)
			, out_t(buffer_)
			, buffer_(buf, N)
		{}

	public:
		pointer buffer()
		{
			return buffer_.buffer();
		}

		const_pointer buffer() const
		{
			return buffer_.buffer();
		}

		std::uint32_t in_length() const
		{
			return in_t::in_length();
		}

		std::uint32_t out_length() const
		{
			return out_t::out_length();
		}

		void read(pointer buf, std::uint32_t len, std::uint32_t pos)
		{
			buffer_.read(buf, len, pos);
		}

		void write(const_pointer buf, std::uint32_t len, std::uint32_t pos)
		{
			buffer_.write(buf, len, pos);
		}
	};

	typedef serialize_t<char, detail::memory_t>		mem_serialize;
	typedef serialize_t<wchar_t, detail::memory_t>	mem_wserialize;

	typedef serialize_t<char, detail::file_t>		file_serialize;

	typedef serialize_t<char, detail::memory_t, detail::text_in_t<char, detail::memory_t>> text_serialize; 

	typedef serialize_t<
		char, 
		detail::memory_t, 
		detail::empty_in_t<char, detail::memory_t>, 
		detail::binary_out_t<char, detail::memory_t>
	> i_serialize;

	typedef serialize_t<
		char, 
		detail::memory_t, 
		detail::binary_in_t<char, detail::memory_t>,
		detail::empty_out_t<char, detail::memory_t>
	> o_serialize;

	typedef serialize_t <
		char,
		detail::memory_t,
		detail::text_in_t<char, detail::memory_t>,
		detail::empty_out_t<char, detail::memory_t>
	> o_text_serialize;

	// -------------------------------------------------

	template < typename CharT, template < typename > class BufferT, typename InT, typename OutT, typename T >
	typename std::enable_if<InT::is_need_in_t::value, serialize_t<CharT, BufferT, InT, OutT> &>::type operator<<(serialize_t<CharT, BufferT, InT, OutT> &os, const T &val)
	{
		detail::select_traits_t<typename std::remove_const<T>::type>::push(os, val);
		return os;
	}

	template < typename CharT, template < typename > class BufferT, typename InT, typename OutT, typename T >
	typename std::enable_if<OutT::is_need_out_t::value, serialize_t<CharT, BufferT, InT, OutT> &>::type &operator>>(serialize_t<CharT, BufferT, InT, OutT> &os, T &val)
	{
		detail::select_traits_t<typename std::remove_const<T>::type>::pop(os, val);
		return os;
	}

}





#endif