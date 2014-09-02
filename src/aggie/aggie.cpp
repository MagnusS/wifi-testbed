/*! \file aggie.cpp
 *  \copydoc aggie.hpp
 */

#include "aggie.hpp"
#include "main.h"
#include "stringutils.hpp"
#include "config.hpp"

#include "json/json.h"

#include <fstream>
#include <sstream>
#include <algorithm>

/*! \brief Constructor
 *
 */
aggie::aggie()
	: message_listener_running(false),
	  supervisor_thread(0),
	  clients_filename_(""),
	  pm_(NULL),
	  pm_url(""),
	  received_a_pm_message(false),
	  last_received_pm_message(0),
	  sent_a_pm_message(false),
	  last_sent_pm_message(0),
	  pm_connected_time(0),
	  running(false),
	  stop_main_loop(false),
	  new_data_from_client(false),
	  new_configs(false),
	  new_connections(false),
	  previous_client_count(0)
{
	last_received_pm_message = timers.add_stopwatch();
	last_sent_pm_message = timers.add_stopwatch();
	pm_connected_time = timers.add_stopwatch();
	::pthread_mutex_init(&mutex_pm_queue, NULL);
	::pthread_mutex_init(&mutex_client_queue, NULL);
	::pthread_cond_init(&cond_message_received, NULL);
	::pthread_mutex_init(&mutex_message_received, NULL);
	::pthread_cond_init(&cond_main_action, NULL);
	::pthread_mutex_init(&mutex_main_action, NULL);
}

/*! \brief Controlled termination of the class.
 * \return Always returns #resulthandler::OK
 */
RH aggie::shutdown()
{
	RH result;
	result.set_ok();

	disconnect_pm();
	disconnect_clients();
	delete_clients();

	::pthread_mutex_destroy(&mutex_pm_queue);
	::pthread_mutex_destroy(&mutex_client_queue);
	::pthread_cond_destroy(&cond_message_received);
	::pthread_mutex_destroy(&mutex_message_received);
	return(result);
}

/*! \brief Reads list of clients from file and adds them to the system.
 *
 * The file must contain one client entry per line, with each entry
 * given as "IP-or-hostname port", e.g. "192.168.3.44 4002"
 * (without the quotes).
 *
 * \param clients_filename Filename of textfile
 * \return Always returns #resulthandler::OK
 */
RH aggie::add_clients(std::string clients_filename)
{
	RH result;
	result.set_ok();

	clients_filename_ = clients_filename;

	vout(VOUT_DEBUG) << "Reading clients from file \"" << clients_filename << "\"" << std::endlc;
	vout(VOUT_DEBUG) << " - ";

	std::ifstream file(clients_filename.c_str());
    std::string str;
    while (std::getline(file, str))
    {
    	trim(str);
    	if ((str.length() > 0) && (str.substr(0,1) != "#"))
    	{
			if (clients.size() > 0)
			{
				vout(VOUT_DEBUG) << ", ";
			}
			wclient *c = new wclient(str);
			clients.push_back(c);
			vout(VOUT_DEBUG) << c->host() << ":" << c->port();
    	}
    }
    if (clients.size() == 0)
    {
    	vout(VOUT_DEBUG) << "none found";
    }
   	vout(VOUT_DEBUG) << std::endlc;

   	file.close();

	return(result);
}

/*! \brief Reads list of clients from file and adds them to the system.
 *
 * The file must contain one client entry per line, with each entry
 * given as "IP-or-hostname port", e.g. "192.168.3.44 4002"
 * (without the quotes).
 *
 * \return Always returns #resulthandler::OK
 */
RH aggie::add_clients()
{
	return(add_clients(clients_filename_));
}

/*! \brief Removes all clients from memory.
 *
 * \return Always returns #resulthandler::OK
 */
RH aggie::delete_clients()
{
	RH result;
	result.set_ok();

	std::vector<wclient*>::iterator clients_itr = clients.begin();
	while (clients_itr != clients.end())
	{
		vout(VOUT_DEBUG) << "Deleting client " << (*clients_itr)->text() << std::endlc;
		delete (*clients_itr)->socket;
		delete *clients_itr;
		clients.erase(clients_itr);
	}

	return(result);
}

/*! \brief Tries to connect to all clients.
 *
 * #add_clients() must have been called first to load list of clients
 * into memory.
 *
 * \return #resulthandler::OK, or #resulthandler::NOT_OK if no clients were connected
 */
RH aggie::connect_clients()
{
	RH result;
	result.set_ok();

	unsigned connected_clients = 0;

	std::vector<wclient*>::iterator clients_itr = clients.begin();
	while (clients_itr != clients.end())
	{
		(*clients_itr)->socket->set_destination((*clients_itr)->host());
		(*clients_itr)->socket->set_port((*clients_itr)->port());
		vout(VOUT_INFO) << "Connecting to client " << (*clients_itr)->text() << " ... ";
		vout(VOUT_INFO).flush();
		(*clients_itr)->socket->connect();
		if ((*clients_itr)->socket->connected())
		{
			vout(VOUT_INFO) << "OK" << std::endlc;
			(*clients_itr)->socket->start_tcpsocket_client_listener(::client_listener);
			connected_clients += 1;
		}
		else
		{
			vout(VOUT_INFO) << ansi::red << "Failed" << std::endlc;
		}
		clients_itr += 1;
	}

	if (connected_clients == 0)
	{
		result.set_not_ok();
	}

	return(result);
}

/*! \brief Disconnects all clients.
 *
 * \return Always returns #resulthandler::OK
 */
RH aggie::disconnect_clients()
{
	RH result;
	result.set_ok();

	std::vector<wclient*>::iterator clients_itr = clients.begin();
	while (clients_itr != clients.end())
	{
		if ((*clients_itr)->socket->connected())
		{
			(*clients_itr)->socket->disconnect();
			if (!(*clients_itr)->socket->connected())
			{
				vout(VOUT_VERBOSE) << "Disconnected client " << (*clients_itr)->text() << std::endlc;
			}
			else
			{
				vout(VOUT_VERBOSE) << ansi::red << "Could not disconnect client " << (*clients_itr)->text() << std::endlc;
			}
		}
		clients_itr += 1;
	}

	return result;
}

wclient *aggie::find_client(tcpsocket *socket)
{
	wclient *found_client = NULL;
	std::vector<wclient*>::iterator clients_itr = clients.begin();
	while (clients_itr != clients.end())
	{
		if ((*clients_itr)->socket == socket)
		{
			found_client = (*clients_itr);
			break;
		}
		clients_itr += 1;
	}

	return(found_client);
}

/*! \brief Message handler for incoming client messages.
 *
 * \param listener Pointer to the tcpsocket instance
 *        message was receive on
 * \param data Incoming message
 */
void aggie::client_listener(tcpsocket *listener, std::string data)
{
	wclient *c = find_client(listener);
	if (c == NULL)
	{
		// We've received a client message from a client we have
		// no knowledge of. Should be impossible, but the impossible
		// happens way more often than you'd think...
		vout(VOUT_ERROR) << "Received message from unknown client: " << data << std::endlc;
	}
	else
	{
		c->received_message = true;
		//std::cout << "stopwatch " << c->last_received_message << ": pre-restart: " << timers.get_stopwatch_elapsed_time_in_ms(c->last_received_message);
		timers.restart_stopwatch(c->last_received_message);
		//std::cout << " post-restart: " << timers.get_stopwatch_elapsed_time_in_ms(c->last_received_message) << std::endl;
		if (message_listener_running)
		{
			::pthread_mutex_lock(&mutex_client_queue);
			msgqueue_client_in.push(client_message(c, data));
			::pthread_mutex_unlock(&mutex_client_queue);
			::pthread_cond_signal(&cond_message_received);
		}
		vout(VOUT_VERBOSEST) << ansi::cyan << "-> from client " << c->text() << ": " << data << std::endlc;
	}
}


/*! \brief Message handler for incoming PM-messages.
 *
 * \param listener Pointer to the websocket instance that communicates
 *                 with Presentation Manager
 * \param data Incoming message
 */
void aggie::pm_listener(websocket *listener, std::string data)
{
	received_a_pm_message = true;
	timers.restart_stopwatch(last_received_pm_message);
	::pthread_mutex_lock(&mutex_pm_queue);
	msgqueue_pm_in.push(data);
	::pthread_mutex_unlock(&mutex_pm_queue);
	::pthread_cond_signal(&cond_message_received);
	vout(VOUT_VERBOSEST) << ansi::cyan << "-> from PM: " << data << std::endlc;
}

/*! \brief Starts the websocket listener that dispatches incoming PM-messages.
 *
 * \param callback Pointer to the message handler function.
 * \return Always returns OK
 */
RH aggie::start_pm_listener(websocket::websocket_callback callback)
{
	return pm_->start_websocket_client_listener(callback);
}

/*! \brief Establishes a websocket connection to the presentation manager.
 * \param url URL of the PM.
 * \return #resulthandler::OK or #AGGIE_ERROR_INVALID_PM_URL.
 */
RH aggie::connect_pm(std::string url)
{
	RH result;
	result.set_ok();

	char host[128];
	int port;
	char path[128];

	if (url.length() == 0)
	{
		// Use previous specified url
		url = pm_url;
	}
	if (url.substr(0, 5) != "ws://")
	{
		url = "ws://" + url;
	}

	if (url.size() >= 128)
	{
		result.set_not_ok(AGGIE_ERROR_INVALID_PM_URL);
		return(result);
	}

	else if (sscanf(url.c_str(), "ws://%[^:/]:%d/%s", host, &port, path) == 3)
	{
	}
	else if (sscanf(url.c_str(), "ws://%[^:/]/%s", host, path) == 2)
	{
		port = 80;
	}
	else if (sscanf(url.c_str(), "ws://%[^:/]:%d", host, &port) == 2)
	{
		path[0] = '\0';
	}
	else if (sscanf(url.c_str(), "ws://%[^:/]", host) == 1)
	{
		port = 80;
		path[0] = '\0';
	}
	else
	{
		result.set_not_ok(AGGIE_ERROR_INVALID_PM_URL);
		return(result);
	}

	result = connect_pm(host, port, path);

	return(result);
}

/*! \brief Establishes a websocket connection to the presentation manager.
 * \param destination IP or hostname of PM
 * \param port Port number to connect to
 * \param path The path-part of the remote url
 * \return #resulthandler::OK or #resulthandler::NOT_OK
 */
RH aggie::connect_pm(std::string destination, unsigned port, std::string path)
{
	RH result;
	result.set_ok();

	disconnect_pm();

	if (path.length() != 0)
	{
		path = "/" + path;
	}

	pm_url = "ws://" + destination + ":" + int_to_string(port) + path;
	pm_ = new websocket(destination, port, path);
	result = pm_->connect();
	if (result.is_ok())
	{
		timers.restart_stopwatch(pm_connected_time);
		vout(VOUT_INFO) << "Connected to presentation manager " << pm_->url() << std::endlc;
	}
	else
	{
		pm_connected_time = 0;
		result.set_exitstatus(format_string("Failed to connect to presentation manager %s", pm_->url().c_str()));
	}

	return(result);
}

/*! \brief Disconnects from the presentation manager.
 * \return Always returns #resulthandler::OK
 */
RH aggie::disconnect_pm()
{
	RH result;
	result.set_ok();

	if (pm_ != NULL)
	{
		if (pm_->connected())
		{
			pm_->stop_websocket_client_listener();
			vout(VOUT_INFO) << "Disconnected from presentation manager " << pm_url << std::endlc;
		}
		pm_->disconnect();
		delete pm_;
		pm_ = NULL;
	}

	return(result);
}

/*! \brief Sends a message to the presentation manager.
 * \param data Message to send
 * \return resulthandler::OK if all is OK, otherwise either
 *         #SOCKET_ERROR_NOT_CONNECTED or #SOCKET_ERROR.

 */
RH aggie::send_pm(std::string data)
{
	sent_a_pm_message = true;
	timers.restart_stopwatch(last_sent_pm_message);
	return pm_->send(data);
}

/*! \brief Processes incoming messages from clients.
 *
 * Runs in it's own thread, and sleeps until the #client_listener wakes it up.
 */
void aggie::thread_entry()
{
	int condwait;

	message_listener_running = true;
	std::string current_dataset = "";

	// Dispatch loop
	while (message_listener_running)
	{
		condwait = pthread_cond_wait(&cond_message_received, &mutex_message_received);
		if (condwait == 0)
		{
			if (::pthread_mutex_trylock(&mutex_pm_queue) == 0)
			{
				// We have incoming PM messages. Deal with them.
				std::string msg = "";
				while (!msgqueue_pm_in.empty())
				{
					msg = msgqueue_pm_in.front();
					vout(VOUT_VERBOSE) << "From PM: " << msg << std::endlc;
					msgqueue_pm_in.pop();
				}
				::pthread_mutex_unlock(&mutex_pm_queue);
			}
			if (::pthread_mutex_trylock(&mutex_client_queue) == 0)
			{
				// We have incoming client messages. Deal with them.
				while (!msgqueue_client_in.empty())
				{
					client_message c = msgqueue_client_in.front();
					msgqueue_client_in.pop();
					wclient *client = c.get_client();
					if (client == NULL)
					{
						vout(VOUT_ERROR) << "Invalid client" << std::endlc;
						continue;
					}
//					vout(VOUT_VERBOSER) << "From client " << c.get_client()->text() <<  ": " << c.message() << std::endlc;
					std::string msg = c.message();
					std::replace(msg.begin(), msg.end(), '\t', ' '); // replace \t with space for easier parsing
					std::istringstream iss (msg);
					std::string token;
					iss >> token;
					int msg_id;
					string_to_int(token, msg_id);
//std::cout << "token = " << token << "    msg_id = " << msg_id << std::endl;
					if (msg_id == IPCSERVER_REPLY_BUSY)
					{
//						client->socket->disconnect();
						vout(VOUT_VERBOSE) << "Client " << client->host_and_port() << " is busy. Disconnecting." << std::endlc;
					}
					else if (msg_id == IPCSERVER_REPLY_HELP)
					{
						// Extract data columns
						client->data_column.clear(); // Erase whatever columns we had before
						while (!iss.eof())
						{
							iss >> token;
							client->data_column.push_back(token);
						}
					}
					else if (msg_id == IPCSERVER_REPLY_COMMAND_OUTPUT)
					{
						::pthread_mutex_lock(&client->request_command_mutex);
						if (client->request_command.size() > 0)
						{
							current_dataset = client->request_command.front();
						}
						::pthread_mutex_unlock(&client->request_command_mutex);
						struct wclient::client_node cn;
						struct wclient::configuration config;
						struct wclient::connection connection;
						int column_index = 0;
						while (!iss.eof())
						{
							iss >> token;
							std::string field = client->data_column[column_index];
//std::cout << ansi::green << "  " << "field = " << field<< std::endlc;
//std::cout << ansi::green << "  " << "token = " << token<< std::endlc;
							if (current_dataset == "list cn")
							{
								if (client->client_nodes_list_finished)
								{
									vout(VOUT_DEBUG2) << "Clearing client node list from client " << client->host_and_port() << std::endlc;
									client->client_nodes.clear();
									client->client_nodes_list_finished = false;
								}
								if (field == "ID") string_to_unsigned(token, cn.id);
								if (field == "AGE") string_to_unsigned(token, cn.age);
								if (field == "CR") string_to_unsigned(token, cn.cr);
								if (field == "LAT") string_to_double(token, cn.lat);
								if (field == "LON") string_to_double(token, cn.lon);
								if (field == "P2P_IP") cn.p2p_ip.set_host_and_port(token);
								if (field == "RADAC_IP") cn.radac_ip.set_host_and_port(token);
							}
							if (current_dataset == "list connections")
							{
								if (client->connection_list_finished)
								{
									vout(VOUT_DEBUG2) << "Clearing connection list from client " << client->host_and_port() << std::endlc;
									client->connections.clear();
									client->connection_list_finished = false;
								}
								if (field == "DIR") connection.dir = token;
								if (field == "PEER_ID") string_to_unsigned(token, connection.peer_id);
								if (field == "PEER_IP") connection.peer_ip.set_host_and_port(token);
							}
							if (current_dataset == "list configs")
							{
								if (client->config_list_finished)
								{
									vout(VOUT_DEBUG2) << "Clearing configuration list from client " << client->host_and_port() << std::endlc;
									client->configs.clear();
									client->config_list_finished = false;
								}
								if (field == "ID") string_to_unsigned(token, config.id);
								if (field == "AGE") string_to_unsigned(token, config.age);
								if (field == "SRC_IP") config.src_ip.set_host_and_port(token);
								if (field == "CONFIG") config.config = token;
							}
							column_index += 1;
						}
						// Complete line has been read; now store it:
						if (current_dataset == "list cn")
						{
							client->client_nodes.push_back(cn);
							vout(VOUT_DEBUG2) << client->host_and_port() << ": Added new client_node entry:" << std::endlc;
							vout(VOUT_DEBUG2) << " - ID       = " << cn.id << std::endlc;
							vout(VOUT_DEBUG2) << " - AGE      = " << cn.age << std::endlc;
							vout(VOUT_DEBUG2) << " - CR       = " << cn.cr << std::endlc;
							vout(VOUT_DEBUG2) << " - LAT      = " << cn.lat << std::endlc;
							vout(VOUT_DEBUG2) << " - LON      = " << cn.lon << std::endlc;
							vout(VOUT_DEBUG2) << " - P2P_IP   = " << cn.p2p_ip.host_and_port() << std::endlc;
							vout(VOUT_DEBUG2) << " - RADAC_IP = " << cn.radac_ip.host_and_port() << std::endlc;
							vout(VOUT_DEBUG2) << " New client_node count = " << client->client_nodes.size() << std::endlc;
						}
						if (current_dataset == "list connections")
						{
							client->connections.push_back(connection);
							vout(VOUT_DEBUG2) << client->host_and_port() << ": Added new connection entry:" << std::endlc;
							vout(VOUT_DEBUG2) << " - DIR      = " << connection.dir << std::endlc;
							vout(VOUT_DEBUG2) << " - PEER_ID  = " << connection.peer_id << std::endlc;
							vout(VOUT_DEBUG2) << " - PEER_IP  = " << connection.peer_ip.host_and_port() << std::endlc;
							vout(VOUT_DEBUG2) << " New connection count = " << client->connections.size() << std::endlc;
						}
						if (current_dataset == "list configs")
						{
							client->configs.push_back(config);
							vout(VOUT_DEBUG2) << client->host_and_port() << ": Added new configuration entry:" << std::endlc;
							vout(VOUT_DEBUG2) << " - ID       = " << config.id << std::endlc;
							vout(VOUT_DEBUG2) << " - AGE      = " << config.age << std::endlc;
							vout(VOUT_DEBUG2) << " - SRC_IP   = " << config.src_ip.host_and_port() << std::endlc;
							vout(VOUT_DEBUG2) << " - CONFIG   = " << config.config << std::endlc;
							vout(VOUT_DEBUG2) << " New configuration count = " << client->configs.size() << std::endlc;
						}
					}
					if (msg_id == IPCSERVER_REPLY_READY)
					{
						// Finished current data set
						vout(VOUT_DEBUG2) << "Finished current current_dataset \"" << current_dataset << "\" from client " << client->host_and_port() << std::endlc;

						::pthread_mutex_lock(&client->request_command_mutex);
						if (client->request_command.size() > 0)
						{
//std::cout << ansi::green << "  " << "current_dataset = " << current_dataset << std::endlc;
//std::cout << "About to pop..." << std::endl;
							client->request_command.pop();
//std::cout << "Popped" << std::endl;
						}
						::pthread_mutex_unlock(&client->request_command_mutex);
						if (current_dataset == "list cn")
						{
							client->client_nodes_list_finished = true;
							client->data_changed = true;
							new_data_from_client = true;
							vout(VOUT_DEBUG2) << "Received all client nodes from client " << client->host_and_port() << std::endlc;
						}
						if (current_dataset == "list connections")
						{
							client->connection_list_finished = true;
							client->data_changed = true;
							new_data_from_client = true;
							vout(VOUT_DEBUG2) << "Received all connections from client " << client->host_and_port() << std::endlc;
						}
						if (current_dataset == "list configs")
						{
							client->config_list_finished = true;
							client->data_changed = true;
							new_data_from_client = true;
							vout(VOUT_DEBUG2) << "Received all configs from client " << client->host_and_port() << std::endlc;
						}
//						std::cout << "Received 200" << std::endlc;
//std::cout << "[3] request_command size = " << client->request_command.size() << std::endlc;
					}

				}
				::pthread_mutex_unlock(&mutex_client_queue);
			}
		}
	}
	vout(VOUT_DEBUG) << "[aggie] exiting listener thread" << std::endlc;
}

/*! \brief Starts the message listener \ref thread_entry() "thread" that
 * processes incoming messages from clients.
 */
void aggie::start_message_listener()
{
	run();
	while (!message_listener_running); // Wait til message listener thread has been started properly
	vout(VOUT_DEBUG) << "[aggie] listener thread started" << std::endlc;

}

/*! \brief Requests information from clients.
 *
 * \param info_command Request \ref CLIENT_COMMANDS "command string" to send to the clients
 *
 */
void aggie::get_info_from_clients(std::string info_command)
{
	std::vector<wclient*>::iterator itr = clients.begin();
	while (itr != clients.end())
	{
		// How long to wait before discarding a previous request:
		unsigned timeout = (config::client_poll_interval_sec * 1000) / 2; // Always discard before new poll
		if (timeout == 0) timeout = 1000;  // One second by default
		if (timers.get_stopwatch_elapsed_time_in_ms((*itr)->last_received_message) > timeout)
		{
			if ((*itr)->request_command.size() > 0)
			{
				(*itr)->request_command.pop();
				vout(VOUT_DEBUG) << "Too long since last reply from " << (*itr)->host_and_port() << " - removing last request. request queue size is now " << (*itr)->request_command.size() << std::endlc;
			}
		}
		if (((*itr)->socket->connected()) && ((*itr)->send_command(info_command).is_not_ok()))
		{
			vout(VOUT_ERROR) << "Error in connection to " << (*itr)->host_and_port() << " - forcing disconnect" << std::endlc;
			(*itr)->socket->disconnect();
		}
		sleep_ms(50);
		itr += 1;
	}
}

void aggie::get_info_from_clients()
{
	if (clients.size() > 0)
	{
		vout(VOUT_VERBOSER) << "Polling clients" << std::endlc;
		get_info_from_clients(GET_CLIENT_NODES);
		get_info_from_clients(GET_CONFIGS);
		get_info_from_clients(GET_CONNECTIONS);
	}
}

void aggie::send_info_to_pm(wclient *client)
{
	Json::Value root;
	std::string host = client->host();
	std::string port = client->port();
	std::string cn("cn");
	std::vector<struct wclient::client_node>::iterator itr = client->client_nodes.begin();
	while(itr != client->client_nodes.end())
	{
		std::string id = int_to_string(itr->id);
		root[host][port][cn][id]["age"]      = itr->age;
		root[host][port][cn][id]["cr"]       = itr->cr;
		root[host][port][cn][id]["lat"]      = itr->lat;
		root[host][port][cn][id]["lon"]      = itr->lon;
		root[host][port][cn][id]["p2pip"]    = itr->p2p_ip.host_and_port();
		root[host][port][cn][id]["radac_ip"] = itr->radac_ip.host_and_port();
		itr += 1;
	}
	send_pm(root.toStyledString());
}

void aggie::send_client_nodes_to_pm()
{
	Json::Value root;
	Json::Value data;

	std::set<struct wclient::client_node, wclient::compare>::iterator itr = aggregated_cn_list.begin();
	while(itr != aggregated_cn_list.end())
	{
		Json::Value entry;
		std::string lat;
		std::string lon;
		double_to_string(itr->lat, lat);
		double_to_string(itr->lon, lon);
		entry["unitId"]     = itr->id;
		entry["unitPos"]    = lat + " " + lon;
		entry["unitSymbol"] = "SFGPICU---Exxx";
		entry["unitEnum"]   = "";
		entry["unitAlt"]    = 0.0;
		entry["unitSpeed"]  = 0.0;
		data.append(entry);
		itr++;
	}
	root["data"] = data;
	vout(VOUT_DEBUG) << "Sending client nodes to PM:" << std::endl << root.toStyledString() << std::endlc;
	send_pm(root.toStyledString());
}

void aggie::add_to_aggregated_list(wclient *client)
{
	std::vector<struct wclient::client_node>::iterator itr = client->client_nodes.begin();
	while(itr != client->client_nodes.end())
	{
		aggregated_cn_list.insert(*itr);
		itr += 1;
	}
}

std::vector<std::string> aggie::get_cn_list()
{
	std::vector<std::string> cn_list;

	std::set<struct wclient::client_node, wclient::compare>::iterator itr = aggregated_cn_list.begin();
	while(itr != aggregated_cn_list.end())
	{
		std::stringstream cn;
		cn << "ID: " << itr->id << " age=" << itr->age << " cr=" << itr->cr << " lat/lon=" << itr->lat << "/" << itr->lon << " p2p-ip=" << itr->p2p_ip.host_and_port() << " radac-ip=" << itr->radac_ip.host_and_port();
		cn_list.push_back(cn.str());
		itr++;
	}

	return(cn_list);
}

/*! \brief Main application loop.
 *
 * This is where all it all happens after everything has been set up.
 *
 *
 * \return Always returns resulthandler::OK
 */
RH aggie::start()
{
	RH result;
	result.set_ok();

	int poll_intervall_counter = config::client_poll_interval_sec * 1000;

	sleep_ms(CLIENTS_CN_POLL_INITIAL_DELAY_MS);
	get_info_from_clients();

	running = true;
	while (!stop_main_loop)
	{
		if (new_data_from_client)
		{
			// New list of client nodes has been received from one or more of the clients.
			// This information must be aggregated and transmitted to the PM.
			aggregated_cn_list.clear();
			std::vector<wclient*>::iterator clients_itr = clients.begin();
			while (clients_itr != clients.end())
			{
//				if ((*clients_itr)->data_changed)
//				{
					//send_info_to_pm(*clients_itr);
					add_to_aggregated_list(*clients_itr);
					(*clients_itr)->data_changed= false;
//				}
				clients_itr += 1;
			}
			new_data_from_client = false;
			if (previous_client_count != aggregated_cn_list.size())
			{
				vout(VOUT_INFO) << "Total client count: " << aggregated_cn_list.size() << std::endlc;
				previous_client_count = aggregated_cn_list.size();
			}
			send_client_nodes_to_pm();
		}

		unsigned long sleep_delay_ms = 500;
		sleep_ms(sleep_delay_ms);

		if (config::client_poll_interval_sec > 0)
		{
			// Perform repolling of clients at specified interval if requested on command line
			if (poll_intervall_counter <= 0)
			{
				vout(VOUT_VERBOSEST) << "Repolling clients after " << config::client_poll_interval_sec << " second" << (config::client_poll_interval_sec > 1 ? "s" : "") << std::endlc;
				get_info_from_clients(GET_CLIENT_NODES);
				get_info_from_clients(GET_CONFIGS);
				get_info_from_clients(GET_CONNECTIONS);
				poll_intervall_counter = config::client_poll_interval_sec * 1000;
			}
			poll_intervall_counter -= sleep_delay_ms;
		}

	}

	vout(VOUT_DEBUG) << "[aggie] leaving main thread loop" << std::endlc;

	running = false;
	return(result);
}

/*! \brief Stops the main application loop.
 *
 * \return Always returns resulthandler::OK
 */
RH aggie::stop()
{
	RH result;
	result.set_ok();

	if (running || message_listener_running)
	{
		vout(VOUT_DEBUG) << "Stopping aggie" << std::endlc;

		if (message_listener_running)
		{
			message_listener_running = false;
			::pthread_cond_signal(&cond_message_received); // Must "fake" this to wake the thread
			wait();
		}
		if (running)
		{
			stop_main_loop = true;
//		while (running); // Wait for main loop top finish
		}
	}

	return(result);
}

/*!\brief Returns an array of strings with current status.
 *
 * \return Status strings
 */
std::vector<std::string> aggie::status()
{
	std::vector<std::string> status;
	if ((pm_ == NULL) || (!pm_->connected()))
	{
		status.push_back("Not connected to PM");
	}
	else
	{
		status.push_back(format_string("Connected to PM @ %s for %ld seconds", pm_->url().c_str(),
		                 timers.get_stopwatch_elapsed_time_in_ms(pm_connected_time).value() / 1000));
	}
	status.push_back(format_string("Last message sent to PM: %s%s",
	                 (sent_a_pm_message ? int_to_string(timers.get_stopwatch_elapsed_time_in_ms(last_sent_pm_message).value() / 1000).c_str() : "never"),
                     (sent_a_pm_message ? " seconds ago" : "")));
	status.push_back(format_string("Last message received from PM: %s%s",
	                 (received_a_pm_message ? int_to_string(timers.get_stopwatch_elapsed_time_in_ms(last_received_pm_message).value() / 1000).c_str() : "never"),
	                 (received_a_pm_message ? " seconds ago" : "")));

	int clients_connected = 0;
	std::vector<wclient*>::iterator clients_itr = clients.begin();
	while (clients_itr != clients.end())
	{
		if ((*clients_itr)->socket->connected())
		{
			clients_connected += 1;
		}
		clients_itr += 1;
	}
	status.push_back(format_string("Clients connected: %d of %d", clients_connected, clients.size()));
	return(status);
}

/*!\brief Returns an array of strings with current status of specified client.
 *
 * \return Status strings
 */
std::vector<std::string> aggie::client_status(wclient *c)
{
//	std::cout << "stopwatch " << c->last_received_message << ": " << timers.get_stopwatch_elapsed_time_in_ms(c->last_received_message) << std::endl;
	std::vector<std::string> status;
	status.push_back(format_string("Client %s:%s is %sconnected", c->host().c_str(), c->port().c_str(),
	                 (c->socket->connected() ? "" : "not ")));
	status.push_back(format_string(" - Last message sent: %s%s",
	                 (c->sent_message ? int_to_string(timers.get_stopwatch_elapsed_time_in_ms(c->last_sent_message).value() / 1000).c_str() : "never"),
                     (c->sent_message ? " seconds ago" : "")));
	status.push_back(format_string(" - Last message received: %s%s",
	                 (c->received_message ? int_to_string(timers.get_stopwatch_elapsed_time_in_ms(c->last_received_message).value() / 1000).c_str() : "never"),
	                 (c->received_message ? " seconds ago" : "")));

	return(status);
}

/*!\brief Returns an array of strings with current status of all clients.
 *
 * \return Status strings
 */
std::vector<std::string> aggie::client_status()
{
	std::vector<std::string> status;
	std::vector<wclient*>::iterator clients_itr = clients.begin();
	while (clients_itr != clients.end())
	{
		std::vector<std::string> c_status;
		c_status = client_status(*clients_itr);
		status.insert(status.begin(), c_status.begin(), c_status.end()); // Much unnecessary copying here; perhaps there's a better way?
		clients_itr += 1;
	}

	return(status);
}

/*!\brief Returns an array of strings with current status of specified client.
 *
 * \return Status strings
 */
std::vector<std::string> aggie::client_status(std::string host, std::string port)
{
	wclient *c = NULL;
	std::vector<wclient*>::iterator clients_itr = clients.begin();
	while (clients_itr != clients.end())
	{
		if (((*clients_itr)->host() == host) && ((*clients_itr)->port() == port))
		{
			c = *clients_itr;
			break;
		}
		clients_itr += 1;
	}
	std::vector<std::string> status;
	if (c == NULL)
	{
		status.push_back(format_string("Client %s:%s not found", host.c_str(), port.c_str()));
	}
	else
	{
		status = client_status(c);
	}

	return(status);
}



