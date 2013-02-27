#ifndef __EXTEND_STL_STRING_ALGORITHM_HPP
#define __EXTEND_STL_STRING_ALGORITHM_HPP

#include <iomanip>
#include <algorithm>
#include <functional>
#include <type_traits>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>
#include <cctype>
#include "../../utility/select.hpp"

/*
大小写转换
	to_upper
	to_lower
	to_number
	to_string

吃掉空格
	trim_left
	trim_right
	trim

删除
	erase

不分大小写比较
	compare_no_case

匹配查找
	find_nocase

起始、结束匹配
	is_start_with
	is_end_with

分割
	split


*/


namespace stdex
{
	namespace detail
	{
		template < typename CharT, typename FuncT >
		struct ret_helper_t
		{
			std::basic_string<CharT> &str_;
			FuncT &func_;
			ret_helper_t(std::basic_string<CharT> &str, FuncT &func)
				: str_(str)
				, func_(func)
			{}

			operator std::basic_string<CharT> &()
			{
				std::transform(str_.begin(), str_.end(), str_.begin(), func_);

				return str_;
			}

			operator size_t() const
			{
				std::transform(str_.begin(), str_.end(), str_.begin(), func_);
				return str_.length();
			}
		};
	}


	// to_upper
	template < typename CharT >
	detail::ret_helper_t<CharT, decltype(::toupper)> to_upper(std::basic_string<CharT> &str)
	{
		return detail::ret_helper_t<CharT, decltype(::toupper)>(str, ::toupper);
	}
	
	template < typename CharT >
	detail::ret_helper_t<CharT, decltype(::toupper)> to_upper(const std::basic_string<CharT> &str)
	{
		return detail::ret_helper_t<CharT, decltype(::toupper)>(const_cast<std::basic_string<CharT> &>(str), ::toupper);
	}

	template < typename CharT, size_t N >
	inline void to_upper(CharT (&str)[N])
	{
		std::transform(str, str + N, str, ::toupper);
	}


	// to_lower

	template < typename CharT >
	detail::ret_helper_t<CharT, decltype(::tolower)> to_lower(std::basic_string<CharT> &str)
	{
		return detail::ret_helper_t<CharT, decltype(::tolower)>(str, ::tolower);
	}

    template < typename CharT >
    detail::ret_helper_t<CharT, decltype(::tolower)> to_lower(const std::basic_string<CharT> &str)
    {
        return detail::ret_helper_t<CharT, decltype(::tolower)>(const_cast<std::basic_string<CharT> &>(str), ::tolower);
    }

	template < typename CharT, size_t N >
	inline void to_lower(CharT (&str)[N])
	{
		std::transform(str, str + N, str, ::tolower);
	}


	namespace detail
	{
		template < typename CharT >
		struct to_number_helper_t
		{
			std::basic_istringstream<CharT> is_;

			to_number_helper_t(const std::basic_string<CharT> &str)
				: is_(str)
			{}

			template < typename T >
			operator T()
			{
				static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value, "T must is a number");

				T val = 0;
				is_ >> val;
				return val;
			}

			operator bool()
			{
				bool val = false;
				is_ >> std::boolalpha >> val;
				return val;
			}
		};
	}

	// to number

	template < typename CharT >
	inline detail::to_number_helper_t<CharT> to_number(const std::basic_string<CharT> &str)
	{
		return detail::to_number_helper_t<CharT>(str);
	}

	template < typename CharT >
	inline detail::to_number_helper_t<CharT> to_number(const CharT *str)
	{
		std::basic_string<CharT> val(str);
		return detail::to_number_helper_t<CharT>(val);
	}


	// to string

	namespace detail
	{
		template < typename T >
		struct to_string_helper_t
		{
			T val_;
			int prec_;
			bool is_boolalpha_;

			to_string_helper_t(const T &val, int prec = 0, bool is_boolalpha = false)
				: val_(val)
				, prec_(prec)
				, is_boolalpha_(is_boolalpha)
			{
				if( prec_ != 0 )
					assert(is_boolalpha == false);
			}

			template < typename CharT >
			operator std::basic_string<CharT>() const
			{
				std::basic_ostringstream<CharT> os_;
				if( prec_ != 0 )
					os_ << std::setiosflags(std::ios::fixed) << std::setprecision(prec_);
				if( is_boolalpha_ )
					os_ << std::boolalpha;

				os_ << val_;
				return os_.str();
			}

			/*operator std::basic_string<wchar_t>() const
			{
				std::basic_ostringstream<wchar_t> os_;
				if( prec_ != 0 )
					os_ << std::setiosflags(std::ios::fixed) << std::setprecision(prec_);
				if( is_boolalpha_ )
					os_ << std::boolalpha;

				os_ << val_;
				return os_.str();
			}*/
		};
	}


	template < typename T >
	inline detail::to_string_helper_t<T> to_string(const T &val)
	{
		return detail::to_string_helper_t<T>(val);
	}

	template < typename T >
	inline detail::to_string_helper_t<T> to_string(const T &val, int prec)
	{
		return detail::to_string_helper_t<T>(val, prec);
	}

	inline detail::to_string_helper_t<bool> to_string(bool val)
	{
		return detail::to_string_helper_t<bool>(val, 0, true);
	}

	
	// 忽略大小写比较

	template < typename CharT >
	int compare_no_case(const CharT *lhs, const CharT *rhs)
	{
		return utility::select<CharT>(_stricmp, _wcsicmp)(lhs, rhs);
	}

	template < typename CharT >
	int compare_no_case(const std::basic_string<CharT> &lhs, const std::basic_string<CharT> &rhs)
	{
		return compare_no_case(lhs.c_str(), rhs.c_str());
			/*std::lexicographical_compare(lhs.begin(), lhs.end(),
			rhs.begin(), rhs.end(), 
			[](CharT c1, CharT c2){ return std::tolower(c1) < std::tolower(c2); });*/
	}

	// 拼音比较
	template < typename CharT >
	int compare_phonetic(const std::basic_string<CharT> &lhs, const std::basic_string<CharT> &rhs)
	{
		LCID cid = MAKELCID(MAKELANGID(LANG_CHINESE_SIMPLIFIED, SUBLANG_CHINESE_SIMPLIFIED), SORT_CHINESE_PRCP);

		int ret = utility::select<CharT>(::CompareStringA, ::CompareStringW)(cid, 0, lhs.c_str(), lhs.length(), rhs.c_str(), rhs.length());
		switch(ret)
		{
		case CSTR_LESS_THAN:
			return (-1);
		case CSTR_EQUAL:
			return 0;
		case CSTR_GREATER_THAN:
			return 1;
		default:
			assert(false);
			return (-2);
		}
	}


	// replace
	template < typename CharT >
	std::basic_string<CharT> &replace_all(std::basic_string<CharT> &str, const std::basic_string<CharT> &old_value, const std::basic_string<CharT> &new_value)     
	{     
		while(true)   
		{     
			std::basic_string<CharT>::size_type pos(0);     
			if( ( pos = str.find(old_value)) != std::basic_string<CharT>::npos )     
				str.replace(pos, old_value.length(), new_value);     
			else   
				break;     
		}     
		return str;     
	}     

	// trim left

	template< typename CharT >
	inline void trim_left(std::basic_string<CharT> &str)
	{
		str.erase(0, str.find_first_not_of(' '));
	}

	// trim left

	template< typename CharT >
	inline void trim_right(std::basic_string<CharT> &str)
	{
		str.erase(str.find_last_not_of(' ') + 1);
	}


	// trim left & right

	template< typename CharT >
	inline void trim(std::basic_string<CharT> &str)
	{
		str.erase(0, str.find_first_not_of(' '));
		str.erase(str.find_last_not_of(' ') + 1);
	}

	// erase

	template < typename CharT >
	inline void erase(std::basic_string<CharT> &str, const CharT &charactor)
	{
		str.erase(std::remove_if(str.begin(), str.end(), 
			std::bind2nd(std::equal_to<CharT>(), charactor)), str.end());
	}


	// is start with

	template < typename CharT >
	inline bool is_start_with(const std::basic_string<CharT> &str, const std::basic_string<CharT> &src)
	{
		return str.compare(0, src.size(), src) == 0;
	}


	// is end with

	template < typename CharT >
	inline bool is_end_with(const std::basic_string<CharT> &str, const std::basic_string<CharT> &src)
	{
		return str.compare(str.size() - src.size(), src.size(), src) == 0;
	}

	template < typename CharT >
	inline bool find_nocase(const std::basic_string<CharT> &str, const std::basic_string<CharT> &val)
	{
		auto iter = std::search(str.begin(), str.end(), val.begin(), val.end(), 
			[](CharT lhs, CharT rhs) { return ::tolower(lhs) == ::tolower(rhs); });
		return iter != str.end();
	}

	// split

	template < typename CharT >
    inline void split(std::vector<std::basic_string<CharT>> &seq, const std::basic_string<CharT> &str, CharT separator)
	{
		if( str.empty() )
			return;

		std::basic_stringstream<CharT> iss(str);
		for(std::basic_string<CharT> s; std::getline(iss, s, separator); )
		{
            std::basic_string<CharT> val;
			std::basic_stringstream<CharT> isss(s);

			isss >> val;
			seq.push_back(std::move(val));
		}

		return;
	}

	template < typename CharT >
	inline std::basic_string<CharT> split(const std::basic_string<CharT> &str, CharT separator, size_t index)
	{
		std::vector<std::basic_string<CharT>> seq;
		split(seq, str, separator);

		if( seq.empty() )
			return std::basic_string<CharT>();

		assert(index < seq.size());
		if( index >= seq.size() )
		{			
			return std::basic_string<CharT>();
		}
		
		return seq[index];
	}


}


#endif