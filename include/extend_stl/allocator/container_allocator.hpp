#ifndef __EXTEND_STL_ALLOCATOR_POOL_ALLOCATOR_HPP
#define __EXTEND_STL_ALLOCATOR_POOL_ALLOCATOR_HPP

#include <limits>
#include <type_traits>
#include <cassert>

#ifdef max
#undef max
#endif


namespace stdex
{
	namespace allocator
	{
		namespace detail
		{
			template< typename PoolT >
			inline PoolT &default_pool_t()
			{
				static PoolT pool;
				return pool;
			}
		}


		template< typename T, typename MemoryPoolT >
		class pool_allocator_t
		{
		public:
			typedef MemoryPoolT					MemoryPoolType;

			typedef typename std::remove_const<T>::type	value_type;
			typedef value_type *				pointer;
			typedef const value_type *			const_pointer;
			typedef value_type &				reference;
			typedef const value_type &			const_reference;
			typedef size_t						size_type;
			typedef ptrdiff_t					difference_type;

			template<typename U>
			struct rebind
			{
				typedef pool_allocator_t<U, MemoryPoolType> other;
			};

		public:
			MemoryPoolType &pool_;

		public:
			pool_allocator_t()
				: pool_(detail::default_pool_t<MemoryPoolType>())
			{}

			explicit pool_allocator_t(MemoryPoolType &pool)
				: pool_(pool)
			{}
			pool_allocator_t(const pool_allocator_t<T, MemoryPoolType> &rhs)
				: pool_(rhs.pool_)
			{}
			template<typename U>
			pool_allocator_t(const pool_allocator_t<U, MemoryPoolType> &rhs)
				: pool_(rhs.pool_)
			{}
			template<typename U>
			pool_allocator_t &operator=(const pool_allocator_t<U, MemoryPoolType> &)
			{ return *this; }


		public:
			pointer address(reference r) const
			{ return &r; }
			const_pointer address(const_reference s) const
			{ return &s; }
			size_type max_size() const
			{ return (std::numeric_limits<size_type>::max)() / sizeof(value_type); }
			//void construct(pointer ptr, const T &t)
			//{ std::_Construct(ptr, t); }
			void construct(pointer _Ptr, T&& _Val)
			{	// construct object at _Ptr with value _Val
				::new ((void *)_Ptr) T(std::forward<T>(_Val));
			}

			template<typename U>
			void construct(pointer _Ptr, U _Val)
			{	
				::new ((void *)_Ptr) T(std::forward<U>(_Val));
			}

			void destroy(pointer ptr)
			{
				std::_Destroy(ptr);
				(void) ptr; // avoid unused variable warning
			}

			pointer allocate(const size_type n)
			{
				return reinterpret_cast<pointer>(pool_.allocate(n * sizeof(value_type)));
			}
			pointer allocate(const size_type n, const void * const)
			{ return allocate(n); }

			void deallocate(const pointer ptr, const size_type n)
			{
				if (ptr == 0 || n == 0)
				{
					assert(0);
					return;
				}

				pool_.deallocate(ptr, n * sizeof(value_type));
			}
		};


		template<typename T, typename U, typename MemoryPoolT>
		inline bool operator==(const pool_allocator_t<T, MemoryPoolT> &, const pool_allocator_t<U, MemoryPoolT> &)
		{ return true; }

		template<typename T, typename U, typename MemoryPoolT>
		inline bool operator!=(const pool_allocator_t<T, MemoryPoolT> &, const pool_allocator_t<U, MemoryPoolT> &)
		{ return false; }
	
		
		template< typename MemoryPoolT >
		class pool_allocator_t<void, MemoryPoolT>
		{
		public:
			typedef void*       pointer;
			typedef const void* const_pointer;
			typedef void        value_type;

			template<typename U> 
			struct rebind 
			{
				typedef pool_allocator_t<U, MemoryPoolT> other;
			};

			pool_allocator_t()
			{}

			pool_allocator_t(const pool_allocator_t<void, MemoryPoolT>&)
			{}

			template<class U>
			pool_allocator_t(const pool_allocator_t<U, MemoryPoolT>&)
			{}

			template<class U>
			pool_allocator_t& operator=(const pool_allocator_t<U, MemoryPoolT>&)
			{ return (*this); }
		};

	}


}





#endif