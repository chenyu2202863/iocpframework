#ifndef __EXTEND_STL_STACK_ALLOCATOR_HPP
#define __EXTEND_STL_STACK_ALLOCATOR_HPP

#include <type_traits>
#include <cassert>
#include <array>
#include <cstdint>

namespace stdex { namespace allocator {


	template<std::uint32_t n_stack_elements>
	struct stack_storage_t
	{
		enum { size = n_stack_elements };

		typedef typename std::aligned_storage<sizeof(char), std::alignment_of<char>::value>::type storage_type;
		
		std::uint32_t pos_;
		storage_type array_[n_stack_elements];

		stack_storage_t()
			: pos_(0)
		{
		}

		stack_storage_t(stack_storage_t &&rhs)
			: pos_(rhs.pos_)
		{
			std::memmove(&array_[0], &rhs.array_[0], pos_);
		}

	private:
		stack_storage_t(const stack_storage_t &);
		stack_storage_t &operator=(const stack_storage_t &);
	};

	template<typename T, std::uint32_t n_stack_elements>
	class stack_allocator_t
		: public std::allocator<T>
	{
		typedef std::allocator<T> base_class;
		
	public:
		template<typename U>
		struct rebind
		{
			typedef stack_allocator_t<U, n_stack_elements> other;
		};

		stack_storage_t<n_stack_elements> *storage_;


		explicit stack_allocator_t(stack_storage_t<n_stack_elements> *storage)
			: storage_(storage)
		{
		}

		stack_allocator_t(stack_allocator_t &&rhs)
			: storage_(rhs.storage_)
		{}

		template < typename U >
		stack_allocator_t(const stack_allocator_t<U, n_stack_elements> &rhs)
			: storage_(rhs.storage_)
		{}

		typename base_class::pointer allocate(typename base_class::size_type n_elements,
			std::allocator<void>::const_pointer hint = 0)
		{
			assert(storage_->pos_ + n_elements * sizeof(base_class::value_type) <= n_stack_elements);
			if( storage_->pos_ + n_elements * sizeof(base_class::value_type) > n_stack_elements )
				return nullptr;
			else
			{
				base_class::pointer p = reinterpret_cast<base_class::pointer>(&storage_->array_[storage_->pos_]);
				storage_->pos_ += n_elements * sizeof(base_class::value_type);
				return p;
			}
		}
		void deallocate(typename base_class::pointer/* p*/, typename base_class::size_type/* n*/)
		{
			
		}	
	};

}


}


#endif

