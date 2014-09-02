/*! \file aggie.hpp
 *
 * \brief Main working class
 *
 * \date 2013
 * \author   Forsvarets Forskningsinstitutt (FFI)\n
 *           http://www.ffi.no \n
 *           - Terje Mikal Mjelde [tmo]\n
 *             terje-mikal-mjelde@ffi.no
 */


#ifndef __AGGIE_HPP
#define __AGGIE_HPP


#include "platform.h"
#include "resulthandler.hpp"
#include "vout.hpp"
#include "ipsocket.hpp"
#include "timetools.hpp"
#include "wclient.hpp"
#include "threadable.hpp"

#include <queue>
#include <vector>
#include <set>

#ifdef PLATFORM_WINDOWS
#	include <windows.h>
#endif
#ifdef PLATFORM_LINUX
#	include <pthread.h>
#endif

//! \brief Time to wait after startup before requesting information from the clients.
#define CLIENTS_CN_POLL_INITIAL_DELAY_MS 10

//! \defgroup CLIENT_COMMANDS Commands we send to the clients.
//!@{
#define GET_CLIENT_NODES "list cn"
#define GET_CONFIGS "list configs"
#define GET_CONNECTIONS "list connections"
//!@}

/*! \brief The main application class where most of the work is coordinated.
 *
 */
class aggie : public threadable
{
public:
	aggie();
	RH add_clients(std::string clients_filename);
	RH add_clients();
	RH delete_clients();
	RH connect_clients();
	RH disconnect_clients();
	RH connect_pm(std::string url);
	RH connect_pm(std::string destination, unsigned port, std::string path);
	RH start_pm_listener(websocket::websocket_callback);
	RH disconnect_pm();
	RH send_pm(std::string);
	RH start();
	RH stop();
	RH shutdown();
	void pm_listener(websocket *listener, std::string data);
	void client_listener(tcpsocket *listener, std::string data);
	std::vector<std::string> client_status(wclient *c);
	std::vector<std::string> client_status();
	std::vector<std::string> client_status(std::string host, std::string port);
	std::vector<std::string> status();
	void get_info_from_clients();
	void get_info_from_clients(std::string info_command);
	void start_message_listener();
	std::vector<std::string> get_cn_list();
protected:
private:
	/*! \brief Container for incoming message from a client.
	 *
	 */
	class client_message
	{
	public:
		client_message() : client_(NULL), message_("") {}
		client_message(wclient* c, std::string s) : client_(c), message_(s) {};
		wclient *get_client() { return client_; } //!< Client getter
		std::string message() { return message_; } //!< Message getter
	private:
		wclient *client_; //!< Pointer to client
		std::string message_; //!< The message
	};
	wclient *find_client(tcpsocket *);
	std::vector<wclient*> clients; //!< List of all clients
	std::queue<std::string> msgqueue_pm_in; //!< Queue of incoming messages from PM
#	ifdef PLATFORM_LINUX
		pthread_mutex_t mutex_pm_queue; //!< Mutex for PM incoming messages queue
		pthread_mutex_t mutex_client_queue; //!< Mutex for client incoming messages queue
		pthread_cond_t  cond_message_received; //!< Condition variable for when messages are received
		pthread_mutex_t  mutex_message_received; //!< Mutex for condition variable
		pthread_cond_t  cond_main_action; //!< Condition variable for when main thread should take action
		pthread_mutex_t  mutex_main_action; //!< Mutex for condition variable
#	endif
	std::queue<client_message> msgqueue_client_in; //!< Queue of incoming messages from clients
	volatile bool message_listener_running; //!< TRUE if this message listener is running (separate thread)
	volatile bool running; // TRUE as long as this class is in control of the main thread
	volatile bool stop_main_loop; // TRUE when main loop should stop executing
	pthread_t supervisor_thread; //!< Instance of #supervisor thread
	std::string clients_filename_; //!< Filename of textfile containing all client IPs and ports. Default is defined in #DEFAULT_CLIENTLIST_FILENAME.
	websocket *pm_; //!< Pointer to websocket instance for communicating with presentation manager
	std::string pm_url; //!< URL of presentation manager server
	bool received_a_pm_message; //!< TRUE if we've ever recevied any message from PM
	timetools::handle last_received_pm_message; //!< Time since we last recevied a message from PM
	bool sent_a_pm_message; //!< TRUE if we've ever sent any message to PM
	timetools::handle last_sent_pm_message; //!< Time since we last sent a message to PM
	timetools::handle pm_connected_time; //!< Time we've been connected to PM
	void thread_entry();
	bool new_data_from_client; //!< TRUE when any client has sent us a list of client nodes that we haven't yet processed
	bool new_configs; //!< TRUE when any client has sent us a list of configs that we haven't yet processed
	bool new_connections; //!< TRUE when any client has sent us a list of connections that we haven't yet processed
	void send_info_to_pm(wclient *client);
	std::set<struct wclient::client_node, wclient::compare> aggregated_cn_list;
	void add_to_aggregated_list(wclient *client);
	void send_client_nodes_to_pm();
	unsigned previous_client_count;
};

#endif // __AGGIE_HPP
