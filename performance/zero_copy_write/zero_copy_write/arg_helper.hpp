#ifndef __BAIMO_NETWORK_ARG_SIZE_HPP
#define __BAIMO_NETWORK_ARG_SIZE_HPP

#include <cstdint>

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <type_traits>
#include <numeric>

namespace baimo { namespace network {


	template < typename T >
	std::uint32_t arg_size(const T &,
						   typename std::enable_if<std::is_pod<T>::value>::type * = nullptr)
	{
		static_assert(std::is_pod<T>::value, "T must be a pod type");
		return sizeof(T);
	}

	template < typename FirstT, typename SecondT >
	std::uint32_t arg_size(const std::pair<FirstT, SecondT> &val)
	{
		return arg_size(val.first) + arg_size(val.second);
	}

	template < typename CharT >
	std::uint32_t arg_size(const std::basic_string<CharT> &msg)
	{
		return sizeof(CharT) * msg.size();
	}

	template < typename T >
	std::uint32_t arg_size(const std::shared_ptr<T> &msg)
	{
		return arg_size(*msg);
	}

	template < typename T, typename AllocatorT >
	std::uint32_t arg_size(const std::vector<T, AllocatorT> &t,
						   typename std::enable_if<std::is_pod<T>::value>::type * = nullptr)
	{
		return arg_size(*(T *) 0) * t.size();
	}

	template < typename T, typename AllocatorT >
	std::uint32_t arg_size(const std::vector<std::pair<std::uint32_t, T>, AllocatorT> &t,
						   typename std::enable_if<!std::is_pod<T>::value>::type * = nullptr)
	{
		return std::accumulate(t.cbegin(), t.cend(), 0,
							   [](const std::uint32_t &sum, const std::pair<std::uint32_t, T> &val)
		{
			return sum + arg_size(val.first) + val.first;
		});
	}

	template < typename KeyT, typename ValueT, typename LessT, typename AllocatorT >
	std::uint32_t arg_size(const std::map<KeyT, std::pair<std::uint32_t, ValueT>, LessT, AllocatorT> &t)
	{
		return std::accumulate(t.cbegin(), t.cend(), 0,
							   [](const std::uint32_t &sum, const std::pair<KeyT, std::pair<std::uint32_t, ValueT>> &val)
		{
			return sum + arg_size(val.first) + arg_size(val.second.first) + val.second.first;
		});
	}


	template < typename T >
	T make_arg(T && t,
			   typename std::enable_if<std::is_pod<T>::value>::type * = nullptr)
	{
		return t;
	}

	template < typename CharT >
	std::pair<std::uint32_t, std::basic_string<CharT>> make_arg(std::basic_string<CharT> && val)
	{
		return { val.size(), std::move(val) };
	}

	template < typename T, typename AllocatorT >
	std::pair<std::uint32_t, std::vector<T, AllocatorT >> make_arg(std::vector<T, AllocatorT> && val,
																   typename std::enable_if<std::is_pod<T>::value>::type * = nullptr)
	{
		return { val.size(), std::move(val) };
	}

	template < typename T, typename AllocatorT >
	std::pair<std::uint32_t, std::vector<std::pair<std::uint32_t, T>, AllocatorT>> make_arg(std::vector<std::pair<std::uint32_t, T>, AllocatorT> && val,
																							typename std::enable_if<!std::is_pod<T>::value>::type * = nullptr)
	{
		return { val.size(), std::move(val) };
	}

	template < typename KeyT, typename ValueT, typename LessT, typename AllocatorT >
	std::pair<std::uint32_t, std::map<KeyT, ValueT, LessT, AllocatorT>> make_arg(std::map<KeyT, ValueT, LessT, AllocatorT> && val)
	{
		return { val.size(), std::move(val) };
	}



	template < typename T, typename ... Args >
	void paramter_size(std::uint32_t &size, const std::pair<std::uint32_t, T> &t, const Args &...args)
	{
		size += arg_size(t.first) + arg_size(t.second);
		paramter_size(size, args...);
	}

	template < typename T >
	void paramter_size(std::uint32_t &size, const std::pair<std::uint32_t, T> &t)
	{
		size += arg_size(t.first) + arg_size(t.second);
	}

	template < typename T, typename ... Args >
	typename typename std::enable_if<std::is_pod<T>::value, void>::type paramter_size(std::uint32_t &size, const T &t, const Args &...args)
	{
		size += arg_size(t);
		paramter_size(size, args...);
	}

	template < typename T >
	typename typename std::enable_if<std::is_pod<T>::value, void>::type paramter_size(std::uint32_t &size, const T &t)
	{
		size += arg_size(t);
	}
}
}

#endif