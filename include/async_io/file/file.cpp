#include "file.hpp"

#include "../iocp/dispatcher.hpp"




namespace  async
{
	namespace iocp
	{
		typedef filesystem::file_handle File;

		// File 工厂
		template<>
		struct object_factory_t< File >
		{
			typedef memory_pool::fixed_memory_pool_t<true, sizeof(File)> PoolType;
			typedef object_pool_t< PoolType >							 ObjectPoolType;
		};
	}



	namespace filesystem
	{

		file_handle_ptr make_file(file_handle::dispatcher_type &io)
		{
			return file_handle_ptr(iocp::object_allocate<file_handle>(io), &iocp::object_deallocate<file_handle>);
		}

		file_handle_ptr make_file(file_handle::dispatcher_type &io, LPCTSTR path, DWORD access, DWORD shared, DWORD create, DWORD flag, LPSECURITY_ATTRIBUTES attribute/* = NULL*/, HANDLE templateMode/* = NULL*/)
		{
			return file_handle_ptr(iocp::object_allocate<file_handle>(io, path, access, shared, create, flag, attribute, templateMode), &iocp::object_deallocate<file_handle>);
		}


		file_handle::file_handle(dispatcher_type &io)
			: file_(INVALID_HANDLE_VALUE)
			, io_(io)
		{}


		file_handle::file_handle(dispatcher_type &io, LPCTSTR lpszFilePath, DWORD dwAccess, DWORD dwShareMode, 
			DWORD dwCreatePosition, DWORD dwFlag, LPSECURITY_ATTRIBUTES lpAttributes/* = NULL*/, HANDLE hTemplate/* = NULL*/)
			: file_(INVALID_HANDLE_VALUE)
			, io_(io)
		{
			open(lpszFilePath, dwAccess, dwShareMode, dwCreatePosition, dwFlag, lpAttributes, hTemplate);
		}	

		file_handle::~file_handle()
		{
			close();
		}


		void file_handle::open(LPCTSTR lpszFilePath, DWORD dwAccess, DWORD dwShareMode, DWORD dwCreatePosition, DWORD dwFlag, LPSECURITY_ATTRIBUTES attribute /* = NULL */, HANDLE hTemplate /* = NULL */)
		{
			// 创建文件句柄
			file_ = ::CreateFile(lpszFilePath, dwAccess, dwShareMode, attribute, dwCreatePosition, dwFlag, hTemplate);
			if( file_ == INVALID_HANDLE_VALUE )
				throw iocp::win32_exception("CreateFile");

			// 不触发文件对象 Vista
			//::SetFileCompletionNotificationModes(file_, FILE_SKIP_EVENT_ON_HANDLE);

			if( dwFlag & FILE_FLAG_NO_BUFFERING ||
				dwFlag & FILE_FLAG_OVERLAPPED )
			{
				// 绑定到IOCP
				io_.bind(file_);
			}
		}


		void file_handle::close()
		{
			if( file_ != INVALID_HANDLE_VALUE )
			{
				::CloseHandle(file_);
				file_ = INVALID_HANDLE_VALUE;
			}
		}

		bool file_handle::flush()
		{
			assert(file_ != INVALID_HANDLE_VALUE);
			return ::FlushFileBuffers(file_) == TRUE;
		}

		bool file_handle::cancel()
		{
			assert(file_ != INVALID_HANDLE_VALUE);
			return ::CancelIo(file_) == TRUE;
		}

		void file_handle::set_file_size(unsigned long long size)
		{
			LARGE_INTEGER offset = {0};
			offset.QuadPart = size;
			if( !::SetFilePointerEx(file_, offset, 0, FILE_BEGIN) )
				throw iocp::win32_exception("SetFilePointerEx");

			if( !::SetEndOfFile(file_) )
				throw iocp::win32_exception("SetEndOfFile");
		}


		size_t file_handle::read(iocp::mutable_buffer &buffer, const u_int64 &offset)
		{
			if( !is_open() )
				throw exception::exception_base("File not open");

			LARGE_INTEGER off = {0};
			off.QuadPart = offset;
			if( !::SetFilePointerEx(file_, off, 0, FILE_BEGIN) )
				throw iocp::win32_exception("SetFilePointerEx");

			DWORD read = 0;
			if( !::ReadFile(file_, buffer.data(), buffer.size(), &read, 0) )
				throw iocp::win32_exception("ReadFile");

			return read;
		}

		size_t file_handle::write(const iocp::const_buffer &buffer, const u_int64 &offset)
		{
			if( !is_open() )
				throw exception::exception_base("File not open");

			LARGE_INTEGER off = {0};
			off.QuadPart = offset;
			if( !::SetFilePointerEx(file_, off, 0, FILE_BEGIN) )
				throw iocp::win32_exception("SetFilePointerEx");

			DWORD write = 0;
			if( !::WriteFile(file_, buffer.data(), buffer.size(), &write, 0) )
				throw iocp::win32_exception("ReadFile");

			return write;
		}

		void file_handle::async_read(iocp::mutable_buffer &buffer, const u_int64 &offset, const iocp::rw_callback_type &handler)
		{
			if( !is_open() )
				throw exception::exception_base("File not open");

			iocp::async_callback_base_ptr asynResult(iocp::make_async_callback<iocp::async_callback>(handler));
			asynResult->Offset		= offset & 0xFFFFFFFF;
			asynResult->OffsetHigh	= (offset >> 32) & 0xFFFFFFFF;

			DWORD bytesRead = 0;
			BOOL bSuc = ::ReadFile(file_, buffer.data(), buffer.size(), &bytesRead, asynResult.get());
			if( !bSuc && ::GetLastError() != ERROR_IO_PENDING )
				throw iocp::win32_exception("ReadFile");

			asynResult.release();
		}

		void file_handle::async_write(const iocp::const_buffer &buffer, const u_int64 &offset, const iocp::rw_callback_type &handler)
		{
			if( !is_open() )
				throw exception::exception_base("File not open");

			iocp::async_callback_base_ptr asynResult(iocp::make_async_callback<iocp::async_callback>(handler));

			asynResult->Offset		= offset & 0xFFFFFFFF;
			asynResult->OffsetHigh	= (offset >> 32) & 0xFFFFFFFF;

			DWORD bytesRead = 0;
			BOOL bSuc = ::WriteFile(file_, buffer.data(), buffer.size(), &bytesRead, asynResult.get());

			if( !bSuc && ::GetLastError() != ERROR_IO_PENDING )
				throw iocp::win32_exception("WriteFile");

			asynResult.release();
		}


		

	}

}