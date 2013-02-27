// sock4_client.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <iostream>
#include <iomanip>
#include <ostream>
#include <string>

#include <iocp\dispatcher.hpp>
#include <network\tcp.hpp>

#include "socks4.hpp"



using namespace async;


int main(int argc, char* argv[])
{
	try
	{
		if (argc != 4)
		{
			std::cout << "Usage: sync_client <socks4server> <socks4port> <user>\n";
			std::cout << "Examples:\n";
			std::cout << "  sync_client 127.0.0.1 1080 chris\n";
			std::cout << "  sync_client localhost socks chris\n";
			return 1;
		}

		iocp::io_dispatcher io_service;

		

		// Try each endpoint until we successfully establish a connection to the
		// SOCKS 4 server.

		auto ip = network::ip_address::parse(std::string(argv[1]));
		auto port = atoi(argv[2]);
		network::tcp::socket socket(io_service, network::tcp::v4());
		socket.connect(ip, port);

		
		// Send the request to the SOCKS 4 server.
		socks4::request socks_request(socks4::request::connect, port, ip, argv[3]);

		auto buffer = socks_request.buffers();
		std::for_each(buffer.begin(), buffer.end(), [&socket](const iocp::const_buffer &buf)
		{
			socket.write(iocp::buffer(buf));
		});
		

		// Receive a response from the SOCKS 4 server.
		socks4::reply socks_reply;
		auto &read_buffers = socks_reply.buffers();
		std::for_each(read_buffers.begin(), read_buffers.end(), [&socket](iocp::mutable_buffer &buf)
		{
			socket.read(iocp::buffer(buf));
		});

		// Check whether we successfully negotiated with the SOCKS 4 server.
		if (!socks_reply.success())
		{
			std::cout << "Connection failed.\n";
			std::cout << "status = 0x" << std::hex << socks_reply.status();
			return 1;
		}

		// Form the HTTP request. We specify the "Connection: close" header so that
		// the server will close the socket after transmitting the response. This
		// will allow us to treat all data up until the EOF as the response.
		std::string request =
			"GET / HTTP/1.0\r\n"
			"Host: www.boost.org\r\n"
			"Accept: */*\r\n"
			"Connection: close\r\n\r\n";

		// Send the HTTP request.
		socket.write(iocp::buffer(request));

		// Read until EOF, writing data to output as we go.
		std::array<char, 512> response;
		while (1)
		{
			std::size_t s = socket.read(iocp::buffer(response));
			if( s == 0 )
				break;
			std::cout.write(response.data(), s);
		}
		
	}
	catch (std::exception& e)
	{
		std::cout << "Exception: " << e.what() << "\n";
	}

	system("pause");
	return 0;
}

