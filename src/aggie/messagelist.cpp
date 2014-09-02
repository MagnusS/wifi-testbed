/*! \file messagelist.cpp
 *  \copydetails messagelist.hpp
 */
#include "messagelist.hpp"

resulthandler_messages_t resulthandler_messages[] =
{
	{ NO_ERRORS,
	  "No error" },

	{ SOCKET_ERROR,
	  "Socket error" },
	{ SOCKET_ERROR_INIT,
	  "Error initializing socket" },
	{ SOCKET_ERROR_MISSING_DESTINATION,
	  "Destination address not specified" },
	{ SOCKET_ERROR_MISSING_DESTINATION_PORT,
	  "Destination port not specified" },
	{ SOCKET_ERROR_MISSING_LISTEN_PORT,
	  "Listening port not specified" },
	{ SOCKET_ERROR_COULD_NOT_BIND,
	  "Could not bind to socket" },
	{ SOCKET_ERROR_COULD_NOT_CONNECT,
	  "Could not establish connection" },
	{ SOCKET_ERROR_NOT_CONNECTED,
	  "Not connected" },
	{ SOCKET_RECEIVE_TIMEOUT,
	  "Socket receive timeout" },
	{ SOCKET_ERROR_INVALID_MESSAGE,
	  "Received an unknown or invalid message" },

	{ AGGIE_ERROR_INVALID_PM_URL,
	  "Invalid websocket-URL for presentation manager" },


	{ -1, "" } // Sentinel entry
};

