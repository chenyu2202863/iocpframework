#include "stdafx.h"
#include "ConnectionMgr.h"


namespace http
{

	void ConnectionMgr::Start(const ConnectionPtr &c)
	{
		{
			AutoLock lock(lock_);
			connections_.insert(c);
		}

		c->Start();
	}

	void ConnectionMgr::Stop(const ConnectionPtr &c)
	{
		{
			AutoLock lock(lock_);
			connections_.erase(c);
		}

		c->Stop();
	}

	void ConnectionMgr::StopAll()
	{
		std::for_each(connections_.begin(), connections_.end(),
			std::tr1::bind(&Connection::Stop, std::placeholders::_1));

		{
			AutoLock lock(lock_);
			connections_.clear();
		}
	}
}