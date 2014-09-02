/*! \file config.hpp
 *
 * \brief Configuration tools.
 *
 * Provides a unified interface for setting configuration values.
 *
 * The proper method for setting values are:
 *
 * -# Set default values
 * -# Read values from a configuration file if present
 * -# Set values that are specified on the command line
 *
 * By doing it in this order we get a logical hierarchy for
 * specifying the configuration.
 *
 * All configuration settings belong to the namespace #config.
 *
 * \todo The configuration settings are not finished. We do not yet support
 *       a configuration file, and there's still far too much labour for
 *       adding new values.
 *
 * \date 2013
 * \author   Forsvarets Forskningsinstitutt (FFI)\n
 *           http://www.ffi.no \n
 *           - Terje Mikal Mjelde [tmo]\n
 *             terje-mikal-mjelde@ffi.no
 */

#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include "cmdline.hpp"
#include "resulthandler.hpp"

#ifdef CONFIG_ALLOC
#	define I(x)      x
#	define EXPORTED  /* empty */
#else
#	define I(x)      /* empty */
#	define EXPORTED  extern
#endif

namespace config
{
	EXPORTED cmdline cmdl;
	EXPORTED cmdline::arg_flag   *print_help;
	EXPORTED cmdline::arg_flag   *display_version;
	EXPORTED cmdline::arg_flag   *verbosity_level;
	EXPORTED cmdline::arg_flag   *verbosity_level2;
	EXPORTED cmdline::arg_flag   *verbosity_level3;
	EXPORTED cmdline::arg_flag   *quiet_level;
#	ifdef DEBUG
		EXPORTED cmdline::arg_flag   *debug;
		EXPORTED cmdline::arg_flag   *debug2;
#	endif
	EXPORTED cmdline::arg_string *new_clients_filename;
	EXPORTED cmdline::arg_uint   *supervisor_port;
	EXPORTED cmdline::arg_uint   *client_poll_interval;

	EXPORTED std::string clientlist_filename;
	EXPORTED std::string presentation_manager;
	EXPORTED unsigned    supervisor_listening_port;
	EXPORTED unsigned    client_poll_interval_sec;

	RH set_default_values();
	RH parse_commandline(int argc, char **argv);
	RH read_configfile();
	RH apply(int argc, char **argv);
	RH cleanup();
}

#undef I
#undef EXPORTED
#undef CONFIG_ALLOC

#endif // CONFIG_HPP_
