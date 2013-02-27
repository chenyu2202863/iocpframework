#include "stdafx.h"


#include "../../../../DispatchEngine/ServerEngine.hpp"
#include "test.pb.h"




class server
{
	async::iocp::IODispatcher io_;
	std::auto_ptr<proto_engine::server> svr_;

public:
	server() 
	{}
	~server()
	{}

private:
	server(const server &);
	server &operator=(const server &);

public:
	void start(u_short port)
	{
		svr_.reset(new proto_engine::server(io_, port, std::bind(&server::_on_error, this, std::placeholders::_1)));
		using namespace std::placeholders;
		svr_->register_callback<test::Require>(std::bind(&server::_on_require, this, _1, _2));
		svr_->start();
	}

	void stop()
	{
		svr_->stop();
		io_.Stop();
	}

private:
	void _on_require(const proto_engine::session_ptr &impl, const std::shared_ptr<test::Require> &msg)
	{
		std::cout << msg->msg() << std::endl;

		test::Response response;
		response.set_msg(msg->msg());
	
		impl->send(response);
	}
	
	void _on_error(const std::basic_string<TCHAR> &msg)
	{
		std::wcout << msg << std::endl;
	}
};


int _tmain(int argc, _TCHAR* argv[])
{
	server svr;
	svr.start(5050);

	system("pause");
	
	svr.stop();

	return 0;
}

