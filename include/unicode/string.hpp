#ifndef __UNICODE_STRING_HPP
#define __UNICODE_STRING_HPP

#include <string>
#include <iterator>
#include <atlbase.h>
#include <atlenc.h>

//////////////////////////////////////////////////////////////////////////////

namespace unicode
{

	//////////////////////////////////////////////////////////////////////////////

	namespace meta 
	{

		//////////////////////////////////////////////////////////////////////////////

		// Meta if
		template<bool C, typename T1, typename T2>
		struct if_ { typedef T1 type; };
		template<typename T1, typename T2>
		struct if_<false, T1, T2> { typedef T2 type; };

		template<size_t Size>
		struct utf_char_selector
		{
			class unknown_char_type;
			typedef 
				typename if_<sizeof(wchar_t) == Size, wchar_t,
				typename if_<sizeof(unsigned short) == Size, unsigned short,
				typename if_<sizeof(unsigned int) == Size, unsigned int, 
				unknown_char_type>::type>::type>::type type;
		};

		//////////////////////////////////////////////////////////////////////////////

	} // namespace meta

	//////////////////////////////////////////////////////////////////////////////

	using std::size_t;


	typedef char								utf8_char;
	typedef meta::utf_char_selector<2>::type	utf16_char;
	typedef meta::utf_char_selector<4>::type	utf32_char;

	typedef std::basic_string<utf8_char>		utf8_string;
	typedef std::basic_string<utf16_char>		utf16_string;
	typedef std::basic_string<utf32_char>		utf32_string;

#ifdef UNICODE
	typedef utf16_char   char_t;
	typedef utf16_string string_t;
#else
	typedef utf8_char    char_t;
	typedef utf8_string  string_t;
#endif

	size_t const npos = (size_t)-1;

	utf8_string  utf16_to_utf8(utf16_char const* str, size_t size = npos);
	utf8_string  utf32_to_utf8(utf32_char const* str, size_t size = npos);

	utf16_string utf8_to_utf16(utf8_char const* str, size_t size = npos);
	utf16_string utf32_to_utf16(utf32_char const* str, size_t size = npos);

	utf32_string utf8_to_utf32(utf8_char const* str, size_t size = npos);
	utf32_string utf16_to_utf32(utf16_char const* str, size_t size = npos);

	//////////////////////////////////////////////////////////////////////////////

	namespace aux 
	{

		//////////////////////////////////////////////////////////////////////////////

		template<typename C, typename T1, typename T2>
		struct selector;

		template<typename T1, typename T2>
		struct selector<utf8_char, T1, T2>
		{
			selector(T1 t1, T2) : result(t1) {}
			typedef T1 type;
			type result;
		};

		template<typename T1, typename T2>
		struct selector<utf16_char, T1, T2>
		{
			selector(T1, T2 t2) : result(t2) {}
			typedef T2 type;
			type result;
		};

		template<typename T1, typename T2>
		inline typename selector<char_t, T1, T2>::type select(T1 t1, T2 t2)
		{
			return selector<char_t, T1, T2>(t1, t2).result;
		}
	
		template<typename CharT, typename T1, typename T2>
		inline typename selector<CharT, T1, T2>::type select(T1 t1, T2 t2)
		{
			return selector<CharT, T1, T2>(t1, t2).result;
		}

		template<typename C>
		struct converter;

		template<>
		struct converter<utf8_char>
		{
			static utf8_string utf(utf8_char const* str, size_t size)
			{
				utf16_string wide = CA2W(str);
				utf8_string tmp = CW2A(wide.c_str(), CP_UTF8);
				return tmp;
			}
			static utf8_string utf(utf16_char const* str, size_t size)
			{
				utf8_string tmp = CW2A(str, CP_UTF8);
				return tmp; //utf16_to_utf8(str, size);
			}
			static utf8_string utf(utf32_char const* str, size_t size)
			{
				return utf32_to_utf8(str, size);
			}
		};

		template<>
		struct converter<utf16_char>
		{
			static utf16_string utf(utf8_char const* str, size_t size)
			{
				utf16_string tmp = CA2W(str, CP_UTF8);
				return tmp;//utf8_to_utf16(str, size);
			}
			static utf16_string utf(utf16_char const* str, size_t)
			{
				return str;
			}
			static utf16_string utf(utf32_char const* str, size_t size)
			{
				return utf32_to_utf16(str, size);
			}
		};

		template<>
		struct converter<utf32_char>
		{
			static utf32_string utf(utf8_char const* str, size_t size)
			{
				return utf8_to_utf32(str, size);
			}
			static utf32_string utf(utf16_char const* str, size_t size)
			{
				return utf16_to_utf32(str, size);
			}
			static utf32_string utf(utf32_char const* str, size_t)
			{
				return str;
			}
		};

		//////////////////////////////////////////////////////////////////////////////

	} // namespace aux


	template < typename CharT >
	struct translate_t;

	template < >
	struct translate_t<char>
	{
		static const std::string &utf(const std::string &src, size_t code = CP_ACP)
		{
			return src;
		}

		static std::string utf(const std::wstring &src, size_t code = CP_ACP)
		{
			std::string tmp(CW2A(src.c_str(), code));
			return tmp;
		}
	};

	template < >
	struct translate_t<wchar_t>
	{
		static const std::wstring &utf(const std::wstring &src, size_t code = CP_ACP)
		{
			return src;
		}

		static std::wstring utf(const std::string &src, size_t code = CP_ACP)
		{
			std::wstring tmp(CA2W(src.c_str(), code));
			return tmp;
		}
	};

	//////////////////////////////////////////////////////////////////////////////

	namespace detail
	{
		template < typename T, typename U >
		struct select_type;
	
		template < >
		struct select_type<char, char>
		{
			static const std::basic_string<char> &convert(const std::basic_string<char> &str, size_t code)
			{
				return str;
			}
		};

		template < >
		struct select_type<char, wchar_t>
		{
			static std::basic_string<wchar_t> convert(const std::basic_string<char> &str, size_t code)
			{
				return std::move(std::basic_string<wchar_t>(CA2W(str.c_str(), code)));
			}
		};

		template < >
		struct select_type<wchar_t, char>
		{
			static std::basic_string<char> convert(const std::basic_string<wchar_t> &str, size_t code)
			{
				return std::move(std::basic_string<char>(CW2A(str.c_str(), code)));
			}
		};

		template < >
		struct select_type<wchar_t, wchar_t>
		{
			static const std::basic_string<wchar_t> &convert(const std::basic_string<wchar_t> &str, size_t code)
			{
				return str;
			}
		};

		template < typename CharT >
		struct ret_helper
		{
			const std::basic_string<CharT> &str_;
			size_t code_;

			ret_helper(const std::basic_string<CharT> &str, size_t code)
				: str_(str)
				, code_(code)
			{}

			operator std::basic_string<char>() const
			{
				return std::move(select_type<CharT, char>::convert(str_, code_));
			}

			operator std::basic_string<wchar_t>() const
			{
				return std::move(select_type<CharT, wchar_t>::convert(str_, code_));
			}
		};
	}

	template < typename CharT >
	detail::ret_helper<CharT> to(const std::basic_string<CharT> &str, size_t code = CP_ACP)
	{
		return std::move(detail::ret_helper<CharT>(str, code));
	}


	template < typename CharT >
	inline std::string to_a(const std::basic_string<CharT> &src, size_t code = CP_ACP)
	{
		return translate_t<char>::utf(src, code);
	}

	template < typename CharT >
	inline std::wstring to_w(const std::basic_string<CharT> &src, size_t code = CP_ACP)
	{
		return translate_t<wchar_t>::utf(src, code);
	}
	
	template < typename CharT >
	inline string_t to_t(const std::basic_string<CharT> &src, size_t code = CP_ACP)
	{
		return translate_t<string_t::value_type>::utf(src, code);
	}

	template < typename CharT >
	inline std::string to_a(const CharT *src, size_t code = CP_ACP)
	{
		return translate_t<char>::utf(src, code);
	}

	template < typename CharT >
	inline std::wstring to_w(const CharT *src, size_t code = CP_ACP)
	{
		return translate_t<wchar_t>::utf(src, code);
	}

	template < typename CharT >
	inline string_t to_t(const CharT *src, size_t code = CP_ACP)
	{
		return translate_t<string_t::value_type>::utf(src, code);
	}

	

} 




#endif

