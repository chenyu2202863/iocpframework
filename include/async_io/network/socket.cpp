#include "socket.hpp"
#include "socket_option.hpp"

#include "../iocp/win_exception.hpp"


namespace async
{
	namespace network
	{

		socket_handle::socket_handle(dispatcher_type &io)
			: socket_(INVALID_SOCKET)
			, io_(io)
		{
		}
		socket_handle::socket_handle(dispatcher_type &io, SOCKET sock)
			: socket_(sock)
			, io_(io)
		{
		}
		socket_handle::socket_handle(dispatcher_type &io, int family, int type, int protocol)
			: socket_(INVALID_SOCKET)
			, io_(io)
		{
			open(family, type, protocol);
		}

		socket_handle::~socket_handle()
		{
			close();
		}


		void socket_handle::open(int family, int nType /* SOCK_STREAM */, int nProtocol /* IPPROTO_TCP */)
		{
			if( is_open() )
				throw exception::exception_base("Socket already opened!");

			socket_ = ::WSASocket(family, nType, nProtocol, NULL, 0, WSA_FLAG_OVERLAPPED);
			if( socket_ == INVALID_SOCKET )
				throw iocp::win32_exception("WSASocket");

			// 绑定到IOCP
			io_.bind(reinterpret_cast<HANDLE>(socket_));
		}
		
		void socket_handle::shutdown(int shut)
		{
			if( !is_open() )
				return;

			/*int ret = */::shutdown(socket_, shut);
			//assert(ret != SOCKET_ERROR);
		}

		void socket_handle::close()
		{
			if( !is_open() )
				return;

			int ret = ::closesocket(socket_);
			//assert(ret != SOCKET_ERROR);

			socket_ = INVALID_SOCKET;
		}
		
		void socket_handle::cancel()
		{
			if( !is_open() )
				throw exception::exception_base("Socket not Open");
			else
				socket_provider::cancel_io(socket_);
		}

		void socket_handle::bind(int family/* AF_INET*/, u_short uPort /* 0 */, const ip_address &addr /* INADDR_ANY */)
		{
			if( !is_open() )
				throw exception::exception_base("Socket not open");

			sockaddr_in addrIn = {0};
			addrIn.sin_family		= family;
			addrIn.sin_port			= ::htons(uPort);
			addrIn.sin_addr.s_addr	= ::htonl(addr.address());

			if( SOCKET_ERROR == ::bind(socket_, (const SOCKADDR *)&addrIn, sizeof(SOCKADDR)) )
				throw iocp::win32_exception("bind");
		}

		void socket_handle::listen(int nMax)
		{
			if( !is_open() )
				throw exception::exception_base("Socket not open");

			if( SOCKET_ERROR == ::listen(socket_, nMax) )
				throw iocp::win32_exception("listen");
		}

		socket_handle_ptr socket_handle::accept()
		{
			if( !is_open() )
				throw exception::exception_base("Socket not open");

			SOCKET sock = ::accept(socket_, 0, 0);
			if( sock == INVALID_SOCKET )
				throw iocp::win32_exception("accept");

			socket_handle_ptr remote(make_socket(io_, sock));
			return remote;
		}

		void socket_handle::connect(int family, const ip_address &addr, u_short uPort)
		{
			if( !is_open() )
				throw exception::exception_base("Socket not open");

			SOCKADDR_IN serverAddr = {0};
			serverAddr.sin_family		= family;
			serverAddr.sin_addr.s_addr	= ::htonl(addr.address());
			serverAddr.sin_port			= ::htons(uPort);

			if( SOCKET_ERROR == ::connect(socket_, reinterpret_cast<const sockaddr *>(&serverAddr), sizeof(SOCKADDR_IN)) )
				throw iocp::win32_exception("connect");
		}

		void socket_handle::dis_connect(int shut, bool bReuseSocket/* = true*/)
		{
			if( !is_open() )
				throw exception::exception_base("Socket not open");

			int ret = ::shutdown(socket_, shut);
			assert(ret != SOCKET_ERROR);
			
			if( bReuseSocket )
			{
				reuse_addr reuse(true);
				set_option(reuse);
			}
			else
			{
				close();
			}
		}
		
		size_t socket_handle::read(iocp::mutable_buffer &buffer, DWORD flag)
		{
			if( !is_open() )
				throw exception::exception_base("Socket not open");

			/*int ret = ::recv(socket_, buffer.data(), buffer.size(), (int)flag);
			if( ret == SOCKET_ERROR )
				throw iocp::win32_exception("recv");
			return size_t(ret);*/

			WSABUF wsabuf = {0};
			wsabuf.buf = buffer.data();
			wsabuf.len = buffer.size();

			if( wsabuf.len == 0 )
				throw exception::exception_base("Buffer allocate size is zero");

			DWORD dwSize = 0;
			if( 0 != ::WSARecv(socket_, &wsabuf, 1, &dwSize, &flag, 0, 0) )
				throw iocp::win32_exception("WSARecv");

			return dwSize;
		}

		size_t socket_handle::write(const iocp::const_buffer &buffer, DWORD flag)
		{
			if( !is_open() )
				throw exception::exception_base("Socket not open");

			/*int ret = ::send(socket_, buffer.data(), buffer.size(), (int)flag);
			if( ret == SOCKET_ERROR )
				throw iocp::win32_exception("send");
			return size_t(ret);*/

			WSABUF wsabuf = {0};
			wsabuf.buf = const_cast<char *>(buffer.data());
			wsabuf.len = buffer.size();

			if( wsabuf.len == 0 )
				throw exception::exception_base("Buffer size is zero");

			DWORD dwSize = 0;
			if( 0 != ::WSASend(socket_, &wsabuf, 1, &dwSize, flag, 0, 0) )
				throw iocp::win32_exception("WSASend");

			return dwSize;
		}


		size_t socket_handle::send_to(const iocp::const_buffer &buf, const SOCKADDR_IN *addr, DWORD flag)
		{
			if( !is_open() )
				throw exception::exception_base("Socket not open");

			WSABUF wsabuf = {0};
			wsabuf.buf = const_cast<char *>(buf.data());
			wsabuf.len = buf.size();

			if( wsabuf.len == 0 )
				throw exception::exception_base("Buffer size is zero");

			DWORD dwSize = 0;
			if( 0 != ::WSASendTo(socket_, &wsabuf, 1, &dwSize, flag, reinterpret_cast<const sockaddr *>(addr), addr == 0 ? 0 : sizeof(*addr), 0, 0) )
				throw iocp::win32_exception("WSASendTo");

			return dwSize;
		}

		size_t socket_handle::recv_from(iocp::mutable_buffer &buf, SOCKADDR_IN *addr, DWORD flag)
		{
			if( !is_open() )
				throw exception::exception_base("Socket not open");

			WSABUF wsabuf = {0};
			wsabuf.buf = buf.data();
			wsabuf.len = buf.size();

			if( wsabuf.len == 0 )
				throw exception::exception_base("Buffer allocate size is zero");

			DWORD dwSize = 0;
			int addrLen = sizeof(*addr);

			if( 0 != ::WSARecvFrom(socket_, &wsabuf, 1, &dwSize, &flag, reinterpret_cast<sockaddr *>(addr), addr == 0 ? 0 : &addrLen, 0, 0) )
				throw iocp::win32_exception("WSARecvFrom");

			return dwSize;
		}


		// 异步关闭连接
		void socket_handle::async_disconnect(const iocp::rw_callback_type &callback, bool bReuseSocket/* = true*/)
		{
			iocp::async_callback_base_ptr asynResult(iocp::make_async_callback<iocp::async_callback>(callback));

			DWORD dwFlags = bReuseSocket ? TF_REUSE_SOCKET : 0;

			if( !socket_provider::singleton().DisconnectEx(socket_, asynResult.get(), dwFlags, 0) 
				&& ::WSAGetLastError() != WSA_IO_PENDING )
				throw iocp::win32_exception("DisConnectionEx");

			asynResult.release();
		}

		// 异步接接收数据
		void socket_handle::async_read(iocp::mutable_buffer &buf, const iocp::rw_callback_type &callback)
		{
			iocp::async_callback_base_ptr asynResult(iocp::make_async_callback<iocp::async_callback>(callback));

			WSABUF wsabuf = {0};
			wsabuf.buf = buf.data();
			wsabuf.len = buf.size();

			DWORD dwFlag = 0;
			DWORD dwSize = 0;

			if( 0 != ::WSARecv(socket_, &wsabuf, 1, &dwSize, &dwFlag, asynResult.get(), NULL)
				&& ::WSAGetLastError() != WSA_IO_PENDING )
				throw iocp::win32_exception("WSARecv");

			asynResult.release();
		}

		// 异步发送数据
		void socket_handle::async_write(const iocp::const_buffer &buf, const iocp::rw_callback_type &callback)
		{
			iocp::async_callback_base_ptr asynResult(iocp::make_async_callback<iocp::async_callback>(callback));

			WSABUF wsabuf = {0};
			wsabuf.buf = const_cast<char *>(buf.data());
			wsabuf.len = buf.size();

			DWORD dwFlag = 0;
			DWORD dwSize = 0;

			if( 0 != ::WSASend(socket_, &wsabuf, 1, &dwSize, dwFlag, asynResult.get(), NULL)
				&& ::WSAGetLastError() != WSA_IO_PENDING )
				throw iocp::win32_exception("WSASend");

			asynResult.release();
		}

		// 异步UDP写出
		void socket_handle::async_send_to(const iocp::const_buffer &buf, const SOCKADDR_IN *addr, const iocp::rw_callback_type &callback)
		{
			iocp::async_callback_base_ptr asynResult(iocp::make_async_callback<iocp::async_callback>(callback));

			WSABUF wsabuf = {0};
			wsabuf.buf = const_cast<char *>(buf.data());
			wsabuf.len = buf.size();

			DWORD dwFlag = 0;
			DWORD dwSize = 0;

			if( 0 != ::WSASendTo(socket_, &wsabuf, 1, &dwSize, dwFlag, reinterpret_cast<const sockaddr *>(addr), addr == 0 ? 0 : sizeof(*addr), asynResult.get(), NULL)
				&& ::WSAGetLastError() != WSA_IO_PENDING )
				throw iocp::win32_exception("WSASendTo");
			
			asynResult.release();
		}	

		// 异步UDP读入
		void socket_handle::async_recv_from(iocp::mutable_buffer &buf, SOCKADDR_IN *addr, const iocp::rw_callback_type &callback)
		{
			iocp::async_callback_base_ptr asynResult(iocp::make_async_callback<iocp::async_callback>(callback));

			WSABUF wsabuf = {0};
			wsabuf.buf = buf.data();
			wsabuf.len = buf.size();

			DWORD dwFlag = 0;
			DWORD dwSize = 0;
			int addrLen = (addr == 0 ? 0 : sizeof(addr));

			if( 0 != ::WSARecvFrom(socket_, &wsabuf, 1, &dwSize, &dwFlag, reinterpret_cast<sockaddr *>(addr), &addrLen, asynResult.get(), NULL)
				&& ::WSAGetLastError() != WSA_IO_PENDING )
				throw iocp::win32_exception("WSARecvFrom");
			
			asynResult.release();
		}
	}

}