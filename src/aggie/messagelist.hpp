/*! \file messagelist.hpp
 *
 *
 * \brief Result codes and strings for use with \ref resulthandler
 *
 * \date 2010
 * \author   Forsvarets Forskningsinstitutt (FFI)\n
 *           http://www.ffi.no \n
 *           - Terje Mikal Mjelde [tmo]\n
 *             terje-mikal-mjelde@ffi.no
 */

#ifndef __MESSAGELIST_HPP
#define __MESSAGELIST_HPP

#include <string>

//! \defgroup RH_MAIN Resulthandler status IDs for main program.
//!@{
enum {
	NO_ERRORS = 0,
};
//!@}

//! \defgroup RH_SOCKETS Resulthandler for sockets.
//!@{
enum {
	SOCKET_ERROR_INIT = 30001,
	SOCKET_ERROR,
	SOCKET_ERROR_MISSING_DESTINATION,
	SOCKET_ERROR_MISSING_DESTINATION_PORT,
	SOCKET_ERROR_MISSING_LISTEN_PORT,
	SOCKET_ERROR_COULD_NOT_BIND,
	SOCKET_ERROR_COULD_NOT_CONNECT,
	SOCKET_ERROR_NOT_CONNECTED,
	SOCKET_RECEIVE_TIMEOUT,
	SOCKET_ERROR_INVALID_MESSAGE,
};
//!@}

//! \defgroup RH_AGGIE Resulthandler for Aggie.
//!@{
enum {
	AGGIE_ERROR_INVALID_PM_URL = 31001,
};
//!@}

//! \defgroup RH_TIMETOOLS Resulthandler TIMETOOLS status IDs.
//!@{
enum {
        TIMETOOLS_ERROR_INVALID_HANDLE = 32001,
        TIMETOOLS_ERROR_INVALID_INTERVAL,
        TIMETOOLS_TIMER_EXPIRED,
        TIMETOOLS_TIMER_RUNNING,
};
//!@}

/*! \brief Association between result ID and string.
*/
typedef struct {
	int id;
	std::string text;
} resulthandler_messages_t;

/*! \brief Result IDs linked to associated text string.
*/
extern resulthandler_messages_t resulthandler_messages[];

#endif // __MESSAGELIST_HPP
