#ifndef __FILESYSTEM_FILE_HPP
#define __FILESYSTEM_FILE_HPP

#include "../iocp/async_result.hpp"
#include "../iocp/read_write_buffer.hpp"
#include "../iocp/buffer.hpp"
#include "../iocp/read.hpp"
#include "../iocp/write.hpp"


namespace async
{
	namespace iocp
	{
		class io_dispatcher;
	}


	namespace filesystem
	{
		// forward declare

		class file_handle;
		typedef std::shared_ptr<file_handle> file_handle_ptr;
		

		//--------------------------------------------------------------------------------
		// class File

		class file_handle
		{
		public:
			typedef iocp::io_dispatcher	dispatcher_type;

		private:
			// File Handle
			HANDLE file_;
			// IO服务
			dispatcher_type &io_;

		public:
			explicit file_handle(dispatcher_type &);
			file_handle(dispatcher_type &, LPCTSTR, DWORD, DWORD, DWORD, DWORD, LPSECURITY_ATTRIBUTES = NULL, HANDLE = NULL);
			~file_handle();

			// non-copyable
		private:
			file_handle(const file_handle &);
			file_handle &operator=(const file_handle &);


		public:
			// explicit转换
			operator HANDLE()					{ return file_; }
			operator const HANDLE () const		{ return file_; }

			// 显示获取
			HANDLE native_handle()					{ return file_; }
			const HANDLE native_handle() const		{ return file_; }

		public:
			// 打开目标文件
			void open(LPCTSTR, DWORD, DWORD, DWORD, DWORD, LPSECURITY_ATTRIBUTES = NULL, HANDLE = NULL);
			// 关闭
			void close();
			
			// 是否打开
			bool is_open() const
			{ return file_ != INVALID_HANDLE_VALUE; }

			// 刷新
			bool flush();
			
			//	取消
			bool cancel();

			// 设置文件大小
			void set_file_size(unsigned long long size);

			// 不需设置回调接口,同步函数
		public:
			size_t read(iocp::mutable_buffer &buffer, const u_int64 &offset);
			size_t write(const iocp::const_buffer &buffer, const u_int64 &offset);

			// 异步调用接口
		public:
			void async_read(iocp::mutable_buffer &buffer, const u_int64 &offset, const iocp::rw_callback_type &handler);
			void async_write(const iocp::const_buffer &buffer, const u_int64 &offset, const iocp::rw_callback_type &handler);
		};
	
		
		file_handle_ptr make_file(file_handle::dispatcher_type &io);
		file_handle_ptr make_file(file_handle::dispatcher_type &io, LPCTSTR path, DWORD access, DWORD shared, DWORD create, DWORD flag, LPSECURITY_ATTRIBUTES attribute = NULL, HANDLE templateMode = NULL);
		
	}
	
}




#endif