#include "socket.hpp"
#include "socket_option.hpp"

#include "../service/exception.hpp"
#include "../../extend_stl/allocator/stack_allocator.hpp"


namespace async { namespace network {

	socket_handle_t::socket_handle_t(dispatcher_type &io)
		: socket_(INVALID_SOCKET)
		, io_(io)
	{
	}
	socket_handle_t::socket_handle_t(dispatcher_type &io, SOCKET sock)
		: socket_(sock)
		, io_(io)
	{
	}
	socket_handle_t::socket_handle_t(dispatcher_type &io, int family, int type, int protocol)
		: socket_(INVALID_SOCKET)
		, io_(io)
	{
		open(family, type, protocol);
	}

	socket_handle_t::socket_handle_t(const socket_handle_t &rhs)
		: socket_(rhs.socket_)
		, io_(rhs.io_)
	{
	}

	socket_handle_t &socket_handle_t::operator=(const socket_handle_t &rhs)
	{
		if( &rhs != this )
		{
			socket_ = rhs.socket_;
		}

		return *this;
	}

	socket_handle_t::~socket_handle_t()
	{
		close();
	}


	void socket_handle_t::open(int family, int nType /* SOCK_STREAM */, int nProtocol /* IPPROTO_TCP */)
	{
		if( is_open() )
			throw service::network_exception("Socket already opened!");

		socket_ = ::WSASocket(family, nType, nProtocol, NULL, 0, WSA_FLAG_OVERLAPPED);
		if( socket_ == INVALID_SOCKET )
			throw service::win32_exception_t("WSASocket");

		// °ó¶¨µ½IOCP
		io_.bind(reinterpret_cast<HANDLE>(socket_));

		// 
		if( !::SetFileCompletionNotificationModes(reinterpret_cast<HANDLE>(socket_), FILE_SKIP_COMPLETION_PORT_ON_SUCCESS) )
			throw service::win32_exception_t("SetFileCompletionNotificationModes");
	}

	void socket_handle_t::shutdown(int shut)
	{
		if( !is_open() )
			return;

		::shutdown(socket_, shut);
	}

	void socket_handle_t::close()
	{
		if( !is_open() )
			return;

		int ret = ::closesocket(socket_);
		socket_ = INVALID_SOCKET;
	}

	void socket_handle_t::cancel()
	{
		if( !is_open() )
			throw service::network_exception("Socket not Open");
		else
			socket_provider::cancel_io(socket_);
	}

	void socket_handle_t::bind(int family/* AF_INET*/, u_short uPort /* 0 */, const ip_address &addr /* INADDR_ANY */)
	{
		if( !is_open() )
			throw service::network_exception("Socket not open");

		sockaddr_in addrIn = {0};
		addrIn.sin_family		= family;
		addrIn.sin_port			= ::htons(uPort);
		addrIn.sin_addr.s_addr	= ::htonl(addr.address());

		if( SOCKET_ERROR == ::bind(socket_, (const SOCKADDR *)&addrIn, sizeof(SOCKADDR)) )
			throw service::win32_exception_t("bind");
	}

	void socket_handle_t::listen(int nMax)
	{
		if( !is_open() )
			throw service::network_exception("Socket not open");

		if( SOCKET_ERROR == ::listen(socket_, nMax) )
			throw service::win32_exception_t("listen");
	}

	/*socket_handle_ptr socket_handle::accept()
	{
	if( !is_open() )
	throw service::network_exception("Socket not open");

	SOCKET sock = ::accept(socket_, 0, 0);
	if( sock == INVALID_SOCKET )
	throw service::win32_exception("accept");

	socket_handle_ptr remote(make_socket(io_, sock));
	return remote;
	}*/

	void socket_handle_t::connect(int family, const ip_address &addr, u_short uPort)
	{
		if( !is_open() )
			throw service::network_exception("Socket not open");

		SOCKADDR_IN serverAddr = {0};
		serverAddr.sin_family		= family;
		serverAddr.sin_addr.s_addr	= ::htonl(addr.address());
		serverAddr.sin_port			= ::htons(uPort);

		if( SOCKET_ERROR == ::connect(socket_, reinterpret_cast<const sockaddr *>(&serverAddr), sizeof(SOCKADDR_IN)) )
			throw service::win32_exception_t("connect");
	}

	void socket_handle_t::dis_connect(int shut, bool bReuseSocket/* = true*/)
	{
		if( !is_open() )
			throw service::network_exception("Socket not open");

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

	size_t socket_handle_t::read(service::mutable_buffer_t &buffer, DWORD flag)
	{
		if( !is_open() )
			throw service::network_exception("Socket not open");

		WSABUF wsabuf = {0};
		wsabuf.buf = buffer.data();
		wsabuf.len = buffer.size();

		if( wsabuf.len == 0 )
			throw service::network_exception("Buffer allocate size is zero");

		DWORD dwSize = 0;
		if( 0 != ::WSARecv(socket_, &wsabuf, 1, &dwSize, &flag, 0, 0) )
			throw service::win32_exception_t("WSARecv");

		return dwSize;
	}

	size_t socket_handle_t::write(const service::const_buffer_t &buffer, DWORD flag)
	{
		if( !is_open() )
			throw service::network_exception("Socket not open");

		WSABUF wsabuf = {0};
		wsabuf.buf = const_cast<char *>(buffer.data());
		wsabuf.len = buffer.size();

		if( wsabuf.len == 0 )
			throw service::network_exception("Buffer size is zero");

		DWORD dwSize = 0;
		if( 0 != ::WSASend(socket_, &wsabuf, 1, &dwSize, flag, 0, 0) )
			throw service::win32_exception_t("WSASend");

		return dwSize;
	}


	size_t socket_handle_t::send_to(const service::const_buffer_t &buf, const SOCKADDR_IN *addr, DWORD flag)
	{
		if( !is_open() )
			throw service::network_exception("Socket not open");

		WSABUF wsabuf = {0};
		wsabuf.buf = const_cast<char *>(buf.data());
		wsabuf.len = buf.size();

		if( wsabuf.len == 0 )
			throw service::network_exception("Buffer size is zero");

		DWORD dwSize = 0;
		if( 0 != ::WSASendTo(socket_, &wsabuf, 1, &dwSize, flag, reinterpret_cast<const sockaddr *>(addr), addr == 0 ? 0 : sizeof(*addr), 0, 0) )
			throw service::win32_exception_t("WSASendTo");

		return dwSize;
	}

	size_t socket_handle_t::recv_from(service::mutable_buffer_t &buf, SOCKADDR_IN *addr, DWORD flag)
	{
		if( !is_open() )
			throw service::network_exception("Socket not open");

		WSABUF wsabuf = {0};
		wsabuf.buf = buf.data();
		wsabuf.len = buf.size();

		if( wsabuf.len == 0 )
			throw service::win32_exception_t("Buffer allocate size is zero");

		DWORD dwSize = 0;
		int addrLen = sizeof(*addr);

		if( 0 != ::WSARecvFrom(socket_, &wsabuf, 1, &dwSize, &flag, reinterpret_cast<sockaddr *>(addr), addr == 0 ? 0 : &addrLen, 0, 0) )
			throw service::win32_exception_t("WSARecvFrom");

		return dwSize;
	}

}

}