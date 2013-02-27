#ifndef __IOCP_POINTER_HPP
#define __IOCP_POINTER_HPP

#include "object_factory.hpp"


namespace async
{


	namespace iocp
	{

		template< typename T, typename RelaseFuncT >
		class pointer_t
		{
		private:
			RelaseFuncT release_;
			T *obj_;

		public:
			explicit pointer_t(T *obj)
				: obj_(obj)
			{}

			~pointer_t()
			{
				reset();
			}

		private:
			pointer_t(const pointer_t &);
			pointer_t &operator=(const pointer_t &);

		public:
			T *get() const
			{
				return obj_;
			}

			T *operator->()
			{
				return get();
			}

			T &operator*()
			{
				return *get();
			}

			T *release()
			{
				T *tmp = obj_;
				obj_ = 0;
				return tmp;
			}

			void reset()
			{
				if( obj_ )
				{
					release_(obj_);
					obj_ = 0;
				}
			}
		};

	}


}




#endif