// ProtoBufClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "../../../../DispatchEngine/ClientEngine.hpp"
#include "../../../../DispatchEngine/SyncHelper.hpp"
#include "test.pb.h"



void handle_error(const std::basic_string<TCHAR> &msg)
{
	std::wcout << msg << std::endl;
}


void handle_connect(bool suc)
{
	std::cout << (suc ? "connected" : "un connected") << std::endl;
	if( !suc )
		return;

}



int _tmain(int argc, _TCHAR* argv[])
{
	async::iocp:: IODispatcher io;
	proto_engine::client cli(io, &handle_error, &handle_connect);
	cli.start("127.0.0.1", 5050);
	
	test::Require require;
	require.set_msg("hello world");

	auto response = async::dispatch::sync_dispatch<std::shared_ptr<test::Response>>(cli, require, 5000);
	
	std::cout << response->msg() << std::endl;
	system("pause");

	cli.stop();
	return 0;
}

