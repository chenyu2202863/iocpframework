#ifndef __SERIALIZE_CONTAINER_DISPATCH_HPP
#define __SERIALIZE_CONTAINER_DISPATCH_HPP


#include <tuple>
#include <string>
#include <map>
#include <set>
#include <list>
#include <vector>
#include <stack>
#include <queue>
#include <deque>
#include <unordered_map>
#include <unordered_set>

namespace serialize { namespace detail {

	template < typename ContainerT >
	struct is_container_t
	{
		static const bool value = false;
	};

	template < typename KeyT, typename ValueT >
	struct is_container_t< std::pair<KeyT, ValueT> >
	{
		static const bool value = true;
	};

	template < typename ...Args >
	struct is_container_t< std::tuple<Args...> >
	{
		static const bool value = true;
	};

	template < typename T, typename AllocatorT >
	struct is_container_t< std::vector<T, AllocatorT> >
	{
		static const bool value = true;
	};

	template < typename T, typename AllocatorT >
	struct is_container_t< std::list<T, AllocatorT> >
	{
		static const bool value = true;
	};

	template < typename T, typename AllocatorT >
	struct is_container_t< std::deque<T, AllocatorT> >
	{
		static const bool value = true;
	};

	template < typename KeyT, typename ValueT, typename LessT, typename AllocatorT >
	struct is_container_t< std::map<KeyT, ValueT, LessT, AllocatorT> >
	{
		static const bool value = true;
	};

	template < typename KeyT, typename LessT, typename AllocatorT >
	struct is_container_t< std::set<KeyT, LessT, AllocatorT> >
	{
		static const bool value = true;
	};

	template < typename KeyT, typename ValueT, typename LessT, typename AllocatorT >
	struct is_container_t< std::unordered_map<KeyT, ValueT, LessT, AllocatorT> >
	{
		static const bool value = true;
	};

	template < typename KeyT, typename LessT, typename AllocatorT >
	struct is_container_t< std::unordered_set<KeyT, LessT, AllocatorT> >
	{
		static const bool value = true;
	};

	template < typename KeyT, typename ValueT, typename LessT, typename AllocatorT >
	struct is_container_t< std::multimap<KeyT, ValueT, LessT, AllocatorT> >
	{
		static const bool value = true;
	};

	template < typename KeyT, typename LessT, typename AllocatorT >
	struct is_container_t< std::multiset<KeyT, LessT, AllocatorT> >
	{
		static const bool value = true;
	};

	template < typename KeyT, typename ValueT, typename LessT, typename AllocatorT >
	struct is_container_t< std::unordered_multimap<KeyT, ValueT, LessT, AllocatorT> >
	{
		static const bool value = true;
	};

	template < typename KeyT, typename LessT, typename AllocatorT >
	struct is_container_t< std::unordered_multiset<KeyT, LessT, AllocatorT> >
	{
		static const bool value = true;
	};

	// -----------------------------------------------------------------
	// class ValueTraits

	template< typename ValTraitsT >
	class container_traits_t
	{
	public:
		typedef ValTraitsT								value_type;

	public:
		template < typename StreamT >
		static void push(StreamT &io, const value_type& val)
		{
			io << val;
		}

		template < typename StreamT >
		static void pop(StreamT &io, value_type &val)
		{
			io >> val;
		}
	};


	// pair
	template< typename FirstT, typename SecondT >
	class container_traits_t<std::pair<FirstT, SecondT>>
	{
	public:
		typedef std::pair<FirstT, SecondT>	value_type;

	public:
		template < typename StreamT >
		static void push(StreamT &io, const value_type& val)
		{
			container_traits_t<value_type::first_type>::push(io, val.first);
			container_traits_t<value_type::second_type>::push(io, val.second);
		}

		template < typename StreamT >
		static void pop(StreamT &io, value_type &val)
		{
			container_traits_t<value_type::first_type>::pop(io, val.first);
			container_traits_t<value_type::second_type>::pop(io, val.second);
		}
	};


	// tuple
	template< typename ...Args >
	class container_traits_t<std::tuple<Args...>>
	{
		struct impl_t
		{
			template < std::uint32_t I, std::uint32_t N, typename StreamT, typename TupleT >
			static typename std::enable_if<I == N>::type push(StreamT &os, const TupleT& t)
			{
				typedef std::tuple_element<I, TupleT>::type type;
				container_traits_t<type>::push(os, std::get<I>( t ));
			}

			template< std::uint32_t I, std::uint32_t N, typename StreamT, typename TupleT >
			static typename std::enable_if < I < N>::type push(StreamT &os, const TupleT& t)
			{
				typedef std::tuple_element<I, TupleT>::type type;
				container_traits_t<type>::push(os, std::get<I>( t ));

				push<I + 1, std::tuple_size<TupleT>::value - 1>( os, t );
			}

			template < std::uint32_t I, std::uint32_t N, typename StreamT, typename TupleT >
			static typename std::enable_if<I == N>::type pop(StreamT &os, TupleT& t)
			{
				typedef std::tuple_element<I, TupleT>::type type;
				container_traits_t<type>::pop(os, std::get<I>( t ));
			}

			template< std::uint32_t I, std::uint32_t N, typename StreamT, typename TupleT >
			static typename std::enable_if < I < N>::type pop(StreamT &os, TupleT& t)
			{
				typedef std::tuple_element<I, TupleT>::type type;
				container_traits_t<type>::pop(os, std::get<I>( t ));

				pop<I + 1, std::tuple_size<TupleT>::value - 1>( os, t );
			}
		};


	public:
		typedef std::tuple<Args...>	value_type;

	public:
		template < typename StreamT >
		static void push(StreamT &io, const value_type& val)
		{
			enum
			{
				TUPLE_SIZE = std::tuple_size<value_type>::value
			};
			impl_t::push<0, TUPLE_SIZE == 1 ? 0 : TUPLE_SIZE>( io, val );
		}

		template < typename StreamT >
		static void pop(StreamT &io, value_type &val)
		{
			enum
			{
				TUPLE_SIZE = std::tuple_size<value_type>::value
			};
			impl_t::pop<0, TUPLE_SIZE == 1 ? 0 : TUPLE_SIZE>( io, val );
		}
	};

	// 关联容器 -- map, multimap, set, multiset

	// -----------------------------------------------------------------
	// class associative_type_traits_t

	template< typename AssocTypeT >
	class associative_type_traits_t
	{
	public:
		typedef typename AssocTypeT::size_type		size_type;
		typedef typename AssocTypeT::key_type		key_type;
		typedef typename AssocTypeT::mapped_type	mapped_type;
		typedef AssocTypeT							value_type;

	public:
		template < typename StreamT >
		static void push(StreamT &io, const value_type& val)
		{
			push_length_impl(io, val.size());

			for(typename value_type::const_iterator iter = val.begin();
				iter != val.end(); ++iter)
			{
				container_traits_t<key_type>::push(io, iter->first);
				container_traits_t<mapped_type>::push(io, iter->second);
			}
		}

		template < typename StreamT >
		static void pop(StreamT &io, value_type &val)
		{
			size_type valSize = 0;
			container_traits_t<size_type>::pop(io, valSize);

			for(; valSize > 0; --valSize)
			{
				key_type key;
				container_traits_t<key_type>::pop(io, key);

				mapped_type value;
				container_traits_t<mapped_type>::pop(io, value);

				val.insert(std::move(std::make_pair(key, value)));
			}
		}
	};


	// 线性容器--list, vector, deque

	// -----------------------------------------------------------------
	// class sequence_traits_t

	template< typename SequenceTypeT, typename EnableT = void >
	class sequence_traits_t
	{
		template < typename T >
		struct reserve_traits_t;

		template < typename T, typename AllocatorT >
		struct reserve_traits_t<std::vector<T, AllocatorT>>
		{
			static void reserve(std::vector<T, AllocatorT> &t, std::uint32_t n)
			{
				t.reserve(n);
			}
		};

		template < typename T, typename AllocatorT >
		struct reserve_traits_t<std::list<T, AllocatorT>>
		{
			static void reserve(std::list<T, AllocatorT> &t, std::uint32_t n)
			{
			}
		};

		template < typename T, typename AllocatorT >
		struct reserve_traits_t<std::deque<T, AllocatorT>>
		{
			static void reserve(std::deque<T, AllocatorT> &t, std::uint32_t n)
			{
			}
		};


	public:
		typedef typename SequenceTypeT::size_type	size_type;
		typedef typename SequenceTypeT::value_type	value_type;

	public:
		template < typename StreamT >
		static void push(StreamT &io, const SequenceTypeT &val)
		{
			push_length_impl(io, val.size());

			for(SequenceTypeT::const_iterator iter = val.begin();
				iter != val.end(); ++iter)
			{
				container_traits_t<value_type>::push(io, *iter);
			}
		}

		template < typename StreamT >
		static void pop(StreamT &io, SequenceTypeT& val)
		{
			size_type val_size = 0;
			container_traits_t<size_type>::pop(io, val_size);

			reserve_traits_t<SequenceTypeT>::reserve(val, val_size);

			for(; val_size > 0; --val_size)
			{
				value_type element;
				container_traits_t<value_type>::pop(io, element);

				val.emplace_back(std::move(element));
			}
		}
	};

	template< typename T, typename AllocatorT >
	class sequence_traits_t<std::vector<T, AllocatorT>, typename std::enable_if<std::is_pod<T>::value>::type>
	{
	public:
		typedef std::vector<T, AllocatorT>		vector_t;
		typedef typename vector_t::size_type	size_type;
		typedef typename vector_t::value_type	value_type;

	public:
		template < typename StreamT >
		static void push(StreamT &io, const vector_t &val)
		{
			push_length_impl(io, val.size());

			io.push_pointer(val.data(), val.size());
		}

		template < typename StreamT >
		static void pop(StreamT &io, vector_t& val)
		{
			size_type val_size = 0;
			container_traits_t<size_type>::pop(io, val_size);

			val.resize(val_size);
			io.pop_pointer(val.data(), val.size());
		}
	};


	// STL map datatype serializer class.

	template< typename KeyT, typename DataT, typename LessT, typename AllocT >
	class container_traits_t<std::map<KeyT, DataT, LessT, AllocT>>
	{
	public:
		typedef std::map<KeyT, DataT, LessT, AllocT>	MapTraitType;

	public:
		template < typename StreamT >
		static void push(StreamT &io, const MapTraitType &val)
		{
			associative_type_traits_t<MapTraitType>::push(io, val);
		}

		template < typename StreamT >
		static void pop(StreamT &io, MapTraitType& val)
		{
			associative_type_traits_t<MapTraitType>::pop(io, val);
		}
	};


	// STL multimap datatype serializer class.

	template< typename KeyT, typename DataT, typename LessT, typename AllocT >
	class container_traits_t<std::multimap<KeyT, DataT, LessT, AllocT>>
	{
	public:
		typedef std::multimap<KeyT, DataT, LessT, AllocT> MultimapTraitType;


	public:
		template < typename StreamT >
		static void push(StreamT &io, const MultimapTraitType &val)
		{
			associative_type_traits_t<MultimapTraitType>::push(io, val);
		}

		template < typename StreamT >
		static void pop(StreamT &io, MultimapTraitType& val)
		{
			associative_type_traits_t<MultimapTraitType>::pop(io, val);
		}
	};



	// STL set datatype serializer class.

	template< typename KeyT, typename LessT, typename AllocT >
	class container_traits_t<std::set<KeyT, LessT, AllocT>>
	{
	public:
		typedef std::set<KeyT, LessT, AllocT>		SetTratiType;
		typedef typename SetTratiType::size_type	size_type;
		typedef typename SetTratiType::value_type	value_type;

	public:
		template < typename StreamT >
		static void push(StreamT &io, const SetTratiType& val)
		{
			push_length_impl(io, val.size());

			for(typename SetTratiType::const_iterator iter = val.begin();
				iter != val.end(); ++iter)
			{
				container_traits_t<value_type>::push(io, *iter);
			}
		}

		template < typename StreamT >
		static void pop(StreamT &io, SetTratiType &val)
		{
			size_type valSize = 0;
			container_traits_t<size_type>::pop(io, valSize);

			for(; valSize > 0; --valSize)
			{
				value_type key;
				container_traits_t<value_type>::pop(io, key);

				val.insert(key);
			}
		}
	};

	// STL multiset datatype serializer class.

	template<typename KeyT, typename LessT, typename AllocT >
	class container_traits_t<std::multiset<KeyT, LessT, AllocT>>
	{
	public:
		typedef std::multiset<KeyT, LessT, AllocT>		MultisetTraitType;
		typedef typename MultisetTraitType::size_type	size_type;
		typedef typename MultisetTraitType::value_type	value_type;

	public:
		template < typename StreamT >
		static void push(StreamT &io, const MultisetTraitType& val)
		{
			push_length_impl(io, val.size());

			for(typename MultisetTraitType::const_iterator iter = val.begin();
				iter != val.end(); ++iter)
			{
				container_traits_t<value_type>::push(io, *iter);
			}
		}

		template < typename StreamT >
		static void pop(StreamT &io, MultisetTraitType &val)
		{
			size_type valSize = 0;
			container_traits_t<size_type>::pop(io, valSize);

			for(; valSize > 0; --valSize)
			{
				value_type key;
				container_traits_t<value_type>::pop(io, key);

				val.insert(key);
			}
		}
	};


	// STL list datatype serializer class.

	template< typename KeyT, typename AllocT >
	class container_traits_t<std::list<KeyT, AllocT>>
	{
	public:
		typedef std::list<KeyT, AllocT> ListTraitType;

	public:
		template < typename StreamT >
		static void push(StreamT &io, const ListTraitType &val)
		{
			sequence_traits_t<ListTraitType>::push(io, val);
		}

		template < typename StreamT >
		static void pop(StreamT &io, ListTraitType &val)
		{
			sequence_traits_t<ListTraitType>::pop(io, val);
		}
	};

	//STL vector datatype serializer class.

	template< typename KeyT, typename AllocT >
	class container_traits_t<std::vector<KeyT, AllocT>>
	{
	public:
		typedef std::vector<KeyT, AllocT> VectorTraitType;


	public:
		template < typename StreamT >
		static void push(StreamT &io, const VectorTraitType &val)
		{
			sequence_traits_t<VectorTraitType>::push(io, val);
		}

		template < typename StreamT >
		static void pop(StreamT &io, VectorTraitType &val)
		{
			sequence_traits_t<VectorTraitType>::pop(io, val);
		}
	};


	// STL deque datatype serializer class.

	template< typename KeyT, typename AllocT >
	class container_traits_t<std::deque<KeyT, AllocT>>
	{
	public:
		typedef std::deque<KeyT, AllocT> DequeTraitType;


	public:
		template < typename StreamT >
		static void push(StreamT &io, const DequeTraitType &val)
		{
			sequence_traits_t<DequeTraitType>::push(io, val);
		}

		template < typename StreamT >
		static void pop(StreamT &io, DequeTraitType &val)
		{
			sequence_traits_t<DequeTraitType>::pop(io, val);
		}
	};

	template< typename ValueT, typename ContainerT >
	class container_traits_t<std::queue<ValueT, ContainerT>>
	{
	public:
		typedef std::queue<ValueT, ContainerT> QueueTraitType;

	public:
		template < typename StreamT >
		static void push(StreamT &io, const QueueTraitType &val)
		{
			static_assert( false, "no impl" );
		}

		template < typename StreamT >
		static void pop(StreamT &io, QueueTraitType &val)
		{
			static_assert( false, "no impl" );
		}
	};

	template< typename ValueT, typename ContainerT >
	class container_traits_t<std::stack<ValueT, ContainerT>>
	{
	public:
		typedef std::stack<ValueT, ContainerT> StackTraitType;

	public:
		template < typename StreamT >
		static void push(StreamT &io, const StackTraitType &val)
		{
			static_assert( false, "no impl" );
		}

		template < typename StreamT >
		static void pop(StreamT &io, StackTraitType &val)
		{
			static_assert( false, "no impl" );
		}
	};

	template< typename ValueT, typename ContainerT, typename LessT >
	class container_traits_t<std::priority_queue<ValueT, ContainerT, LessT>>
	{
	public:
		typedef std::priority_queue<ValueT, ContainerT, LessT> PriorityQueueTraitType;

	public:
		template < typename StreamT >
		static void push(StreamT &io, const PriorityQueueTraitType &val)
		{
			static_assert( false, "no impl" );
		}

		template < typename StreamT >
		static void pop(StreamT &io, PriorityQueueTraitType &val)
		{
			static_assert( false, "no impl" );
		}
	};
}
}







#endif