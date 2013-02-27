#ifndef __IOCP_IOCP_HPP
#define __IOCP_IOCP_HPP

#include "../basic.hpp"
#include <cassert>



namespace async
{

	namespace iocp
	{
		//--------------------------------------------------------------
		// class iocp_handle

		class iocp_handle
		{
		private:
			HANDLE iocp_;

		public:
			iocp_handle()
				: iocp_(0)
			{}
			~iocp_handle()
			{
				close();
			}

		public:
			bool is_open() const
			{
				return iocp_ != 0;
			}

			void close()
			{
				if( iocp_ != 0 )
				{
					BOOL bRes = ::CloseHandle(iocp_);
					iocp_ = 0;
				}
			}

			bool create(size_t nMaxConcurrency)
			{
				iocp_ = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, nMaxConcurrency);

				assert(iocp_ != 0);

				return iocp_ != 0;
			}

			bool associate_device(HANDLE hDevice, ULONG_PTR ulCompKey)
			{
				assert(iocp_ != 0);

				return ::CreateIoCompletionPort(hDevice, iocp_, ulCompKey, 0) == iocp_;
			}

			bool post_status(ULONG_PTR ulCompKey, DWORD dwNumBytes = 0, OVERLAPPED *pOver = 0)
			{
				BOOL bOk = ::PostQueuedCompletionStatus(iocp_, dwNumBytes, ulCompKey, pOver);

				return bOk == TRUE;
			}

			bool get_status(ULONG_PTR *pCompKey, PDWORD pdwNumBytes, OVERLAPPED **pOver, DWORD dwMilliseconds = INFINITE)
			{
				return TRUE == ::GetQueuedCompletionStatus(iocp_, pdwNumBytes, pCompKey, pOver, dwMilliseconds);
			}

			operator HANDLE()
			{
				return iocp_;
			}

			operator const HANDLE() const
			{
				return iocp_;
			}
		};


	}

}

#endif