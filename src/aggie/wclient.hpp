/*! \file wclient.hpp
 *
 * Container class for wireless clients.
 *
 * \date 2013
 * \author   Forsvarets Forskningsinstitutt (FFI)\n
 *           http://www.ffi.no \n
 *           - Terje Mikal Mjelde [tmo]\n
 *             terje-mikal-mjelde@ffi.no
 */

#ifndef __WCLIENT_HPP
#define __WCLIENT_HPP

#include "platform.h"
#include "ipsocket.hpp"
#include "timetools.hpp"
#include "resulthandler.hpp"

#include <sstream>
#include <string>
#include <vector>
#include <queue>

#ifdef PLATFORM_WINDOWS

#endif
#ifdef PLATFORM_LINUX
#	include <pthread.h>
#endif


extern timetools timers;


//! \defgroup IPCSERVER_MSGID Message ID's from PyRadac IPC Server
//!@{
#define IPCSERVER_REPLY_READY             200
#define IPCSERVER_REPLY_COMMAND_OUTPUT    201
#define IPCSERVER_REPLY_BANNER            211
#define IPCSERVER_REPLY_HELP              214
#define IPCSERVER_REPLY_DISCONNECTING     221
// 500 are server errors
#define IPCSERVER_REPLY_BUSY              500
// 400 are parsing/command errors
#define IPCSERVER_REPLY_INVALID_COMMAND   400
#define IPCSERVER_REPLY_INVALID_PARAMETER 401
//}@

/*! \brief Container class for Host/IP and Port.
 *
 */
class ip_address
{
public:
	ip_address();
	ip_address(std::string host, std::string port);
	ip_address(std::string host, unsigned port);
	ip_address(std::string host_and_port);
	std::string host() const;
	std::string port() const;
	std::string host_and_port() const;
	void set_host(std::string);
	void set_port(std::string);
	void set_port(unsigned);
	void set_host_and_port(std::string);
private:
	std::string host_;
	std::string port_;
};


/*! \brief Wireless client container class.
 *
 * One instance of this class for each client.
 */
class wclient
{
public:
	wclient(std::string);
	~wclient();
	struct client_node
	{
		unsigned id;
		unsigned age;
		unsigned cr;
		double lat;
		double lon;
		ip_address p2p_ip;
		ip_address radac_ip;
	};
	struct configuration
	{
		unsigned id;
		unsigned age;
		ip_address src_ip;
		std::string config;
	};
	struct connection
	{
		std::string dir;
		unsigned peer_id;
		ip_address peer_ip;
	};
	class compare
	{
	public:
		bool operator()(struct wclient::client_node c1, struct wclient::client_node c2)
		{
			return (c1.id < c2.id);
		}
	};
	std::string text();
	tcpsocket *socket;
	std::string host() const;
	std::string port() const;
	std::string host_and_port() const;
	bool received_message;
	timetools::handle last_received_message;
	bool sent_message;
	timetools::handle last_sent_message;
	std::vector<struct client_node> client_nodes;
	std::vector<struct configuration> configs;
	std::vector<struct connection> connections;
	bool client_nodes_list_finished;
	bool config_list_finished;
	bool connection_list_finished;
	RH send_command(std::string);
	std::vector<std::string> data_column;
	std::queue<std::string> request_command;
#	ifdef PLATFORM_LINUX
	pthread_mutex_t request_command_mutex;
#	endif
	bool data_changed;
private:
	ip_address ip;
};

#endif // WCLIENT_HPP
