/*! \file main.cpp
 *
 *  \brief Main file.
 *
 *  Contains all global functions and variables.
 *
 * \date 2013
 * \author   Forsvarets Forskningsinstitutt (FFI)\n
 *           http://www.ffi.no \n
 *           - Terje Mikal Mjelde [tmo]\n
 *             terje-mikal-mjelde@ffi.no
 */




#include "platform.h"
#include "main.h"
#include "aggie.hpp"
#include "config.hpp"
#include "color_streams.h"
#include "ipsocket.hpp"
#include "messagelist.hpp"
#include "resulthandler.hpp"
#include "stringutils.hpp"
#include "timetools.hpp"
#include "vout.hpp"

#include <ctime>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sstream>

#ifdef PLATFORM_WINDOWS
#	include <windows.h>
#endif
#ifdef PLATFORM_LINUX
#	include <sys/time.h>
#	include <unistd.h>
#	include <string.h>
#	include <errno.h>
#	include <signal.h>
#endif

aggie *agg = NULL;
timetools timers;
timetools::handle uptime;

bool ctrlc_pressed_before = false;

/*! \brief Pointer to instance of supervisor class.
 *
 * The supervisor is a telnet server running inside Aggie
 * which can be used for statuschecking, controlling and
 * debugging. Default listening port is specified in
 * \ref DEFAULT_SUPERVISOR_LISTENING_PORT and can be
 * specified on the command line when starting Aggie
 * with the `--listen-port` parameter.
 */
telnetserver *supervisor = NULL;

void sleep_ms(unsigned long ms)
{
	struct timespec req = {0};
	req.tv_sec = ms / 1000;
	req.tv_nsec = (ms % 1000) * 1000000L;
	nanosleep(&req, (struct timespec *)NULL);
}

/*! \brief Displays the version number and copyright info in response to `--version`.
 *
 * \param cmdl Pointer to instantiated command line object
 */
void displayversion(cmdline *cmdl)
{
	std::cout << APPNAME << " v" << APPVERSION << " " << APPDATE;
	std::cout << " - " << APPOWNER << std::endl;
	exit(0);
}

/*! \brief Displays command line usage in response to `--help`.
 *
 * \param cmdl Pointer to instantiated command line object
 */
void printhelp(cmdline *cmdl)
{
	std::cout << APPNAME << " v" << APPVERSION << " " << APPDATE;
	std::cout << " - " << APPOWNER << std::endl;
	std::cout << std::endl;
	std::cout << "Usage: " << APPNAME << " [options] WS-URL" << std::endl;
	std::cout << std::endl;
	std::cout << "  WS-URL is the websocket URL to the presentation manager (ws://host:port/path)" << std::endl;
	std::cout << std::endl;
	std::cout << "Available options are:" << std::endl;
	cmdl->print_usage();
	std::cout << std::endl;
	exit(0);
}

/*! \brief Performs a controlled shutdown of all instantiated objects.
 *
 * Called on every type of exit.
 *
 */
void shutdown()
{
	if (supervisor != NULL)
	{
		supervisor->stop_telnet_server();
		delete supervisor;
		supervisor = NULL;
	}
	if (agg != NULL)
	{
		agg->stop();
		agg->shutdown();
		delete agg;
		agg = NULL;
	}
}

#ifdef PLATFORM_LINUX
/*! \brief Signal handler (Linux-only)
 *
 * Responds to CTRL-C, `kill` and `killall` by shutting down
 * the application.
 * \param sig ID of received signal as defined in file signum.h.
 */
static void sigHandler(int sig)
{
	if ((sig == SIGINT) || (sig == SIGTERM))
	{
		/* Received CTRL-C, kill or killall */
		std::cout << std::endl;
		if (ctrlc_pressed_before)
		{
			vout(VOUT_INFO) << ansi::red << "Hard kill" << std::endlc;
			_exit(EXIT_FAILURE);
		}
		vout(VOUT_INFO) << ansi::red << "User abort - If application does not stop, press Ctrl-C again to force hard kill." << std::endlc;
		ctrlc_pressed_before = true;
		agg->stop();
	}
}
#endif

/*! \brief Callback for Presentation Manager.
 *
 * Called whenever we receive a message.
 * \param listener Pointer to socket instance we're using to communicate
 * with the PM
 * \param data Received message
 */
void pm_listener(websocket *listener, std::string data)
{
	agg->pm_listener(listener, data);
}

/*! \brief Callback for Client messages.
 *
 * Called whenever we receive a message from one of the clients.
 * \param listener Pointer to socket instance message is received on
 * \param data Received message
 */
void client_listener(tcpsocket *listener, std::string data)
{
	agg->client_listener(listener, data);
}


/*! \brief Callback function for \ref supervisor.
 *
 * Called whenever a command has been entered via the supervisor.
 *
 * \param server Pointer to the telnet server instance
 * \param socket_handle Handle to the socket that received the command
 *                      (i.e. where to send replies)
 * \param from IP-address of sender
 * \param entry Unparsed command
 */
void supervisor_msghandler(telnetserver *server, int socket_handle, std::string from, std::string entry)
{
	std::istringstream iss(entry);
	std::string command, parameter1, parameter2, parameter3;
	bool valid_command = false;
	bool print_prompt = true;
	iss >> command;
	command = to_lower(command);
	iss >> parameter1;
	parameter1 = to_lower(parameter1);
	iss >> parameter2;
	parameter2 = to_lower(parameter2);
	iss >> parameter3;
	parameter3 = to_lower(parameter3);

	vout(VOUT_VERBOSEST) << ansi::cyan << "-> from " << from << ": " << entry << std::endc;

	if ((command == "help") || (command == "?"))
	{
		server->send(socket_handle, "poll clients              - force repolling of all clients\n");
		server->send(socket_handle, "reload clients            - read client file again\n");
		server->send(socket_handle, "list clients              - show aggregated client list\n");
		server->send(socket_handle, "status                    - display status\n");
		server->send(socket_handle, "status clients            - display status for all clients\n");
		server->send(socket_handle, "status client host port   - display status for specified host\n");
		server->send(socket_handle, "shutdown                  - shutdown aggie (no confirmation)\n");
		server->send(socket_handle, "close                     - close supervisor telnet session\n");
//		server->send(socket_handle, "\n");
//		server->send(socket_handle, "\n");
//		server->send(socket_handle, "\n");
		valid_command = true;
	}
	else if (command == "shutdown")
	{
		vout(VOUT_INFO) << "Shutdown ordered by remote @ " << from << std::endlc;
		agg->stop();
		valid_command = true;
		print_prompt = false;
	}
	else if ((command == "quit") || (command == "close"))
	{
		//vout(VOUT_INFO) << "Shutdown ordered by remote" << std::endlc;
		::close(socket_handle);
		valid_command = true;
		print_prompt = false;
	}
	else if (command == "reload")
	{
		if (parameter1 == "clients")
		{
			agg->disconnect_clients();
			agg->delete_clients();
			agg->add_clients();
			agg->connect_clients();
			valid_command = true;
		}
	}
	else if (command == "pm")
	{
		if (parameter1 == "send")
		{
			agg->send_pm(parameter2);
			valid_command = true;
		}
		if (parameter1 == "disconnect")
		{
			agg->disconnect_pm();
			valid_command = true;
		}
		if (parameter1 == "connect")
		{
			agg->connect_pm(parameter2);
			valid_command = true;
		}
	}
	else if (command == "status")
	{
		std::vector<std::string> status;
		if (parameter1.length() == 0)
		{
			std::vector<std::string> aggie_status;
			status.push_back(format_string("Uptime: %ld seconds", timers.get_stopwatch_elapsed_time_in_ms(uptime).value() / 1000));
			aggie_status = agg->status();
			status.insert(status.end(), aggie_status.begin(), aggie_status.end()); // Not optimal, but good enough for our use
		}
		else if (parameter1 == "clients")
		{
			status = agg->client_status();
		}
		else if (parameter1 == "client")
		{
			if (parameter2.length() == 0)
			{
				status = agg->client_status();
			}
			else if (parameter3.length() == 0)
			{
				status.push_back("Must specify host and port");
			}
			else
			{
				status = agg->client_status(parameter2, parameter3);
			}
		}
		if (!status.empty())
		{
			std::vector<std::string>::iterator status_itr = status.begin();
			while (status_itr != status.end())
			{
				server->send(socket_handle, format_string("%s\n", (*(status_itr++)).c_str()));
			}
			valid_command = true;
		}
	}
	else if (command == "list")
	{
		std::vector<std::string> show;
		if (parameter1 == "clients")
		{
			show = agg->get_cn_list();
		}
		if (!show.empty())
		{
			std::vector<std::string>::iterator status_itr = show.begin();
			while (status_itr != show.end())
			{
				server->send(socket_handle, format_string("%s\n", (*(status_itr++)).c_str()));
			}
			valid_command = true;
		}
	}
	else if (command == "poll")
	{
		if (parameter1 == "clients")
		{
			agg->get_info_from_clients();
		}
	}
	if (!valid_command)
	{
		server->send(socket_handle, "Unknown command. HELP shows available commands.\n");
	}

	if (print_prompt)
	{
		server->send(socket_handle, TELNET_SERVER_PROMPT);
	}

}


/**
 *  Main program.
 *
 *  Responsibilities:
 *  - Set up runtime environment
 *  - Set up communication sockets to
 *    - Presentation manager
 *    - All clients
 *    - Supervisor
 *  - Start the aggregator service
 *
 *  \param argc Number of command line arguments
 *  \param argv Actual arguments
 *
 *  \return EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char **argv)
{
	RH result;
	int exitcode = EXIT_SUCCESS;

	#ifdef PLATFORM_LINUX
	if ((signal(SIGINT, sigHandler) == SIG_ERR)
	 || (signal(SIGTERM, sigHandler) == SIG_ERR))
	{
		vout(VOUT_ERROR) << "Error initializing signal handler" << std::endl;
		return(EXIT_FAILURE);
	}
	#endif

	atexit(shutdown);


	RH apply_config = config::apply(argc, argv);
	if (apply_config.is_not_ok())
	{
		vout(VOUT_ERROR) << "Configuration error: " << apply_config.text() << std::endlc;
		exit(EXIT_FAILURE);
	}

	timers = timetools();
	uptime = timers.add_stopwatch();
	timers.start_stopwatch(uptime);

	agg = new aggie;

	supervisor = new telnetserver();
	supervisor->set_local_port(config::supervisor_listening_port);
	result = supervisor->setup_server(std::string("COGP2P AGGIE Supervisor\n") + TELNET_SERVER_PROMPT);
	if (result.is_not_ok())
	{
		vout(VOUT_ERROR) << result.text() << std::endlc;
	}
	else
	{
		result = supervisor->start_telnet_server(supervisor_msghandler);
		if (result.is_ok())
		{
			vout(VOUT_INFO) << "Supervisor accepting incoming connections on port " << supervisor->listening_port() << std::endlc;
		}
		else
		{
			vout(VOUT_ERROR) << result.text() << std::endlc;
		}
	}


	agg->start_message_listener();

	agg->add_clients(config::clientlist_filename);
	if (agg->connect_clients().is_ok())
	{
		result = agg->connect_pm(config::presentation_manager);
		if (result.is_ok())
		{
			agg->start_pm_listener(pm_listener);
			vout(VOUT_VERBOSE) << "Automatic repolling of clients is " << (config::client_poll_interval_sec == 0 ? "disabled" : format_string("set to %d seconds", config::client_poll_interval_sec)) << std::endlc;
			vout(VOUT_INFO) << "Aggie is running..." << std::endlc;
			agg->start();
			// Program will continue from here when it is shutting down
		}
		else
		{
			vout(VOUT_ERROR) << result.text() << std::endlc;
		}
	}
	else
	{
		vout(VOUT_ERROR) << "Unable to connect to any clients - aborting." << std::endlc;
	}

	shutdown();

	return(exitcode);
}

