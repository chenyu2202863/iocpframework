#ifndef __UTILITY_OBJECT_POOL_HPP
#define __UTILITY_OBJECT_POOL_HPP

#include <memory>
#include <list>
#include <functional>
#include <cstdint>

namespace utility { 

	template < typename T >
	struct object_pool_traits_t;

	template < typename T, typename AllocatorT >
	struct object_pool_traits_t<std::list<std::shared_ptr<T>, AllocatorT>>
	{
		typedef std::shared_ptr<T> value_t;

		static value_t pop(std::list<value_t, AllocatorT> &l)
		{
			if( l.empty() )
			{
				return value_t();
			}
			else
			{
				value_t val = l.front();
				l.pop_front();
				return val;
			}
		}

		static void push(std::list<value_t, AllocatorT> &l, value_t && val)
		{
			l.push_back(std::move(val));
		}

		template < typename HandlerT >
		static void for_each(std::list<value_t, AllocatorT> &l, const HandlerT &handler)
		{
			std::for_each(l.cbegin(), l.cend(), handler);
		}

		static std::uint32_t size()
		{
			return l.size();
		}
	};


	template < 
		typename T,
		typename C = std::list<std::shared_ptr<T>>
	> 
	struct object_pool_t
	{
		typedef std::shared_ptr<T> value_t;
		typedef C queue_t;
		typedef std::shared_ptr<queue_t> pool_t;
		
		typedef std::function<value_t()> create_handler_t;
		static_assert(std::is_same<value_t, typename queue_t::value_type>::value, "container value_type must be std::shared_ptr<T> type");

		create_handler_t create_handler_;
		pool_t pool_;

		object_pool_t(const create_handler_t &create_handler)
			: pool_(std::make_shared<queue_t>())
			, create_handler_(create_handler)
		{
			assert(create_handler_ != nullptr);
		}

		template < typename AllocatorT = std::allocator<char> >
		value_t get(const AllocatorT &allocator = AllocatorT())
		{
			value_t obj = object_pool_traits_t<queue_t>::pop(*pool_);
			if( !obj )
				obj = create_handler_();

			return std::shared_ptr<T>(obj.get(), 
									  [=](T *) mutable 
			{ 
				object_pool_traits_t<queue_t>::push(*pool_, std::move(obj));
			}, allocator);
		}

		value_t raw_aciquire()
		{
			value_t obj = object_pool_traits_t<queue_t>::pop(*pool_);
			if( !obj )
				obj = create_handler_();

			return obj;
		}

		void raw_release(value_t &&val)
		{
			object_pool_traits_t<queue_t>::push(*pool_, std::move(val));
		}

		void for_each(const std::function<void(const value_t &)> &handler)
		{
			object_pool_traits_t<queue_t>::for_each(*pool_, handler);
		}

		std::uint32_t size() const
		{
			return object_pool_traits_t<queue_t>::size(*pool_);
		}
	};
}

#endif