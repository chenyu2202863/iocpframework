#ifndef __EXTEND_STL_ALLOCATOR_HPP
#define __EXTEND_STL_ALLOCATOR_HPP

#include <allocators>


/*
	allocator_new_del
	allocator_unbounded
	allocator_fixed_size
	allocator_variable_size
	allocator_suballoc
	allocator_chunklist
*/


namespace stdex
{
	namespace allocator
	{
		// http://msdn.microsoft.com/en-us/library/ee292134.aspx

		// 进程内同步
		using namespace stdext::allocators;

		// 线程内同步
		_ALLOCATOR_DECL(CACHE_FREELIST(stdext::allocators::max_none),
			sync_per_thread, allocator_per_thread_newdel);
		_ALLOCATOR_DECL(CACHE_FREELIST(stdext::allocators::max_unbounded),
			sync_per_thread, allocator_per_thread_unbounded);
		_ALLOCATOR_DECL(CACHE_FREELIST(stdext::allocators::max_fixed_size<10>),
			sync_per_thread, allocator_per_thread_fixed_size);
		_ALLOCATOR_DECL(CACHE_FREELIST(stdext::allocators::max_variable_size),
			sync_per_thread, allocator_per_thread_variable_size);
		_ALLOCATOR_DECL(CACHE_SUBALLOC,
			sync_per_thread, allocator_per_thread_suballoc);
		_ALLOCATOR_DECL(CACHE_CHUNKLIST,
			sync_per_thread, allocator_per_thread_chunklist);

		// 同类型容器同步
		_ALLOCATOR_DECL(CACHE_FREELIST(stdext::allocators::max_none),
			sync_per_container, allocator_per_container_newdel);
		_ALLOCATOR_DECL(CACHE_FREELIST(stdext::allocators::max_unbounded),
			sync_per_container, allocator_per_container_unbounded);
		_ALLOCATOR_DECL(CACHE_FREELIST(stdext::allocators::max_fixed_size<10>),
			sync_per_container, allocator_per_container_fixed_size);
		_ALLOCATOR_DECL(CACHE_FREELIST(stdext::allocators::max_variable_size),
			sync_per_container, allocator_per_container_variable_size);
		_ALLOCATOR_DECL(CACHE_SUBALLOC,
			sync_per_container, allocator_per_container_suballoc);
		_ALLOCATOR_DECL(CACHE_CHUNKLIST,
			sync_per_container, allocator_per_container_chunklist);

		// 没有同步
		_ALLOCATOR_DECL(CACHE_FREELIST(stdext::allocators::max_none),
			sync_none, allocator_none_sync_newdel);
		_ALLOCATOR_DECL(CACHE_FREELIST(stdext::allocators::max_unbounded),
			sync_none, allocator_none_sync_unbounded);
		_ALLOCATOR_DECL(CACHE_FREELIST(stdext::allocators::max_fixed_size<10>),
			sync_none, allocator_none_sync_fixed_size);
		_ALLOCATOR_DECL(CACHE_FREELIST(stdext::allocators::max_variable_size),
			sync_none, allocator_none_sync_variable_size);
		_ALLOCATOR_DECL(CACHE_SUBALLOC,
			sync_none, allocator_none_sync_suballoc);
		_ALLOCATOR_DECL(CACHE_CHUNKLIST,
			sync_none, allocator_none_sync_chunklist);
	}	
}

#endif