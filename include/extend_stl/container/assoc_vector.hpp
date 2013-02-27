#ifndef __CONTAINER_ASSOCVECTOR_HPP
#define __CONTAINER_ASSOCVECTOR_HPP


#include <algorithm>
#include <functional>
#include <vector>
#include <utility>
#include <iterator>
#include <map>



/*
	

*/

namespace container
{



	// -------------------------------------------------
	// class template assoc_vector_compare_t

	namespace detail
	{
		template < typename V, typename C >
		class assoc_vector_compare_t 
			: public C
		{
			typedef std::pair<typename C::first_argument_type, V> Data;
			typedef typename C::first_argument_type first_argument_type;

		public:
			assoc_vector_compare_t()
			{}

			assoc_vector_compare_t(const C& src) 
				: C(src)
			{}

			bool operator()(const first_argument_type& lhs, const first_argument_type& rhs) const
			{ return C::operator()(lhs, rhs); }

			bool operator()(const Data& lhs, const Data& rhs) const
			{ return operator()(lhs.first, rhs.first); }

			bool operator()(const Data& lhs, const first_argument_type& rhs) const
			{ return operator()(lhs.first, rhs); }

			bool operator()(const first_argument_type& lhs, const Data& rhs) const
			{ return operator()(lhs, rhs.first); }
		};
	}

	// ------------------------------------------------------------------
	// class template AssocVector
	// 使用关联性的vector替代std::map
	// 当心: AssocVector并没有完全替代map,如下
	// 
	// * iterators are invalidated by insert and erase operations
	// * the complexity of insert/erase is O(N) not O(log N)
	// * value_type is std::pair<K, V> not std::pair<const K, V>
	// * iterators are random


	template<
		class K,
		class V,
		class C = std::less<K>,
		class A = std::allocator< std::pair<K, V> >
	>
	class assoc_vector_t
		: private std::vector< std::pair<K, V>, A >
		, private detail::assoc_vector_compare_t<V, C>
	{
		typedef std::vector<std::pair<K, V>, A>		Base;
		typedef detail::assoc_vector_compare_t<V, C>	MyCompare;

	public:
		typedef K								key_type;
		typedef V								mapped_type;
		typedef typename Base::value_type		value_type;
		typedef C								key_compare;
		typedef A								allocator_type;
		typedef typename A::reference			reference;
		typedef typename A::const_reference		const_reference;
		typedef typename Base::iterator			iterator;
		typedef typename Base::const_iterator	const_iterator;
		typedef typename Base::size_type		size_type;
		typedef typename Base::difference_type	difference_type;
		typedef typename A::pointer				pointer;
		typedef typename A::const_pointer		const_pointer;
		typedef typename Base::reverse_iterator reverse_iterator;
		typedef typename Base::const_reverse_iterator const_reverse_iterator;

		class value_compare
			: public std::binary_function<value_type, value_type, bool>
			, private key_compare
		{
			friend class assoc_vector_t;

		protected:
			value_compare(key_compare pred) : key_compare(pred)
			{}

		public:
			bool operator()(const value_type& lhs, const value_type& rhs) const
			{ return key_compare::operator()(lhs.first, rhs.first); }
		};



		explicit assoc_vector_t(const key_compare& comp = key_compare(), const A& alloc = A())
			: Base(alloc)
			, MyCompare(comp)
		{}

		template< typename InputIterator >
		assoc_vector_t(InputIterator first, InputIterator last, const key_compare& comp = key_compare(), const A& alloc = A())
			: Base( alloc )
			, MyCompare( comp )
		{
			typedef std::map< K, V, C, A > TempMap;

			MyCompare & me = *this;
			const A tempAlloc;

			// 使用临时temp来插入元素
			TempMap temp(first, last, me, tempAlloc);
			Base::reserve(temp.size());
			Base & target = static_cast< Base & >(*this);
			std::copy(temp.begin(), temp.end(), std::back_inserter(target));
		}

		assoc_vector_t& operator=(const assoc_vector_t& rhs)
		{
			assoc_vector_t(rhs).swap(*this);
			return *this;
		}

		// iterators:
		iterator begin()				{ return Base::begin(); }
		const_iterator begin() const	{ return Base::begin(); }
		iterator end()					{ return Base::end(); }
		const_iterator end() const		{ return Base::end(); }
		reverse_iterator rbegin()		{ return Base::rbegin(); }
		const_reverse_iterator rbegin() const { return Base::rbegin(); }
		reverse_iterator rend()			{ return Base::rend(); }
		const_reverse_iterator rend() const { return Base::rend(); }

		// capacity:
		bool empty() const		{ return Base::empty(); }
		size_type size() const	{ return Base::size(); }
		size_type max_size()	{ return Base::max_size(); }

		// element access:
		mapped_type& operator[](const key_type& key)
		{ return insert(value_type(key, mapped_type())).first->second; }

		// modifiers:
		std::pair<iterator, bool> insert(const value_type& val)
		{
			bool found(true);
			iterator i(lower_bound(val.first));

			if( i == end() || this->operator()(val.first, i->first))
			{
				i = Base::insert(i, val);
				found = false;
			}
			return std::make_pair(i, !found);
		}

		iterator insert(iterator pos, const value_type& val)
		{
			if( (pos == begin() || this->operator()(*(pos-1),val)) &&
				(pos == end()   || this->operator()(val, *pos)) )
			{
				return Base::insert(pos, val);
			}
			return insert(val).first;
		}

		template< typename InputIterator >
		void insert(InputIterator first, InputIterator last)
		{ 
			for(; first != last; ++first) 
				insert(*first); 
		}

		void erase(iterator pos)
		{ Base::erase(pos); }

		size_type erase(const key_type& k)
		{
			iterator i(find(k));
			if( i == end() ) 
				return 0;

			erase(i);
			return 1;
		}

		void erase(iterator first, iterator last)
		{ Base::erase(first, last); }

		void swap(assoc_vector_t& other)
		{
			Base::swap(other);
			MyCompare& me = *this;
			MyCompare& rhs = other;
			std::swap(me, rhs);
		}

		void clear()
		{ Base::clear(); }

		// observers:
		key_compare key_comp() const
		{ return *this; }

		value_compare value_comp() const
		{
			const key_compare& comp = *this;
			return value_compare(comp);
		}

		// map operations:
		iterator find(const key_type& k)
		{
			iterator i(lower_bound(k));
			if( i != end() && this->operator()(k, i->first) )
			{
				i = end();
			}
			return i;
		}

		const_iterator find(const key_type& k) const
		{
			const_iterator i(lower_bound(k));
			if( i != end() && this->operator()(k, i->first) )
			{
				i = end();
			}
			return i;
		}

		size_type count(const key_type& k) const
		{ return find(k) != end(); }

		iterator lower_bound(const key_type& k)
		{
			MyCompare& me = *this;
			return std::lower_bound(begin(), end(), k, me);
		}

		const_iterator lower_bound(const key_type& k) const
		{
			const MyCompare& me = *this;
			return std::lower_bound(begin(), end(), k, me);
		}

		iterator upper_bound(const key_type& k)
		{
			MyCompare& me = *this;
			return std::upper_bound(begin(), end(), k, me);
		}

		const_iterator upper_bound(const key_type& k) const
		{
			const MyCompare& me = *this;
			return std::upper_bound(begin(), end(), k, me);
		}

		std::pair<iterator, iterator> equal_range(const key_type& k)
		{
			MyCompare& me = *this;
			return std::equal_range(begin(), end(), k, me);
		}

		std::pair<const_iterator, const_iterator> equal_range(const key_type& k) const
		{
			const MyCompare& me = *this;
			return std::equal_range(begin(), end(), k, me);
		}

		template <class K1, class V1, class C1, class A1>
		friend bool operator==(const assoc_vector_t<K1, V1, C1, A1>& lhs, const assoc_vector_t<K1, V1, C1, A1>& rhs);

		bool operator<(const assoc_vector_t& rhs) const
		{
			const Base& me = *this;
			const Base& yo = rhs;
			return me < yo;
		}

		template< typename K1, typename V1, typename C1, typename A1 >
		friend bool operator!=(const assoc_vector_t<K1, V1, C1, A1>& lhs,
			const assoc_vector_t<K1, V1, C1, A1>& rhs);

		template< typename K1, typename V1, typename C1, typename A1 >
		friend bool operator>(const assoc_vector_t<K1, V1, C1, A1>& lhs,
			const assoc_vector_t<K1, V1, C1, A1>& rhs);

		template< typename K1, typename V1, typename C1, typename A1 >
		friend bool operator>=(const assoc_vector_t<K1, V1, C1, A1>& lhs,
			const assoc_vector_t<K1, V1, C1, A1>& rhs);

		template< typename K1, typename V1, typename C1, typename A1 >
		friend bool operator<=(const assoc_vector_t<K1, V1, C1, A1>& lhs,
			const assoc_vector_t<K1, V1, C1, A1>& rhs);
	};

	template< typename K, typename V, typename C, typename A >
	inline bool operator==(const assoc_vector_t<K, V, C, A>& lhs, const assoc_vector_t<K, V, C, A>& rhs)
	{
		const std::vector<std::pair<K, V>, A>& me = lhs;
		return me == rhs;
	}

	template< typename K, typename V, typename C, typename A >
	inline bool operator!=(const assoc_vector_t<K, V, C, A>& lhs, const assoc_vector_t<K, V, C, A>& rhs)
	{ return !(lhs == rhs); }

	template< typename K, typename V, typename C, typename A >
	inline bool operator>(const assoc_vector_t<K, V, C, A>& lhs, const assoc_vector_t<K, V, C, A>& rhs)
	{ return rhs < lhs; }

	template< typename K, typename V, typename C, typename A >
	inline bool operator>=(const assoc_vector_t<K, V, C, A>& lhs, const assoc_vector_t<K, V, C, A>& rhs)
	{ return !(lhs < rhs); }

	template< typename K, typename V, typename C, typename A >
	inline bool operator<=(const assoc_vector_t<K, V, C, A>& lhs, const assoc_vector_t<K, V, C, A>& rhs)
	{ return !(rhs < lhs); }


	// specialized algorithms:
	template< typename K, typename V, typename C, typename A >
	void swap(assoc_vector_t<K, V, C, A>& lhs, assoc_vector_t<K, V, C, A>& rhs)
	{ lhs.swap(rhs); }
}


#endif 

