#ifndef __IOCP_BUFFER_HPP
#define __IOCP_BUFFER_HPP



#include <array>
#include <vector>
#include <string>

#include "object_factory.hpp"
#include "read_write_buffer.hpp"
#include "../../extend_stl/allocator/container_allocator.hpp"


namespace async
{


	namespace iocp
	{
		
		enum { DEFAULT_SIZE = 8 * 1024 };

		//-------------------------------------------------------------------
		// class BufferT
		// 提供缓冲区服务，支持STL标准接口

		template<typename T, typename AllocT>
		class auto_buffer_t
		{
		public:
			typedef T			value_type;
			typedef T*			pointer;
			typedef const T*	const_pointer;
			typedef T&			reference;
			typedef const T&	const_reference;

			// 内存分配类型
			typedef AllocT		allocator_type;

		private:
			allocator_type alloc_;		// 分配器

			bool internal_;				// 是否外部提供缓冲区
			size_t capacity_;			// 已分配缓冲区大小
			size_t bufferSize_;			// 已使用缓冲区大小

			value_type *buffer_;		// 缓冲区指针
			
		public:
			explicit auto_buffer_t(size_t defaultSize = DEFAULT_SIZE, const allocator_type &alloc = allocator_type())
				: alloc_(alloc)
				, internal_(true)
				, capacity_(defaultSize)
				, bufferSize_(0)
				, buffer_(_Allocate(defaultSize))
			{}
			auto_buffer_t(pointer pStr, size_t nSize, const allocator_type &alloc = allocator_type())
				: alloc_(alloc)
				, internal_(false)
				, capacity_(nSize)
				, bufferSize_(nSize)
				, buffer_(pStr)
			{
			}
			auto_buffer_t(const_pointer beg, const_pointer end, const allocator_type &alloc = allocator_type())
				: alloc_(alloc)
				, internal_(false)
				, capacity_(std::distance(beg, end))
				, bufferSize_(capacity_)
				, buffer_(beg)
			{
			}

			~auto_buffer_t()
			{
				// 如果为内部分配缓冲区，则需要自动释放
				if( internal_ )
					alloc_.deallocate(buffer_, capacity_);
			}

		public:
			pointer begin()
			{
				return buffer_;
			}
			const_pointer begin() const
			{
				return buffer_;
			}

			pointer end()
			{
				return buffer_ + bufferSize_;
			}
			const_pointer end() const
			{
				return buffer_ + bufferSize_;
			}

			size_t size() const
			{
				return bufferSize_;
			}
			size_t capacity() const
			{
				return capacity_;
			}

			void resize(size_t nNewSize)
			{
				// 如果是外部缓冲区
				if( !internal_ )
				{
					if( nNewSize <= capacity_ )
						bufferSize_ = nNewSize;
					else
						throw exception::exception_base("buffer out of range");
				}

				if( nNewSize <= capacity_ )
					bufferSize_ = nNewSize;
				else
				{
					// 申请新缓冲区
					pointer pNewBuf = _Allocate(nNewSize);

					// 复制缓冲区
					std::copy(buffer_, buffer_ + capacity_, 
						stdext::make_checked_array_iterator(pNewBuf, nNewSize));

					// 释放旧缓冲区
					alloc_.deallocate(buffer_, capacity_);

					capacity_	= nNewSize;
					bufferSize_ = nNewSize;
					buffer_		= pNewBuf;
				}
			}

			pointer data(size_t offset = 0)
			{
				if( offset >= capacity_ )
					throw exception::exception_base("buffer offset >= allocSize_");

				return buffer_ + offset;
			}
			const_pointer data(size_t offset = 0) const
			{
				if( offset >= capacity_ )
					throw exception::exception_base("buffer offset >= allocSize_");

				return buffer_ + offset;
			}

			pointer operator[](size_t offset)
			{
				return buffer_ + offset;
			}
			const_pointer operator[](size_t offset) const
			{
				return buffer_ + offset;
			}

			void append(pointer buf, size_t len)
			{
				if( len > capacity_ - bufferSize_ )
					resize(2 * len);

				std::copy(buf, buf + len, buffer_);
				bufferSize_ += len;
			}

		private:
			pointer _Allocate(size_t nSize)
			{
				pointer pBuf = alloc_.allocate(nSize);
				std::uninitialized_fill_n(pBuf, nSize, 0);

				return pBuf;
			}
		};

		typedef memory_pool::sgi_memory_pool_t<true, DEFAULT_SIZE>		memory_pool_type;

		typedef stdex::allocator::pool_allocator_t< char, memory_pool_type > auto_buffer_allocator;
		typedef auto_buffer_t< char, auto_buffer_allocator >			auto_buffer;
		typedef std::shared_ptr<auto_buffer>							auto_buffer_ptr;




		

		// -------------------- Buffer Helper Function -----------------------------

		inline auto_buffer_ptr make_buffer(size_t sz)
		{
			return auto_buffer_ptr(object_allocate<auto_buffer>(sz), &object_deallocate<auto_buffer>);
		}

		inline auto_buffer_ptr make_buffer(char *buf, size_t sz)
		{
			return auto_buffer_ptr(object_allocate<auto_buffer>(buf, sz), &object_deallocate<auto_buffer>);
		}

		inline auto_buffer_ptr make_buffer(const char *buf, size_t sz)
		{
			return make_buffer(const_cast<char *>(buf), sz);
		}

		// --------------------------

		template<size_t _N>
		inline auto_buffer_ptr make_buffer(char (&arr)[_N])
		{
			return auto_buffer_ptr(object_allocate<auto_buffer>(arr, _N), &object_deallocate<auto_buffer>);
		}
		template<size_t _N>
		inline auto_buffer_ptr make_buffer(const char (&arr)[_N])
		{
			return make_buffer(const_cast<char (&)[_N]>(arr));
		}

		// --------------------------

		template<size_t _N>
		inline auto_buffer_ptr make_buffer(std::tr1::array<char, _N> &arr)
		{
			return auto_buffer_ptr(object_allocate<auto_buffer>(arr.data(), _N), &object_deallocate<auto_buffer>);
		}
		template<size_t _N>
		inline auto_buffer_ptr make_buffer(const std::tr1::array<char, _N> &arr)
		{
			return make_buffer(const_cast<std::tr1::array<char, _N> &>(arr));
		}

		// --------------------------

		inline auto_buffer_ptr make_buffer(std::vector<char> &arr)
		{
			return auto_buffer_ptr(object_allocate<auto_buffer>(&arr[0], arr.size()), &object_deallocate<auto_buffer>);
		}
		inline auto_buffer_ptr make_buffer(const std::vector<char> &arr)
		{
			return make_buffer(const_cast<std::vector<char> &>(arr));
		}

		// --------------------------

		inline auto_buffer_ptr make_buffer(std::string &arr)
		{
			return auto_buffer_ptr(object_allocate<auto_buffer>(&*arr.begin(), arr.length()), &object_deallocate<auto_buffer>);
		}
		inline auto_buffer_ptr make_buffer(const std::string &arr)
		{
			return make_buffer(const_cast<std::string &>(arr));
		}


		// -----------------------------
		inline const_buffer buffer(const auto_buffer &buf)
		{
			return const_buffer(buf.data(), buf.size());
		}

		inline mutable_buffer buffer(auto_buffer &buf)
		{
			return mutable_buffer(buf.data(), buf.size());
		}

	} // end of iocp


} // end of async





#endif