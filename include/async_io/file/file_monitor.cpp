#include "file_monitor.hpp"


namespace async
{
	namespace filesystem
	{

		change_monitor::change_monitor(dispatcher_type &io, DWORD filter)
			: io_(io)
			, file_(INVALID_HANDLE_VALUE)
			, filter_(filter)
		{}

		change_monitor::change_monitor(dispatcher_type &io, LPCTSTR path, DWORD filter)
			: io_(io)
			, file_(INVALID_HANDLE_VALUE)
			, filter_(filter)
		{
			open(path);
		}
		change_monitor::~change_monitor()
		{
			close();
		}


		void change_monitor::open(LPCTSTR path)
		{
			assert(!is_open());
			if( is_open() )
				return;

			file_ = ::CreateFile(path,  FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, 0);
			if( file_ == INVALID_HANDLE_VALUE )
				throw iocp::win32_exception("CreateFile");

			io_.bind(file_);
		}

		void change_monitor::close()
		{
			if( is_open() )
			{
				::CloseHandle(file_);
				file_ = INVALID_HANDLE_VALUE;
			}
		}

	}
}