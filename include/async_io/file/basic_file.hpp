#ifndef __FILESYSTEM_BASIC_FILE_HPP
#define __FILESYSTEM_BASIC_FILE_HPP

#include "file.hpp"


namespace async
{
	namespace filesystem
	{
		
		struct readwrite
		{
			static const DWORD access_right				= GENERIC_READ | GENERIC_WRITE;
			static const DWORD shared_mode				= FILE_SHARE_READ | FILE_SHARE_WRITE;
			static const DWORD creation_flag			= OPEN_ALWAYS;
			static const DWORD attribute				= FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE;
		};

		struct readonly
		{
			static const DWORD access_right				= GENERIC_READ;
			static const DWORD shared_mode				= FILE_SHARE_READ;
			static const DWORD creation_flag			= OPEN_EXISTING;
			static const DWORD attribute				= FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE;
		};

		struct overlapped_read
		{
			static const DWORD access_right				= GENERIC_READ;
			static const DWORD shared_mode				= FILE_SHARE_READ;
			static const DWORD creation_flag			= OPEN_EXISTING;
			static const DWORD attribute				= FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED;
		};

		struct overlapped_write
		{
			static const DWORD access_right				= GENERIC_READ | GENERIC_WRITE;
			static const DWORD shared_mode				= FILE_SHARE_READ;
			static const DWORD creation_flag			= CREATE_ALWAYS;
			static const DWORD attribute				= FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED;
		};

		// -------------------------------------------
		// class basic_file

		class basic_file
		{
		public:
			typedef file_handle_ptr							impl_type;
			typedef file_handle::dispatcher_type			dispatcher_type;	

		private:
			impl_type impl_;

		public:
			explicit basic_file(dispatcher_type &io)
				: impl_(make_file(io))
			{}
			explicit basic_file(const impl_type &impl)
				: impl_(impl)
			{}
			template < typename FlagT >
			basic_file(dispatcher_type &io, LPCTSTR path, FlagT flag, LPSECURITY_ATTRIBUTES attribute = NULL, HANDLE templateMode = NULL)
				: impl_(make_file(io, path, flag.access_right, flag.shared_mode, flag.creation_flag, flag.attribute, attribute, templateMode))
			{}

		public:
			// 显示获取
			impl_type &get() 
			{
				return impl_;
			}
			const impl_type &get() const
			{
				return impl_;
			}

			// 支持隐式转换
			operator impl_type()
			{
				return impl_;
			}
			operator const impl_type&() const
			{
				return impl_;
			}

		public:
			template < typename FlagT >
			void open(LPCTSTR path, FlagT flag, LPSECURITY_ATTRIBUTES attribute = NULL, HANDLE templateMode = NULL)
			{
				return impl_->open(path, flag.access_right, flag.shared_mode, flag.creation_flag, flag.attribute, attribute, templateMode);
			}

			void close()
			{
				return impl_->close();
			}

			bool is_open() const
			{
				return impl_->is_open();
			}
			
			bool flush()
			{
				return impl_->flush();
			}

			bool cancel()
			{
				return impl_->cancel();
			}

			void set_file_size(unsigned long long size)
			{
				return impl_->set_file_size(size);
			}

		public:
			// 阻塞式发送数据直到数据发送成功或出错
			template < typename ConstBufferT >
			size_t write(const ConstBufferT &buffer)
			{
				return impl_->write(buffer, 0);
			}

			template < typename ConstBufferT >
			size_t write(const ConstBufferT &buffer, const u_int64 &offset)
			{
				return impl_->write(buffer, offset);
			}

			// 异步发送数据
			template < typename ConstBufferT, typename HandlerT >
			void async_write(const ConstBufferT &buffer, const HandlerT &callback)
			{
				return impl_->async_write(buffer, 0, callback);
			}

			template < typename ConstBufferT, typename HandlerT >
			void async_write(const ConstBufferT &buffer, const u_int64 &offset, const HandlerT &callback)
			{
				return impl_->async_write(buffer, offset, callback);
			}


			// 阻塞式接收数据直到成功或出错
			template < typename MutableBufferT >
			size_t read(MutableBufferT &buffer)
			{
				return impl_->read(buffer, 0);
			}

			template < typename MutableBufferT >
			size_t read(MutableBufferT &buffer, const u_int64 &offset)
			{
				return impl_->read(buffer, offset);
			}

			// 异步接收数据
			template < typename MutableBufferT, typename HandlerT >
			void async_read(MutableBufferT &buffer, const HandlerT &callback)
			{
				return impl_->async_read(buffer, 0, callback);
			}

			// 异步接收数据
			template < typename MutableBufferT, typename HandlerT >
			void async_read(MutableBufferT &buffer, const u_int64 &offset, const HandlerT &callback)
			{
				return impl_->async_read(buffer, offset, callback);
			}
		};
	}
}



#endif