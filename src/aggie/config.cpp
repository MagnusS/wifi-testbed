//! \file config.cpp
//! \copydoc config.hpp

#define CONFIG_ALLOC
#include "config.hpp"
#undef CONFIG_ALLOC

#include "main.h"
#include "vout.hpp"
#include "stringutils.hpp"

/*! \brief Namespace for configuration value control.
 * \copydetails config.hpp
 */
namespace config
{
	/*! \brief Default values must be set here.
	 * \return Always returns resulthandler::OK
	 */
	RH set_default_values()
	{
		RH result;
		result.set_ok();

		clientlist_filename = DEFAULT_CLIENTLIST_FILENAME;
		supervisor_listening_port = DEFAULT_SUPERVISOR_LISTENING_PORT;
		client_poll_interval_sec = DEFAULT_CLIENT_REPOLL_INTERVALL_SEC;
		return result;
	}

	/*! \brief Parses the command line and sets configuration accordingly.
	 * \param argc Number of command line arguments
	 * \param argv Array of command line arguments
	 * \return Always returns resulthandler::OK
	 */
	RH parse_commandline(int argc, char **argv)
	{
		RH result;
		result.set_ok();

		cmdl.set(argc, argv);
		print_help           = cmdl.add_token_flag  ("h",  "help", 0, 1, "Print this help");
		display_version      = cmdl.add_token_flag  ("V",  "version", 0, 1, "Display version");
		verbosity_level      = cmdl.add_token_flag  ("v",  "verbose", 0, -1, "Increase verbosity level (use several for higher verbosity)");
		verbosity_level2     = cmdl.add_token_flag  ("v2", "", 0, 1, "Set verbosity level 2", false);
		verbosity_level3     = cmdl.add_token_flag  ("v3", "", 0, 1, "Set verbosity level 3", false);
#ifdef DEBUG
		debug                = cmdl.add_token_flag  ("v4",   "debug", 0, 1, "Shorthand for -v -v -v -v", false);
		debug2               = cmdl.add_token_flag  ("v5",   "debug2", 0, 1, "Shorthand for -v -v -v -v -v", false);
#endif
		quiet_level          = cmdl.add_token_flag  ("q",  "quiet", 0, -1, "Increase quiet level (used twice will also silence errors)");
		new_clients_filename = cmdl.add_token_string("c",  "clients", 0, 1, format_string("File containing client list - default %s", DEFAULT_CLIENTLIST_FILENAME));
		supervisor_port      = cmdl.add_token_uint  ("l",  "listen-port", 0, 1, format_string("Supervisor listening port - default %u", DEFAULT_SUPERVISOR_LISTENING_PORT));
		client_poll_interval = cmdl.add_token_uint  ("p",  "poll-interval", 0, 1, format_string("Interval (in seconds) between repolling of clients (0 means no repolling) - default %d", DEFAULT_CLIENT_REPOLL_INTERVALL_SEC));

		print_help->set_callback(&printhelp);
		display_version->set_callback(&displayversion);
		cmdl.parse();

		// Adjust verbosity level
		verbosity += verbosity_level->count();
		verbosity -= quiet_level->count();
		if (verbosity > VOUT_MAX) verbosity = VOUT_MAX;
		if (verbosity < 0) verbosity = 0;

		if (verbosity_level2->is_set()) verbosity = VOUT_VERBOSER;
		if (verbosity_level3->is_set()) verbosity = VOUT_VERBOSEST;
#ifdef DEBUG
		if (debug->is_set())			verbosity = VOUT_DEBUG;
		if (debug2->is_set())			verbosity = VOUT_DEBUG2;
#endif

		// Read presentation manager from command line
		std::vector<std::string> loose_args = cmdl.main_arguments();
		if (loose_args.size() != 1)
			printhelp(&cmdl);
		else
			presentation_manager = loose_args[0];

		// Read filename for clients list if present
		if (new_clients_filename->count() == 1)
		{
			clientlist_filename = new_clients_filename->value();
		}

		if (supervisor_port->count() == 1)
		{
			supervisor_listening_port = supervisor_port->value();
		}

		if (client_poll_interval->count() == 1)
		{
			client_poll_interval_sec = client_poll_interval->value();
		}
		return(result);
	}

	/*! \brief Reads configuration values from file.
	 *
	 * \todo Make it actually read configuration from a file.
	 * \return Always returns resulthandler::OK
	 */
	RH read_configfile()
	{
		RH result;
		result.set_ok();

		// Insert code to read configuration file here

		return(result);
	}

	/*! \brief Performs configuration setting in the proper order.
	 * \param argc Number of command line arguments
	 * \param argv Array of command line arguments
	 * \return Always returns resulthandler::OK
	 */
	RH apply(int argc, char **argv)
	{
		RH result;
		result.set_ok();

		if (result.is_ok()) result = set_default_values();
		if (result.is_ok()) result = read_configfile();
		if (result.is_ok()) result = parse_commandline(argc, argv);

		return result;
	}

	/*! \brief Clean up memory when configuration is no longer needed.
	 * \return Always returns resulthandler::OK
	 */
	RH cleanup()
	{
		RH result;
		result.set_ok();

		cmdl.destroy();

		return result;
	}
}

