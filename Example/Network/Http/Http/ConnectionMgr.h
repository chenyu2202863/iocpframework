#ifndef __HTTP_CONNECTION_MANAGER_HPP
#define __HTTP_CONNECTION_MANAGER_HPP

#include <set>
#include "Connection.h"

#include <multi_thread/lock.hpp>

namespace http
{

	class ConnectionMgr
	{
		typedef multi_thread::critical_section		CSLock;
		typedef multi_thread::auto_lock_t<CSLock>	AutoLock;

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