#ifndef __FILESYSTEM_FILE_CHANGE_HPP
#define __FILESYSTEM_FILE_CHANGE_HPP

#include "../iocp/dispatcher.hpp"
#include "detail/file_change_notify_hook.hpp"
#include "detail/file_change_callback.hpp"


namespace async
{

	namespace filesystem
	{

		class change_monitor
		{
			typedef iocp::io_dispatcher dispatcher_type;
		private:
			// File Handle
			HANDLE file_;
			DWORD filter_;

			// IO服务
			dispatcher_type &io_;

		public:
			explicit change_monitor(dispatcher_type &, 
				DWORD filter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES |
				FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS |
				FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SECURITY);
			change_monitor(dispatcher_type &, LPCTSTR,
				DWORD filter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES |
				FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS |
				FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SECURITY);
			~change_monitor();

			// non-copyable
		private:
			change_monitor(const change_monitor &);
			change_monitor &operator=(const change_monitor &);


		public:
			// explicit转换
			operator HANDLE()					{ return file_; }
			operator const HANDLE () const		{ return file_; }

			// 显示获取
			HANDLE native_handle()				{ return file_; }
			const HANDLE native_handle() const	{ return file_; }

		public:
			void open(LPCTSTR path);

			void close();

			bool is_open() const
			{
				return file_ != INVALID_HANDLE_VALUE;
			}

		public:
			template < typename HandlerT >
			void monitor(const HandlerT &handler, bool sub_dir = true)
			{
				if( !is_open() )
					throw iocp::win32_exception("File Path Not Valid");

				typedef detail::file_change_handle_t<HandlerT> FileChangeHandleHook;
				iocp::file_change_callback_ptr asynResult(iocp::make_async_callback(FileChangeHandleHook(*this, handler)));
				
				DWORD ret = 0;
				BOOL suc = ::ReadDirectoryChangesW(file_, &asynResult->buffer_, iocp::file_change_callback::BUFFER_LEN, sub_dir ? TRUE : FALSE, 
					filter_, &ret, asynResult.get(), 0);
				
				if( !suc )
					throw iocp::win32_exception("ReadDirectoryChangesW");

				asynResult.release();
			}
		};
	}
}




#endif