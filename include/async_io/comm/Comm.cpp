#include "Comm.hpp"


namespace  async
{
	namespace comm
	{


		Comm::Comm(AsyncIODispatcherType &io)
			: comm_(INVALID_HANDLE_VALUE)
			, io_(io)
		{}


		Comm::Comm(AsyncIODispatcherType &io, const std::string &device)
			: comm_(INVALID_HANDLE_VALUE)
			, io_(io)
		{
			Open(device);
		}	

		Comm::~Comm()
		{
			Close();
		}


		void Comm::Open(const std::string &device)
		{
			std::string name = (device[0] == '\\') ? device : "\\\\.\\" + device;
			// 创建文件句柄
			comm_ = ::CreateFileA(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0,
				OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
			if( comm_ == INVALID_HANDLE_VALUE )
				throw iocp::Win32Exception("CreateFile");

			DCB dcb = GetState();

			// Set some default serial port parameters. This implementation does not
			// support changing these, so they might as well be in a known state.
			dcb.fBinary = TRUE; // Win32 only supports binary mode.
			dcb.fDsrSensitivity = FALSE;
			dcb.fNull = FALSE; // Do not ignore NULL characters.
			dcb.fAbortOnError = FALSE; // Ignore serial framing errors.
			SetState(dcb);

			// Set up timeouts so that the serial port will behave similarly to a
			// network socket. Reads wait for at least one byte, then return with
			// whatever they have. Writes return once everything is out the door.
			::COMMTIMEOUTS timeouts = {0};
			timeouts.ReadIntervalTimeout = 1;
			timeouts.ReadTotalTimeoutMultiplier = 0;
			timeouts.ReadTotalTimeoutConstant = 0;
			timeouts.WriteTotalTimeoutMultiplier = 0;
			timeouts.WriteTotalTimeoutConstant = 0;
			if( !::SetCommTimeouts(comm_, &timeouts) )
				throw iocp::Win32Exception("SetCommTimeouts");

			// 绑定到IOCP
			io_.Bind(comm_);
		}


		void Comm::Close()
		{
			if( comm_ != INVALID_HANDLE_VALUE )
			{
				::CloseHandle(comm_);
				comm_ = INVALID_HANDLE_VALUE;
			}
		}

		bool Comm::Cancel()
		{
			assert(comm_ != INVALID_HANDLE_VALUE);
			return ::CancelIo(comm_) == TRUE;
		}


		void Comm::SetState(const DCB &dcb)
		{
			if( !IsOpen() )
				throw std::logic_error("Comm not open");

			if( !::SetCommState(comm_, &const_cast<DCB &>(dcb)) )
				throw iocp::Win32Exception("SetCommState");
		}

		DCB Comm::GetState() const
		{
			if( !IsOpen() )
				throw std::logic_error("Comm not open");

			::DCB dcb = {0};
			dcb.DCBlength = sizeof(DCB);
			if( !::GetCommState(comm_, &dcb) )
				throw iocp::Win32Exception("GetCommState");

			return dcb;
		}

		void Comm::SetTimeOut()
		{
			
		}

		void Comm::GetTimeOut() const
		{

		}

		size_t Comm::Read(void *buf, size_t len)
		{
			if( !IsOpen() )
				throw std::logic_error("Comm not open");

			DWORD read = 0;
			if( !::ReadFile(comm_, buf, len, &read, 0) )
				throw iocp::Win32Exception("ReadFile");

			return read;
		}

		size_t Comm::Write(const void *buf, size_t len)
		{
			if( !IsOpen() )
				throw std::logic_error("Comm not open");

			DWORD read = 0;
			if( !::WriteFile(comm_, buf, len, &read, 0) )
				throw iocp::Win32Exception("ReadFile");

			return read;
		}

		void Comm::AsyncRead(void *buf, size_t len, const CallbackType &handler)
		{
			if( !IsOpen() )
				throw std::logic_error("Comm not open");

			AsyncCallbackBasePtr asynResult(iocp::MakeAsyncCallback<iocp::AsyncCallback>(handler));

			DWORD bytesRead = 0;
			BOOL bSuc = ::ReadFile(comm_, buf, len, &bytesRead, asynResult.Get());
			if( !bSuc && ::GetLastError() != ERROR_IO_PENDING )
				throw iocp::Win32Exception("ReadFile");

			asynResult.Release();
		}

		void Comm::AsyncWrite(const void *buf, size_t len, const CallbackType &handler)
		{
			if( !IsOpen() )
				throw std::logic_error("Comm not open");

			AsyncCallbackBasePtr asynResult(iocp::MakeAsyncCallback<iocp::AsyncCallback>(handler));

			DWORD bytesRead = 0;
			BOOL bSuc = ::WriteFile(comm_, buf, len, &bytesRead, asynResult.Get());

			if( !bSuc && ::GetLastError() != ERROR_IO_PENDING )
				throw iocp::Win32Exception("WriteFile");

			asynResult.Release();
		}




	}

}