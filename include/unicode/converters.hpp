#ifndef __UNICODE_CONVERTERS_HPP
#define __UNICODE_CONVERTERS_HPP

#include "string.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace unicode
{

//////////////////////////////////////////////////////////////////////////////

template<typename T, typename Enable = void>
struct converter;

template<typename T, typename U>
struct converter_base
{
	typedef U base_type;
	static T to(U const& u) { return static_cast<T>(u); }
	static U from(T const& t) { return static_cast<U>(t); }
};

template<> struct converter<bool> : converter_base<bool, int> {};
template<> struct converter<char> : converter_base<char, int> {};
template<> struct converter<signed char> : converter_base<signed char, int> {};
template<> struct converter<unsigned char> : converter_base<unsigned char, int> {};
template<> struct converter<wchar_t> : converter_base<wchar_t, int> {};
template<> struct converter<short> : converter_base<short, int> {};
template<> struct converter<unsigned short> : converter_base<unsigned short, int> {};
template<> struct converter<int> : converter_base<int, int> {};
template<> struct converter<unsigned int> : converter_base<unsigned int, int> {};
template<> struct converter<long> : converter_base<long, int> {};
template<> struct converter<unsigned long> : converter_base<unsigned long, int> {};
template<> struct converter<long long> : converter_base<long long, long long> {};
template<> struct converter<unsigned long long> : converter_base<unsigned long long, long long> {};
template<> struct converter<float> : converter_base<float, double> {};
template<> struct converter<double> : converter_base<double, double> {};


template<>
struct converter<string_t>
{
	typedef string_t base_type;
	static string_t const& to(string_t const& b)
	{
		return b;
	}
	static string_t const& from(string_t const& t)
	{
		return t;
	}
};


//////////////////////////////////////////////////////////////////////////////

} //namespace 

//////////////////////////////////////////////////////////////////////////////

#endif // 

//////////////////////////////////////////////////////////////////////////////
