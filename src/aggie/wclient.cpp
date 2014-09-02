/*! \file wclient.cpp
 *
 * \copydoc wclient.hpp
 */

#include "wclient.hpp"
#include "stringutils.hpp"
#include "vout.hpp"

#include <algorithm>

ip_address::ip_address()
	: host_(""),
	  port_("")
{

}

ip_address::ip_address(std::string host, std::string port)
{
	set_host(host);
	set_port(port);
}

ip_address::ip_address(std::string host, unsigned port)
{
	set_host(host);
	set_port(port);
}

ip_address::ip_address(std::string host_and_port)
{
	set_host_and_port(host_and_port);
}

void ip_address::set_host(std::string host)
{
	host_ = host;
}

void ip_address::set_port(std::string port)
{
	port_ = port;
	if (port_.length() == 0) port_ = "0";
}

void ip_address::set_port(unsigned port)
{
	port_ = int_to_string(port);
}

void ip_address::set_host_and_port(std::string host_and_port)
{
	std::replace(host_and_port.begin(), host_and_port.end(), ':', ' ');
	std::istringstream iss(host_and_port);
	iss >> host_;
	iss >> port_;
	if (port_.length() == 0) port_ = "0";
}

std::string ip_address::host() const
{
	return host_;
}

std::string ip_address::port() const
{
	return port_;
}

std::string ip_address::host_and_port() const
{
	return host_ + ":" + port_;
}

/*! \brief Constructor specifying new client.
 * \param new_entry Host and port on format "IP port" or "Hostname port"
 */
wclient::wclient(std::string new_entry)
{
	ip.set_host_and_port(new_entry);
	socket = new tcpsocket();
	received_message = false;
	last_received_message = 0;
	last_received_message = timers.add_stopwatch();
	sent_message = false;
	last_sent_message = 0;
	last_sent_message = timers.add_stopwatch();
	::pthread_mutex_init(&request_command_mutex, NULL);
	client_nodes_list_finished = false;
	config_list_finished = false;
	connection_list_finished = false;
	data_changed = false;
}

wclient::~wclient()
{
	::pthread_mutex_destroy(&request_command_mutex);
	timers.delete_stopwatch(last_sent_message);
	timers.delete_stopwatch(last_received_message);
	//if (socket != NULL)	delete socket;
}

//! \brief Host-getter
std::string wclient::host() const
{
	return(ip.host());
}

//! \brief Port-getter
std::string wclient::port() const
{
	return(ip.port());
}

std::string wclient::host_and_port() const
{
	return ip.host_and_port();
}


/*! \brief Returns textual representation of host and port.
 * \return String with hostname and port
 */
std::string wclient::text()
{
	return(host() + std::string(" port ") + port());
}


RH wclient::send_command(std::string command)
{
	RH result;
	result.set_ok();

	vout(VOUT_DEBUG) << "[wclient] sending command to " << ip.host_and_port() << ": " << command << std::endl;
	socket->set_endline("\r\n");
	::pthread_mutex_lock(&request_command_mutex);
	result = socket->sendline(command);
//	std::cout << "!!!!!!!!!!!!!" << (result.is_ok() ? "OK ":"NOT OK ") << result.text() << std::endlc;
	if (result.is_ok())
	{
//		std::cout << "command " << command << " sent succesfully to " << ip.host_and_port() << std::endlc;
		request_command.push(command);
		sent_message = true;
		timers.restart_stopwatch(last_sent_message);
	}
	::pthread_mutex_unlock(&request_command_mutex);

	return(result);
}
