#ifndef __HTTP_CONNECTION_MANAGER_HPP
#define __HTTP_CONNECTION_MANAGER_HPP

#include <set>
#include "Connection.h"

#include "../../../../MultiThread/Lock.hpp"

namespace http
{

	class ConnectionMgr
	{
		typedef async::thread::AutoCriticalSection CSLock;
		typedef async::thread::AutoLock<CSLock>	AutoLock;

	private:
		std::set<ConnectionPtr> connections_;
		CSLock lock_;

	public:
		ConnectionMgr(){}

	private:
		ConnectionMgr(const ConnectionMgr &);
		ConnectionMgr &operator=(const ConnectionMgr &);

	public:
		void Start(const ConnectionPtr &cc);
		void Stop(const ConnectionPtr &c);

		void StopAll();
	};
}




#endif