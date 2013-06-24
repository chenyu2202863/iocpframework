#ifndef __NETWORK_HELPER_HPP
#define __NETWORK_HELPER_HPP


/** @network_helper.hpp
*
* @author <陈煜>
* [@author <chenyu2202863@yahoo.com.cn>]
* @date <2012/10/12>
* @version <0.1>
*
* win32中网络相关帮助函数
*/


#include <cstdint>

#include <WinSock2.h>
#include <Windows.h>
#include <iphlpapi.h>
#include <vector>
#include <Psapi.h>


#include "../../extend_stl/string/algorithm.hpp"
#include "../../unicode/string.hpp"
#include "../../utility/select.hpp"


#pragma comment(lib, "Psapi")
#pragma comment(lib, "Ws2_32")
#pragma comment(lib, "Iphlpapi")


/*
ip转换
	ip_2_string
	string_2_ip

验证IP
	is_valid_ip	

检测port是否占用
	is_port_used
	
本地网络信息
	get_local_ips
	get_local_gate_ips
	get_mac_addr
	get_local_dns

mac转换
	mac_string_to_binary

是否本地判断
	is_local_by_ip
	is_local_by_machine_name


根据socket获取IP
	get_sck_ip

*/


namespace win32
{
	namespace network
	{

		namespace detail
		{
			struct ip_2_string_helper_t
			{
				unsigned long ip_;
				ip_2_string_helper_t(unsigned long ip)
					:ip_(ip)
				{}

				template < typename CharT >
				operator std::basic_string<CharT>() const
				{
					std::basic_ostringstream<CharT> os_;

					in_addr tmp = {0};
					tmp.s_addr = ip_;

					char *p = ::inet_ntoa(tmp);
					os_ << p;
					
					return std::move(os_.str());
				}
			};
		}

		/**
		* @brief IP转字符串
		* @param <ip> <IP数据，网络序>
		* @exception <无任何异常抛出>
		* @return <返回IP对应的字符串>
		* @note <无>
		* @remarks <无>
		*/
		inline detail::ip_2_string_helper_t ip_2_string(unsigned long ip)
		{
			return detail::ip_2_string_helper_t(ip);
		}
		
		/**
		* @brief 字符串转IP
		* @param <ip> <IP字符串>
		* @exception <无任何异常抛出>
		* @return <返回IP， 网络序>
		* @note <返回的是网络序IP数字>
		* @remarks <无>
		*/
		inline unsigned long string_2_ip(const std::string &ip)
		{
			return ::ntohl(::inet_addr(ip.c_str()));
		}

		inline unsigned long string_2_ip(const std::wstring &ip)
		{
			return string_2_ip((LPCSTR)CW2A(ip.c_str()));
		}


		/**
		* @brief 判断是否为合法IP
		* @param <ip> <IP字符串>
		* @exception <无任何异常抛出>
		* @return <如果为合法IP则为true，否则为false>
		* @note <无>
		* @remarks <无>
		*/
		template < typename CharT >
		inline bool is_valid_ip(const std::basic_string<CharT> &ip)
		{
			int nIP = 0, nCount = 0;   

			std::vector<std::basic_string<CharT>> vecIP;
			stdex::split(vecIP, ip, CharT('.'));

			// 无4个点 认为不符合规则
			if( vecIP.size() != 4 ) 
				return false;

			// ip首尾段为0 认为不符合规则
			if( *(vecIP.begin()) == utility::select<CharT>("0", L"0") || 
				*(vecIP.rbegin()) == utility::select<CharT>("0", L"0") )
				return false;

			for(auto iter = vecIP.begin(); iter != vecIP.end(); ++iter)
			{
				if( *iter == utility::select<CharT>("255", L"255") ) 
					++nCount;

				// ip 每个段不在0－255范围内 认为不符合规则
				nIP = stdex::to_number(*iter);
				if ( nIP < 0 || nIP > 255 )
					return false;
			}

			// ip 每个段都是255 认为不符合规则
			if ( nCount == 4 ) 
				return false;

			// ip 解释成功 认为符合规则
			if( ::inet_addr(unicode::translate_t<CharT>::utf(ip).c_str()) != INADDR_NONE )   
				return true; 

			// 否则认为不符合规则
			return false;
		}

		template < typename CharT >
		inline bool is_valid_ip(const CharT *ip)
		{
			std::basic_string<CharT> val(ip);
			return is_valid_ip(val);
		}

		/**
		* @brief 得到本机所有IP
		* @param <IPs> <IP字符串集合>
		* @exception <无任何异常抛出>
		* @return <如果成功则填充本机上的IP>
		* @note <无>
		* @remarks <无>
		*/
		template < typename CharT >
		inline bool get_local_ips(std::vector<std::basic_string<CharT>> &IPs)
		{
			IP_ADAPTER_INFO info[16] = {0};
			DWORD dwSize = sizeof(info);
			if( ERROR_SUCCESS != ::GetAdaptersInfo(info, &dwSize) )
				return false; 

			PIP_ADAPTER_INFO pAdapter = info;
			while (pAdapter != NULL)
			{
				PIP_ADDR_STRING pAddr = &pAdapter->IpAddressList;
				while (pAddr != NULL)
				{
					std::basic_string<CharT> tmp = unicode::translate_t<CharT>::utf(pAddr->IpAddress.String);

					IPs.push_back(std::move(tmp));
					pAddr = pAddr->Next;
				}
				pAdapter = pAdapter->Next;
			}
			return true;
		}


		/**
		* @brief 获取本地网关IP
		* @param <IPs> <IP字符串集合>
		* @exception <无任何异常抛出>
		* @return <如果成功则填充本机上的网关IP>
		* @note <无>
		* @remarks <无>
		*/
		template < typename CharT >
		inline bool get_local_gate_ips(std::vector<std::basic_string<CharT>> &gateIPs)
		{
			IP_ADAPTER_INFO info[16] = {0};
			DWORD dwSize = sizeof(info);
			if( ERROR_SUCCESS != ::GetAdaptersInfo(info, &dwSize) )
				return false; 

			PIP_ADAPTER_INFO pAdapter = info;
			while (pAdapter != NULL)
			{
				PIP_ADDR_STRING pAddr = &pAdapter->GatewayList;
				while (pAddr != NULL)
				{
					std::basic_string<CharT> &&tmp = unicode::to(std::basic_string<CharT>(pAddr->IpAddress.String));
					gateIPs.push_back(std::move(tmp));
					pAddr = pAddr->Next;
				}
				pAdapter = pAdapter->Next;
			}
			return true;
		}

		/**
		* @brief 根据IP获取MAC地址
		* @param <ip> <ip字符串>
		* @exception <无任何异常抛出>
		* @return <如果成功则返回该IP对应MAC地址>
		* @note <无>
		* @remarks <无>
		*/
		template < typename CharT >
		inline std::basic_string<CharT> get_mac_addr(const std::basic_string<CharT> &ip)
		{
			assert(is_valid_ip(ip));

			const int MAX_ADAPTER_NUM = 10; //最多支持10个网卡
			IP_ADAPTER_INFO astAdapter[MAX_ADAPTER_NUM] = {0};
			ULONG nSize = sizeof(astAdapter);
			if( ERROR_SUCCESS != ::GetAdaptersInfo(astAdapter, &nSize) )
			{
				assert(0 && "网卡的数量超出预计");
				return std::basic_string<CharT>();
			}

			const std::string &&srcIP = unicode::to(ip);
			for(PIP_ADAPTER_INFO pAdapter = astAdapter; pAdapter != NULL; pAdapter = pAdapter->Next)
			{
				// 确保是以太网,确保MAC地址的长度为 00-00-00-00-00-00
				if(pAdapter->Type == MIB_IF_TYPE_ETHERNET && 
					pAdapter->AddressLength == 6 && 
					srcIP == pAdapter->IpAddressList.IpAddress.String)
				{
					CharT mac[32] = {0};
					utility::select<CharT>(sprintf_s, swprintf_s)(mac, _countof(mac), 
						utility::select<CharT>("%02X-%02X-%02X-%02X-%02X-%02X", L"%02X-%02X-%02X-%02X-%02X-%02X"),
						int (pAdapter->Address[0]),
						int (pAdapter->Address[1]),
						int (pAdapter->Address[2]),
						int (pAdapter->Address[3]),
						int (pAdapter->Address[4]),
						int (pAdapter->Address[5]));
					return std::move(std::basic_string<CharT>(mac));
				}
			}

			return std::basic_string<CharT>();
		}

		template < typename CharT >
		std::basic_string<CharT> get_mac_addr(const CharT *ip)
		{
			std::basic_string<CharT> val(ip);
			return get_mac_addr(val);
		}

		/**
		* @brief 根据本地DNS
		* @param <dns> <需要填充dns集合>
		* @exception <无任何异常抛出>
		* @return <如果成功则返回true，否则返回false>
		* @note <无>
		* @remarks <无>
		*/
		template < typename CharT >
		inline bool get_local_dns(std::vector<std::basic_string<CharT>> &dns)
		{
			FIXED_INFO fixed = {0};
			ULONG outBufLen = sizeof(FIXED_INFO);

			std::vector<char> tmpBuf;
			FIXED_INFO *tmpFixedInfo = 0;
			if( ::GetNetworkParams(&fixed, &outBufLen) == ERROR_BUFFER_OVERFLOW ) 
			{
				tmpBuf.resize(outBufLen);
				tmpFixedInfo = reinterpret_cast<FIXED_INFO *>(tmpBuf.data());
			}
			else
			{
				tmpFixedInfo = &fixed;
			}

			IP_ADDR_STRING *pIPAddr = 0;
			DWORD dwRetVal = 0;
			if( dwRetVal = ::GetNetworkParams(tmpFixedInfo, &outBufLen) == NO_ERROR ) 
			{
				std::basic_string<CharT> &&tmp = unicode::to(std::basic_string<CharT>(tmpFixedInfo->DnsServerList.IpAddress.String));
				dns.push_back(tmp);

				pIPAddr = tmpFixedInfo->DnsServerList.Next;
				while (pIPAddr) 
				{
					pIPAddr = tmpFixedInfo->DnsServerList.Next;
					tmp = std::move(unicode::to(std::basic_string<CharT>(pIPAddr->IpAddress.String)));
					dns.push_back(std::move(tmp));

					pIPAddr = pIPAddr->Next;
				}
			}


			return true;
		}


		/**
		* @brief MAC字符串转二进制地址
		* @param <mac> <MAC地址字符串>
		* @exception <无任何异常抛出>
		* @return <返回二进制>
		* @note <无>
		* @remarks <无>
		*/
		template < typename CharT >
		std::basic_string<CharT> mac_string_to_binary(const std::basic_string<CharT> &mac)
		{
			if( mac.empty() )
				return std::basic_string<CharT>();

			static const size_t len = ::strlen("00-12-EF-AC-0A-78");

			if( mac.length() != len ) 
				return std::basic_string<CharT>();

			std::basic_string<CharT> tmp;
			for(size_t i = 0; i < 6; ++i) 
			{
				tmp += (mac[i*3] - (mac[i*3] >= 'A' ? ('A'-10) : '0')) * 16;
				tmp += mac[i*3+1] - (mac[i*3+1] >= 'A' ? ('A'-10) : '0');
			}

			return tmp;
		}


		/**
		* @brief 根据IP判断是否为本地IP
		* @param <ip> <ip地址>
		* @exception <无任何异常抛出>
		* @return <如果是本地则返回true，否则返回false>
		* @note <无>
		* @remarks <无>
		*/
		template < typename CharT >
		inline bool is_local_by_ip(const std::basic_string<CharT> &ip)
		{
			if( ip == utility::select<CharT>("0.0.0.0", L"0.0.0.0") )
				return false;

			if( ip == utility::select<CharT>("127.0.0.1", L"127.0.0.1") )
				return true;

			std::vector<std::basic_string<CharT>> IPs;
			if( !get_local_ips(IPs) )
				return false;

			return std::find(IPs.begin(), IPs.end(), ip) != IPs.end();
		}

		template < typename CharT >
		inline bool is_local_by_ip(const CharT *ip)
		{
			std::basic_string<CharT> val(ip);
			return is_local_by_ip(val);
		}

		/**
		* @brief 根据机器名判断是否为本地IP
		* @param <name> <机器名称>
		* @exception <无任何异常抛出>
		* @return <如果是本地则返回true，否则返回false>
		* @note <无>
		* @remarks <无>
		*/
		template < typename CharT >
		inline bool is_local_by_machine_name(const std::basic_string<CharT> &name)
		{
			hostent *remoteHost = ::gethostbyname(unicode::to_a(name).c_str());
			if( remoteHost == 0 )
				return false;

			in_addr addr = {0};
			addr.s_addr = *(u_long *)remoteHost->h_addr_list[0];

			std::string ip = ::inet_ntoa(addr);
			return is_local_by_ip(ip);
		}

		template < typename CharT >
		inline bool is_local_by_machine_name(const CharT *name)
		{
			std::basic_string<CharT> val(name);
			return is_local_by_machine_name(val);
		}
		
		/**
		* @brief 根据SOCKET返回该SOCKET的IP
		* @param <hSocket> <socket句柄>
		* @exception <无任何异常抛出>
		* @return <返回socket的句柄>
		* @note <无>
		* @remarks <无>
		*/
		inline std::uint32_t get_sck_ip(SOCKET hSocket)
		{
			assert(hSocket != NULL && hSocket != INVALID_SOCKET);
			sockaddr_in addr = {0};
			int len = sizeof(addr);
			if (::getpeername(hSocket, reinterpret_cast<PSOCKADDR>(&addr), &len) != 0)
			{
				assert(0 && "错误的套接字");
			}
			return addr.sin_addr.S_un.S_addr;
		}

		/**
		* @brief 判断指定端口是被占用
		* @param <port> <指定的端口号>
		* @exception <无任何异常抛出>
		* @return <如果被占用则为true，否则为false>
		* @note <传入端口号为网络序>
		* @remarks <无>
		*/
		inline bool is_port_used(std::uint16_t port)
		{
			std::vector<MIB_TCPTABLE> tcp_table;
			tcp_table.resize(100);

			DWORD table_size = sizeof(MIB_TCPTABLE) * tcp_table.size();
			DWORD ret = ::GetTcpTable(&tcp_table[0], &table_size, TRUE);

			if( ret == ERROR_INSUFFICIENT_BUFFER )
			{
				tcp_table.resize(table_size / sizeof(MIB_TCPTABLE));
				ret = ::GetTcpTable(&tcp_table[0], &table_size, TRUE);
				assert(ret == NO_ERROR);
			}
			else if( ret == NO_ERROR )
			{
				size_t mib_cnt = tcp_table[0].dwNumEntries;
				for(size_t i = 0; i != mib_cnt; ++i)
				{
					MIB_TCPROW tcp_row = tcp_table[0].table[i];
					std::uint16_t tmp_port = ::ntohs((std::uint16_t)tcp_row.dwLocalPort);
					if( tmp_port == port )
						return true;
				}
			}
			

			return false;
		}

	}
}




#endif