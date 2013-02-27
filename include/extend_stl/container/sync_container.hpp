#ifndef __CONTAINER_SYNC_CONTAINER_HPP
#define __CONTAINER_SYNC_CONTAINER_HPP

#include <vector>
#include <map>
#include "../../multi_thread/lock.hpp"


/*
线程安全线性容器(vector, list, deque)
sync_sequence_container_t

线程安全关联容器(map, set, multimap, multiset, hash)
sync_assoc_container_t

*/

namespace std
{
	template< typename T >
	class hash< std::shared_ptr<T> >
		: public unary_function<std::shared_ptr<T>, size_t>
	{	// hash functor
	public:
		typedef std::shared_ptr<T> _Kty;
		typedef _Uint32t _Inttype;	// use first (1 or 2)*32 bits

		size_t operator()(_Kty _Keyval) const
		{	// hash _Keyval to size_t value by pseudorandomizing transform
			typedef typename _STD _If<sizeof (T *) <= sizeof (_Inttype),
				_Inttype, _ULonglong>::_Type _Integer;
			return (hash<_Integer>()((_Integer)_Keyval.get()));
		}
	};
}

namespace stdex
{


	namespace container
	{
		// -----------------------------------------------------

		template < 
			typename T,
			typename C = std::vector<T>, 
			typename S = multi_thread::critical_section
		>
		class sync_sequence_container_t
		{
		public:
			typedef S Mutex;
			typedef multi_thread::auto_lock_t<Mutex>	AutoLock;
			typedef C Container;

			typedef typename Container::value_type		value_type;
			typedef typename Container::iterator		iterator;
			typedef typename Container::const_iterator	const_iterator;
			typedef typename Container::reference		reference;
			typedef typename Container::const_reference	const_reference;
			typedef typename Container::allocator_type	allocator_type;

		private:
			mutable Mutex mutex_;
			Container container_;

		public:
			sync_sequence_container_t()
			{}

			explicit sync_sequence_container_t(const allocator_type &alloc)
				: container_(alloc)
			{}

		private:
			sync_sequence_container_t(const sync_sequence_container_t &);
			sync_sequence_container_t &operator=(const sync_sequence_container_t &);

		public:
			iterator begin()
			{ 
				AutoLock lock(mutex_);
				return container_.begin();
			}
			const_iterator begin() const
			{ 
				AutoLock lock(mutex_);
				return container_.begin();
			}
			iterator end()
			{ 
				AutoLock lock(mutex_);
				return container_.end();
			}
			const_iterator end() const
			{ 
				AutoLock lock(mutex_);
				return container_.end();
			}

			size_t size() const
			{
				AutoLock lock(mutex_);
				return container_.size();
			}

			bool empty() const
			{
				AutoLock lock(mutex_);
				return container_.empty();
			}

			void clear()
			{
				AutoLock lock(mutex_);
				container_.clear();
			}

			reference operator[](size_t pos)
			{
				AutoLock lock(mutex_);
				iterator iter = container_.begin();
				std::advance(iter, pos);

				return *iter;
			}

			const_reference top() const
			{
				AutoLock lock(mutex_);
				return container_.front();
			}

			void pop_top()
			{
				AutoLock lock(mutex_);
				container_.pop_front();
			}

			void push_front(const T &val)
			{
				AutoLock lock(mutex_);
				return container_.push_front(val);
			}
			void push_back(const T &val)
			{
				AutoLock lock(mutex_);
				return container_.push_back(val);
			}

			template < typename OP >
			void for_each(const OP &op)
			{
				AutoLock lock(mutex_);
				std::for_each(container_.begin(), container_.end(), op);
			}

			template < typename Functor, typename OP >
			iterator op_if(const Functor &func, const OP &op)
			{
				AutoLock lock(mutex_);
				iterator iter = find_if(func);
				if( iter != container_.end() )
					op(*iter);

				return iter;
			}

			template < typename Functor, typename OP1, typename OP2 >
			void op_if(const Functor &func, OP1 op1, OP2 op2)
			{
				AutoLock lock(mutex_);
				iterator iter = find_if(func);
				if( iter != container_.end() )
					op1(*iter);
				else
					op2();
			}

			template < typename OP >
			iterator find_if(const OP &op)
			{
				AutoLock lock(mutex_);
				return std::find_if(container_.begin(), container_.end(), op);
			}

			template < typename OP >
			const_iterator find_if(const OP &op) const
			{
				AutoLock lock(mutex_);
				return std::find_if(container_.begin(), container_.end(), op);
			}

			template < typename OP >
			void erase(const OP &op)
			{
				AutoLock lock(mutex_);
				const_iterator iter = find_if(op);
				if( iter != container_.end() )
					container_.erase(iter);
			}

			void sort()
			{
				AutoLock lock(mutex_);
				std::sort(container_.begin(), container_.end());
			}

			template < typename OP >
			void sort(const OP &op)
			{
				AutoLock lock(mutex_);
				std::sort(container_.begin(), container_.end(), op);
			}

		};



		// ---------------------------------------------------

		template < 
			typename K, 
			typename V,
			typename C = std::map<K, V>, 
			typename S = multi_thread::critical_section
		>
		class sync_assoc_container_t
		{
		public:
			typedef S Mutex;
			typedef multi_thread::auto_lock_t<Mutex>	AutoLock;
			typedef C Container;

			typedef typename Container::key_type		key_type;
			typedef typename Container::mapped_type		mapped_type;
			typedef typename Container::value_type		value_type;
			typedef typename Container::iterator		iterator;
			typedef typename Container::const_iterator	const_iterator;
			typedef typename Container::reference		reference;
			typedef typename Container::const_reference	const_reference;
			typedef typename Container::allocator_type	allocator_type;

		private:
			mutable Mutex mutex_;
			Container container_;

		public:
			sync_assoc_container_t()
			{}

			explicit sync_assoc_container_t(const allocator_type &alloc)
				: container_(alloc)
			{}

		private:
			sync_assoc_container_t(const sync_assoc_container_t &);
			sync_assoc_container_t &operator=(const sync_assoc_container_t &);

		public:
			iterator begin()
			{ 
				AutoLock lock(mutex_);
				return container_.begin();
			}
			const_iterator begin() const
			{ 
				AutoLock lock(mutex_);
				return container_.begin();
			}
			iterator end()
			{ 
				AutoLock lock(mutex_);
				return container_.end();
			}
			const_iterator end() const
			{ 
				AutoLock lock(mutex_);
				return container_.end();
			}

			Container &get_container()
			{
				AutoLock lock(mutex_);
				return container_;
			}

			const Container &get_container() const
			{
				AutoLock lock(mutex_);
				return container_;
			}

			size_t size() const
			{
				AutoLock lock(mutex_);
				return container_.size();
			}

			bool empty() const
			{
				AutoLock lock(mutex_);
				return container_.empty();
			}

			void clear()
			{
				AutoLock lock(mutex_);
				container_.clear();
			}

			mapped_type &operator[](const key_type &key)
			{
				AutoLock lock(mutex_);
				return container_[key];
			}

			const mapped_type &operator[](const key_type &key) const
			{
				AutoLock lock(mutex_);
				return container_.find(key)->second;
			}

			bool exsit(const key_type &key) const
			{
				AutoLock lock(mutex_);
				return container_.find(key) != container_.end();
			}

			void insert(const key_type &key, const mapped_type &val)
			{
				AutoLock lock(mutex_);
				container_.insert(std::make_pair(key, val));
			}

			template < typename OP >
			void for_each(const OP &op) const
			{
				AutoLock lock(mutex_);
				std::for_each(container_.begin(), container_.end(), op);
			}

			template < typename OP >
			void for_each(OP op)
			{
				AutoLock lock(mutex_);
				std::for_each(container_.begin(), container_.end(), op);
			}

			template < typename OP >
			iterator find_if(const OP &op)
			{
				AutoLock lock(mutex_);
				return std::find_if(container_.begin(), container_.end(), op);
			}

			template < typename OP >
			const_iterator find_if(const OP &op) const
			{
				AutoLock lock(mutex_);
				return std::find_if(container_.begin(), container_.end(), op);
			}

			template < typename OP >
			bool not_if_op(const key_type &key, OP &&op)
			{
				AutoLock lock(mutex_);
				const_iterator iter = container_.find(key);
				if( iter == container_.end() )
				{
					op();
					return true;
				}

				return false;
			}

			template < typename OP >
			bool op_if(const key_type &key, OP &&op)
			{
				AutoLock lock(mutex_);
				iterator iter = container_.find(key);
				if( iter != container_.end() )
				{
					op(*iter);
					return true;
				}

				return false;
			}

			void erase(const key_type &key)
			{
				AutoLock lock(mutex_);

				container_.erase(key);
			}

			void erase(const const_iterator &iter)
			{
				AutoLock lock(mutex_);

				container_.erase(iter);
			}
		};

	}

}



#endif