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

namespace serialize
{

	// ------------------------------------
	// CharT 缓冲类型
	// OutT  输出目标

	template < typename CharT, typename OutT >
	class serialize_t
		: public detail::serialize_in_t<CharT, serialize_t<CharT, OutT> >
		, public detail::serialize_out_t<CharT, serialize_t<CharT, OutT> >
	{
	public:
		typedef typename OutT::value_type		value_type;
		typedef typename OutT::pointer			pointer;
		typedef typename OutT::reference		reference;
		typedef typename OutT::const_pointer	const_pointer;
		typedef typename OutT::const_reference	const_reference;

	private:
		typedef detail::serialize_in_t<CharT, serialize_t<CharT, OutT> >	InType;
		typedef detail::serialize_out_t<CharT, serialize_t<CharT, OutT> >	OutType;
		typedef OutT														BufferType;

	protected:
		BufferType buffer_;

	public:
		template < typename Arg1 >
		serialize_t(Arg1 &&arg1)
			: buffer_(std::forward<Arg1>(arg1))
		{}

		template < typename Arg1, typename Arg2 >
		serialize_t(Arg1 &&arg1, Arg2 &&arg2)
			: buffer_(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2))
		{}

		template < typename Arg1, typename Arg2, typename Arg3 >
		serialize_t(Arg1 &&arg1, Arg2 &&arg2, Arg3 &&arg3)
			: buffer_(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3))
		{}

		template < size_t N >
		explicit serialize_t(value_type (&buf)[N])
			: buffer_(buf)
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

		size_t in_length() const
		{
			return InType::in_length();
		}

		size_t out_length() const
		{
			return OutType::out_length();
		}

		size_t buffer_length() const
		{
			return buffer_.buffer_length();
		}

		size_t data_length() const
		{
			return buffer_.data_length();
		}

		void read(pointer buf, size_t len, size_t pos)
		{
			buffer_.read(buf, len, pos);
		}

		void write(const_pointer buf, size_t len, size_t pos)
		{
			buffer_.write(buf, len, pos);
		}
	};

	typedef serialize_t<char,	detail::memory_t<char> >		mem_serialize;
	typedef serialize_t<wchar_t, detail::memory_t<wchar_t> >	mem_wserialize;

	typedef serialize_t<char,	detail::file_t>					file_serialize;


	// -------------------------------------------------

	template < typename CharT, typename OutT, typename T >
	inline serialize_t<CharT, OutT> &operator<<(serialize_t<CharT, OutT> &os, const T &val)
	{
		detail::select_traits_t<typename std::remove_const<T>::type>::push(os, val);
		return os;
	}

	template < typename CharT, typename OutT, typename T >
	inline serialize_t<CharT, OutT> &operator>>(serialize_t<CharT, OutT> &os, T &val)
	{
		detail::select_traits_t<typename std::remove_const<T>::type>::pop(os, val);
		return os;
	}

}





#endif