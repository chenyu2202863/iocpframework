#ifndef __SERIALIZE_DISPATCH_HPP
#define __SERIALIZE_DISPATCH_HPP

#include <type_traits>
#include <cstdint>
#include <memory>

#include "container_dispatch.hpp"


namespace serialize
{
	// forward declare
	template < typename CharT, typename OutT >
	class serialize_t;


	namespace detail
	{

		template < typename T >
		struct select_traits_t
		{
			template < typename CharT, typename OutT, typename U > 
			static void push(serialize_t<CharT, OutT> &os, const U &val, 
				typename std::enable_if<
				std::is_integral<U>::value || 
				std::is_floating_point<U>::value ||
				std::is_enum<U>::value
				>::type * = nullptr)
			{
				os.push(val);
			}

			template < typename CharT, typename OutT, typename U > 
			static void push(serialize_t<CharT, OutT> &os, const U &val, 
				typename std::enable_if<is_container_t<U>::value>::type * = nullptr)
			{
				container_traits_t<U>::push(os, val);
			}


			template < typename CharT, typename OutT, typename U > 
			static void pop(serialize_t<CharT, OutT> &os, U &val, 
				typename std::enable_if<
				std::is_integral<U>::value ||
				std::is_floating_point<U>::value ||
				std::is_enum<U>::value
				>::type * = nullptr)
			{
				os.pop(val);
			}


			template < typename CharT, typename OutT, typename U > 
			static void pop(serialize_t<CharT, OutT> &os, U &val, 
				typename std::enable_if<is_container_t<U>::value>::type * = nullptr)
			{
				container_traits_t<U>::pop(os, val);
			}
		};


		template < >
		struct select_traits_t< char * >
		{
			template < typename CharT, typename OutT > 
			static void push(serialize_t<CharT, OutT> &os, const char *val)
			{
				std::uint32_t len = std::char_traits<char>::length(val);
				os.push(len);
				os.push_pointer(val, len);
			}

			template < typename CharT, typename OutT > 
			static void pop(serialize_t<CharT, OutT> &os, char *val)
			{
				std::uint32_t bufLen = 0;
				os.pop(bufLen);
				os.pop_pointer(val, bufLen);
			}
		};

		template < >
		struct select_traits_t< wchar_t * >
		{
			template < typename CharT, typename OutT > 
			static void push(serialize_t<CharT, OutT> &os, const wchar_t *val)
			{
				std::uint32_t len = std::char_traits<wchar_t>::length(val);
				os.push(len);
				os.push_pointer(val, len);
			}

			template < typename CharT, typename OutT > 
			static void pop(serialize_t<CharT, OutT> &os, wchar_t *val)
			{
				std::uint32_t bufLen = 0;
				os.pop(bufLen);
				os.pop_pointer(val, bufLen);
			}
		};

		template < std::uint32_t N >
		struct select_traits_t< char[N] >
		{
			template < typename CharT, typename OutT > 
			static void push(serialize_t<CharT, OutT> &os, const char (&val)[N])
			{
				std::uint32_t len = std::char_traits<char>::length(val);
				os.push(len);
				os.push_pointer(val, len);
			}

			template < typename CharT, typename OutT > 
			static void pop(serialize_t<CharT, OutT> &os, char (&val)[N])
			{
				std::uint32_t bufLen = 0;
				os.pop(bufLen);
				os.pop_pointer(val, bufLen);
			}
		};

		template < std::uint32_t N >
		struct select_traits_t< wchar_t[N] >
		{
			template < typename CharT, typename OutT > 
			static void push(serialize_t<CharT, OutT> &os, const wchar_t (&val)[N])
			{
				std::uint32_t len = std::char_traits<wchar_t>::length(val);
				os.push(len);
				os.push_pointer(val, len);
			}

			template < typename CharT, typename OutT > 
			static void pop(serialize_t<CharT, OutT> &os, wchar_t (&val)[N])
			{
				std::uint32_t bufLen = 0;
				os.pop(bufLen);
				os.pop_pointer(val, bufLen);
			}
		};

		template < typename T >
		struct select_traits_t< T * >
		{
			template < typename CharT, typename OutT > 
			static void push(serialize_t<CharT, OutT> &os, const T *val)
			{
				os << *val;
			}

			template < typename CharT, typename OutT > 
			static void pop(serialize_t<CharT, OutT> &os, T *val)
			{
				os >> (*val);
			}
		};


		template < typename T, std::uint32_t N >
		struct select_traits_t< T[N] >
		{
			template < typename CharT, typename OutT, typename U > 
			static void push(serialize_t<CharT, OutT> &os, const U &val)
			{
				push(os, val, std::integral_constant<bool, std::is_pod<T>::value>());
			}

			template < typename CharT, typename OutT, typename U > 
			static void pop(serialize_t<CharT, OutT> &os, U &val)
			{
				pop(os, val, std::integral_constant<bool, std::is_pod<T>::value>());
			}


			template < typename CharT, typename OutT, typename U > 
			static void push(serialize_t<CharT, OutT> &os, const U &val, std::true_type)
			{
				os.push_array(val);
			}

			template < typename CharT, typename OutT, typename U > 
			static void push(serialize_t<CharT, OutT> &os, const U &val, std::false_type)
			{
				for(std::uint32_t i = 0; i != N; ++i)
					os << val[i];
			}

			template < typename CharT, typename OutT, typename U > 
			static void pop(serialize_t<CharT, OutT> &os, U &val, std::true_type)
			{
				os.pop_array(val);
			}

			template < typename CharT, typename OutT, typename U > 
			static void pop(serialize_t<CharT, OutT> &os, U &val, std::false_type)
			{
				for(std::uint32_t i = 0; i != N; ++i)
					os >> val[i];
			}
		};

		template < typename T, typename TraitsT, typename AllocatorT >
		struct select_traits_t< std::basic_string<T, TraitsT, AllocatorT> >
		{
			template < typename CharT, typename OutT > 
			static void push(serialize_t<CharT, OutT> &os, const std::basic_string<T, TraitsT, AllocatorT> &val)
			{
				os.push(val.size());
				os.push_pointer(val.c_str(), val.size());
			}

			template < typename CharT, typename OutT > 
			static void pop(serialize_t<CharT, OutT> &os, std::basic_string<T, TraitsT, AllocatorT> &val)
			{
				std::uint32_t bufLen = 0;
				os.pop(bufLen);

				val.resize(bufLen);
				os.pop_pointer(&val[0], bufLen);
			}
		};

		template < typename T >
		struct select_traits_t< std::shared_ptr<T> >
		{
			template < typename CharT, typename OutT > 
			static void push(serialize_t<CharT, OutT> &os, const std::shared_ptr<T> &val)
			{
				os << *val;
			}

			template < typename CharT, typename OutT > 
			static void pop(serialize_t<CharT, OutT> &os, std::shared_ptr<T> &val)
			{
				val.reset(new T);
				os >> *val;
			}
		};

	}
}



#endif