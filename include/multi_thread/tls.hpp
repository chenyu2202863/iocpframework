#ifndef __MULTI_THREAD_TLS_HPP
#define __MULTI_THREAD_TLS_HPP

#include <exception>


namespace multi_thread
{
	// ----------------------------------------
	// class tls_ptr_t

	template < typename T >
	class tls_ptr_t
	{
	private:
		// 检测线程是否在线程池中
		DWORD tss_key_;

	public:
		tls_ptr_t()
			: tss_key_(::TlsAlloc())
		{
			if( tss_key_ == TLS_OUT_OF_INDEXES )
				throw std::runtime_error("TlsAlloc");
		}
		~tls_ptr_t()
		{
			BOOL suc = ::TlsFree(tss_key_);
			assert(suc);
		}

	public:
		operator T*()
		{
			return static_cast<T *>(::TlsGetValue(tss_key_));
		}
		T *operator->()
		{
			return static_cast<T *>(::TlsGetValue(tss_key_));
		}

		void operator=(T *val)
		{
			::TlsSetValue(tss_key_, val);
		}
	};



	// --------------------------------------------------
	// class CallStack

	// 检测当前是否在线程进行分派

	template < typename OwnerT >
	class call_stack_t
	{
	public:
		// 在栈上设置owner
		class context;

	private:
		// 在栈顶的调用
		static tls_ptr_t<context> top_;

	public:
		// 检测owner是否在栈上
		static bool contains(OwnerT *owner)
		{
			context *val = top_;
			while( val )
			{
				if( val->owner_ == owner )
					return true;

				val = val->next_;
			}

			return false;
		}

	};

	template < typename OwnerT >
	tls_ptr_t<typename call_stack_t<OwnerT>::context> call_stack_t<OwnerT>::top_;



	template < typename OwnerT >
	class call_stack_t<OwnerT>::context
	{
	private:
		OwnerT *owner_;		// owner与context关联
		context *next_;		// 在栈上的下一个元素

		friend class call_stack_t<OwnerT>;

	public:
		explicit context(OwnerT *owner)
			: owner_(owner)
			, next_(call_stack_t<OwnerT>::top_)
		{
			call_stack_t<OwnerT>::top_ = this;
		}
		~context()
		{
			call_stack_t<OwnerT>::top_ = next_;
		}

	private:
		context(const context &);
		context &operator=(const context &);
	};
}






#endif