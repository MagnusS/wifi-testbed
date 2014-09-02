/*! \file main.h
 *
 * \brief Global definitions for main program.
 */

#if !defined __MAIN_H
#define __MAIN_H


#include "aggie.hpp"
#include "cmdline.hpp"
#include "timetools.hpp"

#include <string>


#if defined DEBUG
#	define DEBUGINDICATOR "(debug)"
#else
#	define DEBUGINDICATOR ""
#endif


#define APPNAME       "aggie"
#define APPVERSION    "0.1" DEBUGINDICATOR
#define APPDATE       __DATE__
#define APPOWNER      "Forsvarets forskningsinstitutt"

#define DEFAULT_CLIENTLIST_FILENAME "clients.txt"
#define DEFAULT_SUPERVISOR_LISTENING_PORT 17408
#define DEFAULT_CLIENT_REPOLL_INTERVALL_SEC 15

void displayversion(cmdline *cmdl);
void printhelp(cmdline *cmdl);

extern void sleep_ms(unsigned long ms);
extern timetools timers;
extern timetools::handle uptime;
extern void client_listener(tcpsocket *listener, std::string data);


#endif // __MAIN_H

/*! \mainpage
 *
 * Information aggregator for Cognitive-P2P.
 *
 * \date 2013
 * \author   Forsvarets Forskningsinstitutt (FFI)\n
 *           http://www.ffi.no \n
 *           - Terje Mikal Mjelde [tmo]\n
 *             terje-mikal-mjelde@ffi.no
 *
 *
 * \section overview Overview
 *
 * Aggie's responsibilities is two-fold. It must collect information from all wireless clients
 * (position, channel/frequency ...) and send this to the Presentation Manager (PM) which
 * presents a geographical map of all the clients and corresponding configuration.
 * It must also respond to configuration commands from the PM and forward them to the
 * respective client(s) in a format the clients will recognize.
 *
 * \image html aggie_overview.png
 *
 * \subsection Clients
 *
 * A text file is used to let Aggie know which clients exists. This file has a default filename
 * of "clients.txt" and consists of one client per line with the IP or hostname first followed
 * by a space and then the port which that client is listening on. An example file can look like this:
 * \code
   192.168.1.11 4002
   192.168.1.138 4002
   172.20.33.45 4094
   10.10.100.110 4500
   \endcode
 *
 * \subsection Controlling Aggie
 *
 * Aggie also has a supervisor interface, which is basically a telnet-server listening on port
 * 17408 (can be changed on the command line) where a user can supervise, control and monitor
 * the application.
 *
 * \subsection running_aggie Running Aggie
 *
 * Certain aspects of Aggie can be set on the command line. Aggie recognizes the following options:
 *
 * \code
   $ ./aggie ws://url-of-pm [-h] [-v] [-q] [-c filename] [-l port]
   \endcode
 *
 * Only the \c url-of-pm part is mandatory. This is the websocket-URL of the presentation manager
 * and can be specified in several ways with or without both port and path, for instance
 * \c "ws://10.10.10.10:8080/aggiemap" or \c "10.10.10.10/aggiemap".
 *
 * The \c -v and \c -q options increases and decreases the application's verbosity level. They can
 * be specified more than once (e.g. \c "-v -v -v" to set the verbosity level to 3). A verbosity level
 * of 5 or 6 will generate debugging output as well.
 *
 * The \c "-c filename" tells Aggie which file contains the client list. If this parameter is not given,
 * Aggie uses the default filename of \c "clients.txt".
 *
 * The \c "-l port" tells Aggie which port the supervisor should be listening on (default 17408).
 *
 * \c -h gives a list of all options.
 *
 *
 * \section platforms Platforms
 * The following set of platforms and compilers have been tested:
 * - Linux 3.2 32bit
 *   - GNU GCC 4.7.1
 *
 */
