/*! \file ipsocket.hpp
 * \brief Class for socket communications.
 */

#ifndef __IPSOCKET_HPP
#define __IPSOCKET_HPP

#include "resulthandler.hpp"
#include "messagelist.hpp"
#include "threadable.hpp"
#include <string>


#include "platform.h"

#ifdef PLATFORM_WINDOWS
#	include <winsock.h>
#endif
#ifdef PLATFORM_LINUX
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <netdb.h>
#	include <unistd.h>
#	include <sys/poll.h>
#	include <sys/epoll.h>
#	include <sys/fcntl.h>
#	include <errno.h>
#	include <pthread.h>
#endif

#define MAX_EPOLL_EVENTS 64

#define DEFAULT_RECEIVE_TIMEOUT_MS 500
#define TELNET_SERVER_PROMPT "> "



/*! \brief Base class for all socket communications.
 *
 * \todo Make this class and all sub-classes Windows-compatible.
 *
 */
class ipsocket
{
public:
	ipsocket();
	ipsocket(std::string destination, unsigned port);
	ipsocket(std::string destination);
	void set_destination(std::string destination);
	void set_port(unsigned port);
	void set_port(std::string port);
	void set_local_port(unsigned port);
	void set_local_port(std::string port);
	virtual ~ipsocket();
	void common_constructor();
	unsigned port();
	unsigned listening_port();
	//typedef RH(*fetchmoredata)(char * const, unsigned, unsigned);
	//virtual fetchmoredata fetch_data;
	virtual RH_INT fetch_data(char *buffer, unsigned max_length, unsigned timeout_ms);
	RH set_endline(std::string);
protected:
	std::string destination_; //!< IP/Host of other end.
	unsigned remote_port_; //!< Port at other end.
	unsigned local_port_; //!< Port at this end. Only used when acting as a server.
	std::string endline_; //!< Usually either "\n\r" "\n".
	virtual std::string whoami() { return "ipsocket"; } //!< Name of class. Used for debugging output.
	volatile bool socket_going_down;
#ifdef PLATFORM_WINDOWS

#endif
#ifdef PLATFORM_LINUX
	void *get_in_addr(struct sockaddr *sa);
	int socket_handle;
#endif
	RH get_next_line(char * const buffer, unsigned max_bytes, unsigned timeout);
private:
	void init();
	static bool init_done; //!< Makes sure init() is only called once.
	char *inbuffer; //!< Buffer where the incoming payload is stored.
	char *inbuffer_ptr; //!< Buffer pointer in \ref inbuffer. Points to next unread byte.
	unsigned inbuffer_used; //!< How many unread bytes there are in \ref inbuffer.
	unsigned inbuffer_size; //!< How many bytes are allocated for \ref inbuffer.
};

/*! \brief TCP socket communications.
 *
 * Uses threads when acting as a TCP-server.
 * Needs a callback function for processing those messages.
 *
 */
class tcpsocket : public ipsocket, public threadable
{
public:
	tcpsocket();
	tcpsocket(std::string destination, unsigned port);
	tcpsocket(std::string destination, std::string port);
	tcpsocket(std::string destination);
	~tcpsocket();
	void common_constructor();
	virtual RH connect();
	void disconnect();
	bool connected();
	RH send(std::string data);
	RH sendline(std::string data);
	RH send(int socket_handle, std::string data);
	RH sendline(int socket_handle, std::string data);
	RH_STRING readline(unsigned max_length, unsigned timeout_ms = DEFAULT_RECEIVE_TIMEOUT_MS);
	RH_INT fetch_data(char *buffer, unsigned max_length, unsigned timeout_ms);
	typedef void (*tcpsocket_callback)(tcpsocket *, std::string);
	RH start_tcpsocket_client_listener(tcpsocket_callback);
	RH stop_tcpsocket_client_listener();
protected:
	virtual std::string whoami() { return "tcpsocket"; }
	volatile bool is_connected; //!< TRUE if socket is connected
	bool use_ipv4; //!< Recognise IPv4 addresses
	bool use_ipv6; //!< Recognize IPv6 addresses
private:
//	RH telnet_server_loop(server_callback);
	tcpsocket_callback tcpsocketclient_callback; //!< Pointer to the websocket callback function
	void thread_entry();
	volatile bool tcpsocket_client_listener_running; //!< TRUE when we're able to receive messages
	bool allow_auto_disconnect; //!< If TRUE, we must manually disconnect a session before connecting somewhere else.
};

/*! \brief UDP socket communications.
 *
 * \note No substantial functionality in here yet.
 */
class udpsocket : public ipsocket
{
public:
	udpsocket();
	udpsocket(std::string destination, unsigned port);
	udpsocket(std::string destination);
	~udpsocket();
	void common_constructor();
	RH_INT fetch_data(char *buffer, unsigned max_length, unsigned timeout_ms);
protected:
	virtual std::string whoami() { return "udpsocket"; }
private:

};

/*! \brief Telnet server.
 *
 */
class telnetserver : public tcpsocket
{
public:
	telnetserver();
	telnetserver(std::string destination, unsigned port);
	telnetserver(std::string destination, std::string port);
	telnetserver(std::string destination);
	~telnetserver();
	void common_constructor();
	RH setup_server(std::string banner);
	//! Signature of callback function that handles incoming messages from the TCP-server.
	typedef void (*telnetserver_callback)(telnetserver *, int, std::string, std::string);
	RH start_telnet_server(telnetserver_callback);
	RH stop_telnet_server();
protected:
	virtual std::string whoami() { return "telnetserver"; }
private:
	void thread_entry();
	telnetserver_callback telnet_callback; //!< Pointer to the TCP-server callback function
	volatile bool telnet_server_running; //!< TRUE When the TCP-server is running
	std::string telnet_server_banner; //!< Welcome banner for new clients
};

/*! \brief Websocket communications (specialized TCP-socket).
 *
 * Uses threads for receiving messages from a websocket server.
 * Needs a callback function for processing those messages.
 *
 * Much of this code is adapted from https://github.com/dhbaird/easywsclient (MIT-licensed).
 *
 */
class websocket : public tcpsocket
{
public:
	websocket();
	websocket(std::string destination, std::string port);
	websocket(std::string destination, unsigned port);
	websocket(std::string destination);
	websocket(std::string destination, std::string port, std::string path);
	websocket(std::string destination, unsigned port, std::string path);
	~websocket();
	void common_constructor();
	RH connect();
	void disconnect();
	bool connected();
	//! Signature of callback function that handles incoming messages from the websocket-thread.
	typedef void (*websocket_callback)(websocket *, std::string);
	RH start_websocket_client_listener(websocket_callback);
	RH stop_websocket_client_listener();
	RH send(std::string);
	RH_STRING read_data(int timeout_ms = DEFAULT_RECEIVE_TIMEOUT_MS);
	std::string url();
protected:
	virtual std::string whoami() { return "websocket"; }
private:
	websocket_callback websocketclient_callback; //!< Pointer to the websocket callback function
	void thread_entry();
	volatile bool websocket_client_listener_running; //!< TRUE when we're able to receive messages
	std::string remote_path_; //!< Remote path of websocket (with leading / if not empty)
	bool use_mask; //!< TRUE if we should mask the data we're sending (http://tools.ietf.org/html/rfc6455#section-5.3)
	/*! \brief TRUE if websocket is connected to the other end.
	 * \note NOT the same as \ref is_connected in #tcpsocket which works at a lower level in the stack.
	 */
	volatile bool websocket_is_connected_;

	//! \brief Websocket header definition
	//!
	//! http://tools.ietf.org/html/rfc6455#section-5.2  Base Framing Protocol
	//!
	//! \code
	//!  0                   1                   2                   3
	//!  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	//! +-+-+-+-+-------+-+-------------+-------------------------------+
	//! |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
	//! |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
	//! |N|V|V|V|       |S|             |   (if payload len==126/127)   |
	//! | |1|2|3|       |K|             |                               |
	//! +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
	//! |     Extended payload length continued, if payload len == 127  |
	//! + - - - - - - - - - - - - - - - +-------------------------------+
	//! |                               |Masking-key, if MASK set to 1  |
	//! +-------------------------------+-------------------------------+
	//! | Masking-key (continued)       |          Payload Data         |
	//! +-------------------------------- - - - - - - - - - - - - - - - +
	//! :                     Payload Data continued ...                :
	//! + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
	//! |                     Payload Data continued ...                |
	//! +---------------------------------------------------------------+
	//! \endcode
	struct wsheader_type
	{
		unsigned header_size; //!< Size of header
		bool fin;
		bool mask;
		enum opcode_type {
			CONTINUATION = 0x0,
			TEXT_FRAME = 0x1,
			BINARY_FRAME = 0x2,
			CLOSE = 8,
			PING = 9,
			PONG = 0xa,
		} opcode;
		int N0;
		uint64_t N;
		uint8_t masking_key[4];
	};
};


#endif // __IPSOCKET_HPP
