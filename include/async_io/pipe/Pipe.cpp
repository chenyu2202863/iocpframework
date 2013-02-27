#include "stdafx.h"
#include "Pipe.hpp"



namespace async
{
	namespace pipe
	{
		Pipe::Pipe(DispatcherType &io)
			: io_(io)
			, pipe_(INVALID_HANDLE_VALUE)
		{}

		void Pipe::Create(LPCTSTR lpName, DWORD dwOpenMode, DWORD dwPipeMode, DWORD nMaxInstances,
			DWORD nOutBufferSize, DWORD nInBufferSize, DWORD nDefaultTimeOut, LPSECURITY_ATTRIBUTES lpSecurityAttributes/* = NULL*/)
		{
			assert(!IsOpen());
			assert(_tcslen(lpName) != 0);

			TCHAR pszPipeName[MAX_PATH] = {0};
			_tcscpy(pszPipeName, _T("\\\\.\\PIPE\\"));
			_tcscat(pszPipeName, lpName);


			pipe_ = ::CreateNamedPipe(pszPipeName, dwOpenMode, dwPipeMode, nMaxInstances, nOutBufferSize, nInBufferSize, nDefaultTimeOut, lpSecurityAttributes);
			if( pipe_ == INVALID_HANDLE_VALUE )
				throw Win32Exception(_T("CreateNamePipe"));
		}

		void Pipe::Open(LPCTSTR lpszServerName, LPCTSTR lpszPipeName, DWORD dwDesiredAccess, 
			DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwFlagsAndAttributes)
		{
			assert(!IsOpen());
			assert(_tcslen(lpszServerName));
			assert(_tcslen(lpszPipeName));
			
			std::basic_string<TCHAR> pipeName  = _T("\\\\");
			pipeName += lpszServerName + _T("\\") + lpszPipeName;
			
			pipe_ = ::CreateFile(pipeName.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
				 OPEN_EXISTING, dwFlagsAndAttributes, NULL);
			
			if( pipe_ == INVALID_HANDLE_VALUE )
				throw Win32Exception(_T("CreateFile"));
		}

		void Pipe::Close()
		{
			assert(IsOpen());

			if( !::CloseHandle(pipe_) )
				throw Win32Exception(_T("CloseHandle"));

			pipe_ = INVALID_HANDLE_VALUE;
		}


		void Pipe::Connect()
		{
			if( !::ConnectNamedPipe(pipe_, 0) )
				throw Win32Exception(_T("ConnectNamedPipe"));
		}

		void Pipe::AsyncConnect(const IOCallbackType &callback)
		{
			assert(IsOpen());

			AsyncCallbackBasePtr asynResult(MakeAsyncIOCallback(callback));
			//asynResult->AddRef();

			if( !::ConnectNamedPipe(pipe_, asynResult.Get()) )
			{
				throw Win32Exception(_T("ConnectNamedPipe"));
				asynResult->Release();
			}
		}

		void Pipe::Disconnect()
		{
			assert(IsOpen());

			if( !::DisconnectNamedPipe(pipe_) )
				throw Win32Exception(_T("DisconnectNamedPipe"));
		}

		bool Pipe::Flush()
		{
			assert(IsOpen());
			return ::FlushFileBuffers(pipe_) == TRUE;
		}


		void Pipe::Write(void *buf, size_t len)
		{
			assert(IsOpen());

			DWORD outLen = 0;
			if( !::WriteFile(pipe_, buf, len, &outLen, 0) )
				throw Win32Exception(_T("WriteFile"));
		}

		void Pipe::Read(const void *buf, size_t len)
		{
			assert(IsOpen());

			DWORD outLen = 0;
			if( !::ReadFile(pipe_, buf, len, &outLen, 0) )
				throw Win32Exception(_T("ReadFile"));
		}

		void Pipe::AsyncWrite(const void *buf, size_t size, const IOCallbackType &callback)
		{
			assert(IsOpen());
			AsyncCallbackBasePtr asynResult(MakeAsyncIOCallback(callback));

			DWORD outLen = 0;
			if( !::WriteFile(pipe_, buf, size, &outLen, asynResult.Get()) )
			{
				if( ::WSAGetLastError() != ERROR_IO_PENDING )
				{
					asynResult->Release();
					throw iocp::Win32Exception("WriteFile");
				}
			}
		}

		void Pipe::AsyncRead(void *buf, size_t size, const IOCallbackType &callback)
		{
			assert(IsOpen());

			AsyncCallbackBasePtr asynResult(MakeAsyncIOCallback(callback));

			DWORD outLen = 0;
			if( !::ReadFile(pipe_, buf, size, &outLen, asynResult.Get()) )
			{
				if( ::WSAGetLastError() != ERROR_IO_PENDING )
				{
					asynResult->Release();
					throw iocp::Win32Exception("ReadFile");
				}
			}
		}

		void Pipe::Call(LPCTSTR lpszServerName, LPCTSTR lpszPipeName, LPVOID lpInBuffer, 
			DWORD dwInBufferSize, void *lpOutBuffer, DWORD dwOutBufferSize, DWORD &dwBytesRead, DWORD dwTimeOut)
		{
			assert(!IsOpen());
			assert(_tcslen(lpszServerName));
			assert(_tcslen(lpszPipeName));

			TCHAR pszPipeName[MAX_PATH] = {0};
			_tcscpy(pszPipeName, _T("\\\\.\\PIPE\\"));
			_tcscat(pszPipeName, lpName);
		
			
			if( !::CallNamedPipe(pszPipeName, lpInBuffer, dwInBufferSize, lpOutBuffer, 
				dwOutBufferSize, &dwBytesRead, dwTimeOut) )
				throw iocp::Win32Exception("ReadFile");
		}

		void Pipe::Wait(LPCTSTR lpszServerName, LPCTSTR lpszPipeName, DWORD dwTimeOut)
		{
			assert(!IsOpen());
			assert(_tcslen(lpszServerName));
			assert(_tcslen(lpszPipeName));

			TCHAR pszPipeName[MAX_PATH] = {0};
			_tcscpy(pszPipeName, _T("\\\\.\\PIPE\\"));
			_tcscat(pszPipeName, lpName);
			
			if( !::WaitNamedPipe(pszPipeName, dwTimeOut) )
				throw iocp::Win32Exception("ReadFile");
		}


		// 从管道里读出数据，但不删除管道里数据
		void Pipe::Peek(void *lpBuffer, u_long dwBufferSize, u_long &dwBytesRead, 
			u_long &dwTotalBytesAvail, u_long &dwBytesLeftThisMessage)
		{
			assert(IsOpen());
			
			if( !::PeekNamedPipe(pipe_, lpBuffer, dwBufferSize, &dwBytesRead,
				&dwTotalBytesAvail, &dwBytesLeftThisMessage) )
				throw Win32Exception("PeekNamedPipe");
		}
		
		// 合并读取与写入功能
		u_long Pipe::Transact(void *lpInBuffer, u_long dwInBufferSize, void *lpOutBuffer, u_long dwOutBufferSize)
		{
			assert(IsOpen());
			
			u_long readBytes = 0;
			if( !::TransactNamedPipe(pipe_, lpInBuffer, dwInBufferSize, lpOutBuffer, dwOutBufferSize, &readBytes, 0) )
				throw Win32Exception("TransactNamedPipe");

			return readBytes;
		}

		void Pipe::AsyncTransact(void *lpInBuffer, u_long dwInBufferSize, void *lpOutBuffer, u_long dwOutBufferSize, const IOCallbackType &callback)
		{
			assert(IsOpen());

			AsyncCallbackBasePtr asynResult(MakeAsyncIOCallback(callback));
			u_long readBytes = 0;
			if( !::TransactNamedPipe(pipe_, lpInBuffer, dwInBufferSize, lpOutBuffer, dwOutBufferSize, &readBytes, asynResult.Get()) )
				throw Win32Exception("TransactNamedPipe");
		}


		bool Pipe::IsBlocking() const
		{
			assert(IsOpen());

			DWORD dwState = 0;
			if( ::GetNamedPipeHandleState(pipe_, &dwState, NULL, NULL, NULL, NULL, 0) )
				throw Win32Exception("GetNamedPipeHandleState");
			
			return (dwState & PIPE_NOWAIT) == 0;
		}

		bool Pipe::IsServer() const
		{
			assert(IsOpen());

			DWORD dwFlags = 0;
			if( ::GetNamedPipeInfo(pipe_, &dwFlags, NULL, NULL, NULL) )
				throw Win32Exception("GetNamedPipeHandleState");

			return (dwFlags & PIPE_SERVER_END) == 0;
		}

		bool Pipe::IsClient() const
		{
			assert(IsOpen());

			DWORD dwFlags = 0;
			if( ::GetNamedPipeInfo(pipe_, &dwFlags, NULL, NULL, NULL) )
				throw Win32Exception("GetNamedPipeHandleState");

			return (dwFlags & PIPE_CLIENT_END) == 0;
		}

		bool Pipe::IsMessage() const
		{
			assert(IsOpen());

			DWORD dwFlags = 0;
			if( ::GetNamedPipeHandleState(pipe_, &dwState, NULL, NULL, NULL, NULL, 0) )
				throw Win32Exception("GetNamedPipeHandleState");

			return (dwFlags & PIPE_READMODE_MESSAGE) == 0;
		}

		u_long Pipe::GetCurrentInstances() const
		{
			assert(IsOpen());
			
			u_long dwCurInstances = 0;
			if( ::GetNamedPipeHandleState(pipe_, NULL, &dwCurInstances, NULL, NULL, NULL, 0) )
				throw Win32Exception("GetNamedPipeHandleState");                                
			
			return dwCurInstances;
		}

		void Pipe::SetMode(bool isByte, bool isBlocking)
		{
			assert(IsOpen());
			
			DWORD dwMode = 0;
			if( isByte )
			{
				if( isBlocking )
					dwMode = PIPE_READMODE_BYTE | PIPE_WAIT;
				else
					dwMode = PIPE_READMODE_BYTE | PIPE_NOWAIT;
			}
			else
			{
				if( isBlocking )
					 dwMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
				else
					 dwMode = PIPE_READMODE_MESSAGE | PIPE_NOWAIT;
			}
			
			if( !::SetNamedPipeHandleState(pipe_, &dwMode, NULL, NULL) )
				throw Win32Exception("SetNamedPipeHandleState");
	}
}