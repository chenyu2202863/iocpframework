#ifndef __CONTAINER_SYNC_CONTAINER_HPP
#define __CONTAINER_SYNC_CONTAINER_HPP

/** @async_container.hpp
*
* @author <陈煜>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/08>
* @version <0.1>
*
* 线程安全线性容器,提供线性容器和关联容器
*/


#include <vector>
#include <map>
#include <mutex>


/*
线程安全线性容器(vector, list, deque)
sync_sequence_container_t

线程安全关联容器(map, set, multimap, multiset, hash)
sync_assoc_container_t

*/


namespace stdex { namespace container {
		/**
		* @class <sync_sequence_container_t>
		* @brief 线性容器，线程安全，接口与stl容器类似
		*
		* T 值类型
		* C 容器类型，可以vector、list、deque，默认使用vector
		* S 同步类型，可以critical_section、event_t、mutex
		*/

		template < 
			typename T,
			typename C = std::vector<T>, 
			typename S = std::recursive_mutex
		>
		class sync_sequence_container_t
		{
		public:
			typedef S Mutex;
			typedef std::lock_guard<Mutex>	AutoLock;
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

			/**
			* @brief 传入一个allocator
			* @param <alloc> <allocator对象>
			* @exception <不会抛出任何异常>
			* @return <无>
			* @note <根据传入容器类型，定制容器内存分配器>
			* @remarks <提高内存分配效率>
			*/
			explicit sync_sequence_container_t(const allocator_type &alloc)
				: container_(alloc)
			{}

		private:
			sync_sequence_container_t(const sync_sequence_container_t &);
			sync_sequence_container_t &operator=(const sync_sequence_container_t &);

		public:
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

			void push_front(T &&val)
			{
				AutoLock lock(mutex_);
				return container_.push_front(std::move(val));
			}
			void push_back(T &&val)
			{
				AutoLock lock(mutex_);
				return container_.push_back(std::move(val));
			}

			/**
			* @brief 遍历整个容器
			* @param <op> <回调函数参数>
			* @exception <不会抛出任何异常>
			* @return <无>
			* @note <op类型为function<void(const T &)>,接受一个参数的回调函数>
			* @remarks <>
			*/
			template < typename OP >
			void for_each(OP &&op)
			{
				AutoLock lock(mutex_);
				std::for_each(container_.begin(), container_.end(), op);
			}

			/**
			* @brief 如果满足func条件，则执行op
			* @param <func> <条件回调函数，返回bool，接受一个const T &参数>
			* @param <op> <回调函数参数，接受一个const T &参数>
			* @exception <不会抛出任何异常>
			* @return <返回一个迭代器>
			* @note <如果通过functor检测，则执行op>
			* @remarks <使用find_if>
			*/
			template < typename Functor, typename OP >
			bool op_if(Functor &&func, OP &&op)
			{
				AutoLock lock(mutex_);
				iterator iter = find_if(func);
				if( iter != container_.end() )
				{
					op(std::ref(*iter));
					return true;
				}

				return false;
			}

			/**
			* @brief 如果满足func条件，执行op1，否则执行op2
			* @param <func> <条件回调函数，返回bool，接受一个const T &参数>
			* @param <op1> <回调函数参数，接受一个const T &参数>
			* @param <op2> <回调函数参数，接受一个const T &参数>
			* @exception <不会抛出任何异常>
			* @return <无>
			* @note <无>
			* @remarks <使用find_if>
			*/
			template < typename Functor, typename OP1, typename OP2 >
			void op_if(Functor &&func, OP1 &&op1, OP2 &&op2)
			{
				AutoLock lock(mutex_);
				iterator iter = find_if(func);
				if( iter != container_.end() )
					op1(std::ref(*iter));
				else
					op2();
			}

			/**
			* @brief 删除容器中元素第一个满足条件op的元素
			* @param <op> <回调函数参数，接受一个const T &参数，返回值为bool>
			* @exception <不会抛出任何异常>
			* @return <无>
			* @note <如果没有满足条件的元素，则不进行删除>
			* @remarks <使用find_if>
			*/
			template < typename OP >
			void erase(OP &&op)
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

			/**
			* @brief 对容器所有元素进行排序
			* @param <op> <回调函数参数，接受一个const T &参数，返回值为bool，需要支持'<' >
			* @exception <不会抛出任何异常>
			* @return <无>
			* @note <op需要支持'<'>
			* @remarks <无>
			*/
			template < typename OP >
			void sort(OP &&op)
			{
				AutoLock lock(mutex_);
				std::sort(container_.begin(), container_.end(), op);
			}

		};



		/**
		* @class <sync_assoc_container_t>
		* @brief 关联容器，线程安全，接口与stl容器类似
		*
		* K key类型
		* V value类型
		* C 容器类型，可以map、set、multi_map、multi_set、unordered_map、unordered_set
		* S 同步类型，可以critical_section、event_t、mutex
		*/

		template < 
			typename K, 
			typename V,
			typename C = std::map<K, V>, 
			typename S = std::recursive_mutex
		>
		class sync_assoc_container_t
		{
		public:
			typedef S Mutex;
			typedef std::unique_lock<Mutex>	AutoLock;
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

			sync_assoc_container_t(sync_assoc_container_t &&rhs)
				: container_(std::move(rhs.container_))
			{}

			sync_assoc_container_t &operator=(sync_assoc_container_t &&rhs)
			{
				if( this != &rhs )
				{
					container_ = std::move(rhs.container_);
				}

				return *this;
			}

			/**
			* @brief 传入一个allocator
			* @param <alloc> <allocator对象>
			* @exception <不会抛出任何异常>
			* @return <无>
			* @note <根据传入容器类型，定制容器内存分配器>
			* @remarks <提高内存分配效率>
			*/
			explicit sync_assoc_container_t(const allocator_type &alloc)
				: container_(alloc)
			{}

		private:
			sync_assoc_container_t(const sync_assoc_container_t &);
			sync_assoc_container_t &operator=(const sync_assoc_container_t &);

		public:
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

			std::pair<iterator, bool> insert(key_type &&key, mapped_type &&val)
			{
				AutoLock lock(mutex_);
				return container_.insert({std::forward<key_type>(key), std::forward<mapped_type>(val)});
			}

			/**
			* @brief 遍历整个容器
			* @param <op> <回调函数参数>
			* @exception <不会抛出任何异常>
			* @return <无>
			* @note <op是接受一个参数的回调函数>
			* @remarks <>
			*/
			template < typename OP >
			void for_each(OP &&op)
			{
				AutoLock lock(mutex_);
				std::for_each(container_.begin(), container_.end(), op);
			}

			template < typename OP >
			void for_each(const OP & op) const
			{
				AutoLock lock(mutex_);
				std::for_each(container_.cbegin(), container_.cend(), op);
			}

			/**
			* @brief 如果在容器中找不到key，则执行op
			* @param <key> <key关键字>
			* @param <op> <回调函数参数，接受一个const value_type &参数>
			* @exception <不会抛出任何异常>
			* @return <如果执行op，则返回true，否则返回false>
			* @note <无>
			* @remarks <无>
			*/
			template < typename OP >
			bool not_if_op(const key_type &key, OP &&op)
			{
				AutoLock lock(mutex_);
				const_iterator iter = container_.find(key);
				if( iter == container_.end() )
				{
					lock.unlock();
					op();
					return true;
				}

				return false;
			}

			/**
			* @brief 如果在容器中找到key，则执行op
			* @param <key> <key关键字>
			* @param <op> <回调函数参数，接受一个const value_type &参数>
			* @exception <不会抛出任何异常>
			* @return <如果执行op，则返回true，否则返回false>
			* @note <无>
			* @remarks <无>
			*/
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

			bool erase(const key_type &key)
			{
				AutoLock lock(mutex_);

				return container_.erase(key) != 0;
			}

			bool erase(const const_iterator &iter)
			{
				AutoLock lock(mutex_);

				return container_.erase(iter) != container_.cend();
			}
		};

	}

}



#endif