/*! \file ipsocket.cpp
 *  \copydoc ipsocket.hpp
 */

#include "ipsocket.hpp"
#include "stringutils.hpp"
#include "vout.hpp"
#include <cstring>
#include <iostream>
#include <typeinfo>

bool ipsocket::init_done = false;


/*! \brief Common initialization called by all constructors.
*/
void ipsocket::common_constructor()
{
	init();
	inbuffer_size = 100;
	inbuffer_used = 0;
	inbuffer = new char[inbuffer_size];
	memset(inbuffer, 0, inbuffer_size);
	inbuffer_ptr = inbuffer;
	set_endline("\n");
	socket_going_down = false;
}

/*! \brief Default constructor.
*/
ipsocket::ipsocket()
	: destination_(""),
	  remote_port_(0),
	  local_port_(0),
	  inbuffer(NULL),
	  inbuffer_ptr(NULL)
{
	common_constructor();
}

/*! \brief Constructor with specified destination and port.
 *
 */
ipsocket::ipsocket(std::string destination, unsigned port)
	: destination_(destination),
	  remote_port_(port),
	  local_port_(0),
	  inbuffer(NULL),
	  inbuffer_ptr(NULL)
{
	common_constructor();
}

/*! \brief Constructor with specified destination only.
 *
 */
ipsocket::ipsocket(std::string destination)
	: destination_(destination),
	  remote_port_(0),
	  local_port_(0),
	  inbuffer(NULL),
	  inbuffer_ptr(NULL)
{
	common_constructor();
}

/*! \brief Destructor.
*/
ipsocket::~ipsocket()
{
	if (inbuffer != NULL)
	{
		delete[] inbuffer;
	}
}

/*! Specifies the remote host address
 *
 * It will not change an existing connection.
 * \param destination IP or host name
 */
void ipsocket::set_destination(std::string destination)
{
	destination_ = destination;
}

/*! Specifies the remote port number.
 *
 * It will not change an existing connection.
 * \param port Port number
 */
void ipsocket::set_port(std::string port)
{
	int i;
	string_to_int(port, i);
	remote_port_ = i;
}

/*! Specifies the remote port number.
 *
 * It will not change an existing connection.
 * \param port Port number
 */
void ipsocket::set_port(unsigned port)
{
	remote_port_ = port;
}

/*! Specifies the local port number (for server).
 *
 * It will not change an existing connection.
 * \param port Port number
 */
void ipsocket::set_local_port(std::string port)
{
	int i;
	string_to_int(port, i);
	local_port_ = i;
}

/*! Specifies the local port number (for server).
 *
 * It will not change an existing connection.
 * \param port Port number
 */
void ipsocket::set_local_port(unsigned port)
{
	local_port_ = port;
}


/*! \brief Perform one-time socket initialization.
 *
 * Must be called once before sockets are used.
 * It will only execute once per application run,
 * regardless of the number of class instantiations.
 */
void ipsocket::init()
{
	if (!init_done)
	{
#		ifdef PLATFORM_WINDOWS
		WSAData wsaData;
		if (WSAStartup(MAKEDWORD(2,0), &wsaData) != 0)
		{
			result = SOCKET_ERROR_INIT;
		}
#		endif
		init_done = true;
	}
}

/*! \brief Extracts the proper IPv4 or IPv6 address struct from given sockaddr.
 *
 * \param sa Socket address structure
 * \return Pointer to IPv4 or IPv6 address struct
 */
void *ipsocket::get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*! \brief Port-getter (remote port).
 *
 */
unsigned ipsocket::port()
{
	return remote_port_;
}

/*! \brief Port-getter (local port).
 *
 */
unsigned ipsocket::listening_port()
{
	return local_port_;
}

/*! \brief Fetches the next line (terminated by CR or CRLF) of incoming text.
 *
 *  CR and LF are never included in the returned textline.
 *
 *  Text are fetched from the \ref inbuffer buffer. If there's not
 *  enough characters there, more data is read from the socket.
 *
 *  \param[out] buffer Pointer to buffer where the returning textline should be stored
 *  \param max_bytes Size of buffer
 *  \param timeout Timeout in miliseconds before returning anyway
 *  \return #NO_ERRORS, #SOCKET_ERROR or #SOCKET_RECEIVE_TIMEOUT
 *
 */
RH ipsocket::get_next_line(char * const buffer, unsigned max_bytes, unsigned timeout)
{
	RH result;
	unsigned bytes_read = 0;
	char * outbuffer_ptr;
	bool giveup = false;

	outbuffer_ptr = buffer;
	result.set_ok();

	while (!giveup && !socket_going_down)
	{
		// First, try to read from the buffer
		while ((inbuffer_ptr < (inbuffer + inbuffer_used))
			&& (inbuffer_ptr < (inbuffer + inbuffer_size))
		    && (bytes_read < max_bytes)
		    && (*inbuffer_ptr != '\n'))
		{
			if (*inbuffer_ptr != '\r' && (*inbuffer_ptr != '\0'))
			{
				*outbuffer_ptr = *inbuffer_ptr;
				bytes_read += 1;
				outbuffer_ptr += 1;
			}
			inbuffer_ptr += 1;
		}
		if ((inbuffer_ptr < (inbuffer + inbuffer_used))
		 && (inbuffer_ptr < (inbuffer + inbuffer_size))
		 && (*inbuffer_ptr == '\n'))
		{
			*inbuffer_ptr = '\0'; // To prevent trigging on the same \n the next time
			giveup = true;
		}
		else if (bytes_read < max_bytes)
		{
			// We've not read an entire line yet; neet to get more data
			RH_INT fetch_result = fetch_data(inbuffer, inbuffer_size, timeout);
			if (fetch_result.is_not_ok())
			{
//				std::cout << "Fetch data failed on " << destination_ << ":" << remote_port_ << std::endl;
				result.set_not_ok(fetch_result.id());
				giveup = true;
			}
			else
			{
				inbuffer_used = fetch_result.value();
				inbuffer_ptr = inbuffer;
				if (fetch_result.is_not_ok())
				{
					giveup = true;
					if (fetch_result.is_not_ok())
					{
						result.set_not_ok();
						result.set_exitstatus(fetch_result.id(), fetch_result.text());
					}
				}
			}
		}
		else
		{
			// Return buffer is full
		}
	}
	*outbuffer_ptr = '\0';
	return(result);
}

/*! \brief Reads data from the socket.
 *
 * \note Currently, the ipsocket class does not have any functionality that requires
 *       reading data. Hence, this method does nothing but return OK. This may change
 *       in the future.
 *
 * \param[out] buffer Buffer for storing incoming data
 * \param max_length Size of buffer
 * \param timeout_ms Timeout in miliseconds
 * \return #NO_ERRORS, SOCKET_ERROR or SOCKET_RECEIVE_TIMEOUT
 *
 */
RH_INT ipsocket::fetch_data(char * const buffer, unsigned max_length, unsigned timeout_ms)
{
	RH_INT result;
	result.set_ok();
	result.set_value(0);
	return(result);
}

/*! \brief Specifies what characters are used as newline when sending text.
 *
 * \param endline Sequence of characters; typically either "\n\r" or "\n"
 * \return #NO_ERRORS
 */
RH ipsocket::set_endline(std::string endline)
{
	RH result;
	result.set_ok();

	endline_ = endline;

	return(result);
}

/*! \brief Common initialization called by all constructors.
*/
void tcpsocket::common_constructor()
{
	is_connected = false;
	socket_handle = 0;
	allow_auto_disconnect = true;
	use_ipv4 = true;
	use_ipv6 = false;
	tcpsocket_client_listener_running = false;
}

/*! \brief Default constructor.
*/
tcpsocket::tcpsocket()
{
	common_constructor();
}

/*! \brief Constructor with specified destination and port.
 *
 */
tcpsocket::tcpsocket(std::string destination, unsigned port)
{
	common_constructor();
	destination_ = destination;
	remote_port_ = port;
}

/*! \brief Constructor with specified destination and port.
 *
 */
tcpsocket::tcpsocket(std::string destination, std::string port)
{
	common_constructor();
	destination_ = destination;
	int i;
	string_to_int(port, i);
	remote_port_ = i;
}

/*! \brief Constructor with specified destination only.
 *
 */
tcpsocket::tcpsocket(std::string destination)
{
	common_constructor();
	destination_ = destination;
	remote_port_ = 0;
}


/*! \brief Destructor.
*/
tcpsocket::~tcpsocket()
{
	disconnect();
}

/*! \brief Establishes a connection to remote #destination_ : #remote_port_.
 *
 * \todo Make non-blocking connect to avoid long timeouts when host is not responding.
 *
 * \returns #NO_ERRORS, #SOCKET_ERROR_MISSING_DESTINATION,
 *          #SOCKET_ERROR_MISSING_DESTINATION_PORT or #SOCKET_ERROR_COULD_NOT_CONNECT
 */
RH tcpsocket::connect()
{
	RH result;
	result.set_ok();

	if (destination_.length() == 0)
	{
		result.set_not_ok(SOCKET_ERROR_MISSING_DESTINATION);
		goto connect_end;
	}
	if (remote_port_ == 0)
	{
		result.set_not_ok(SOCKET_ERROR_MISSING_DESTINATION_PORT);
		goto connect_end;
	}

#ifdef PLATFORM_WINDOWS

#endif
#ifdef PLATFORM_LINUX
	if (allow_auto_disconnect)
	{
		disconnect(); // Make sure we close any existing connections
	}
	struct ::addrinfo socket_hints;
	memset(&socket_hints, 0, sizeof (socket_hints));
	socket_hints.ai_family = AF_UNSPEC;
	socket_hints.ai_socktype = SOCK_STREAM;
	int rv;
	struct ::addrinfo *servinfo;
	if ((rv = ::getaddrinfo(destination_.c_str(), int_to_string(remote_port_).c_str(),
						  &socket_hints, &servinfo)) != 0)
	{
		result.set_not_ok(::gai_strerror(rv));
		goto connect_end;
	}
	struct ::addrinfo *p;
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((socket_handle = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			continue;
		}
		if (::connect(socket_handle, p->ai_addr, p->ai_addrlen) == -1)
		{
			::close(socket_handle);
			continue;
		}
		break;
	}

	if (p == NULL)
	{
		result.set_not_ok(SOCKET_ERROR_COULD_NOT_CONNECT);
		::freeaddrinfo(servinfo);
		goto connect_end;
	}
	is_connected = true;

	char s[INET6_ADDRSTRLEN];
	::inet_ntop(p->ai_family, get_in_addr((struct ::sockaddr *)p->ai_addr), s, sizeof s);
	::freeaddrinfo(servinfo);

#endif

connect_end:
	return(result);
}

/*! \brief Disconnects from currently \ref connect() "connected" session.
 *
 */
void tcpsocket::disconnect()
{
//std::cout << destination_  << ":" << remote_port_ << " is_connected? " << (is_connected ? "yes" : "no") << std::endlc;
	//if (is_connected)
	{
		//::shutdown(socket_handle, SHUT_RDWR);
		stop_tcpsocket_client_listener();
		if (socket_handle != 0)
		{
			::close(socket_handle);
		}
	}
	is_connected = false;
	socket_handle = 0;
}

/*! \brief Tells if the socket is \ref connect() "connected".
 * \return TRUE if connected
 */
bool tcpsocket::connected()
{
	return is_connected;
}

/*! \brief Transmits a string over the socket.
 * \param data String to transmit
 * \return #NO_ERRORS, #SOCKET_ERROR_NOT_CONNECTED or #SOCKET_ERROR
 */
RH tcpsocket::send(std::string data)
{
	RH result;
	result.set_ok();

	if (!is_connected)
	{
		result.set_not_ok(SOCKET_ERROR_NOT_CONNECTED);
		return(result);
	}

	vout(VOUT_DEBUG) << "[" << whoami() << "] (socket# " << socket_handle << ") sending string \"" << ascii_safe(data, true) << "\"" << std::endlc;
	if (::send(socket_handle, data.c_str(), data.length(), MSG_NOSIGNAL) == -1)
	{
		result.set_not_ok(SOCKET_ERROR);
		return(result);
	}

	return(result);
}

/*! \brief Transmits a string with an appended \ref set_endline() "newline" over the socket.
 * \param data String to transmit
 * \return #NO_ERRORS, #SOCKET_ERROR_NOT_CONNECTED or #SOCKET_ERROR
 */
RH tcpsocket::sendline(std::string data)
{
	data += endline_;
	return send(data);
}

/*! \brief Transmits a string over the socket.
 *
 * This function is primarily used for sending data to a connected client
 * when we act as a TCP-server.
 *
 * \param handle Socket handle
 * \param data String to transmit
 * \return #NO_ERRORS, #SOCKET_ERROR_NOT_CONNECTED or #SOCKET_ERROR
 */
RH tcpsocket::send(int handle, std::string data)
{
	RH result;
	result.set_ok();

	if (!is_connected)
	{
		result.set_not_ok(SOCKET_ERROR_NOT_CONNECTED);
		goto send_end;
	}

	if (::send(handle, data.c_str(), data.length(), 0) == -1)
	{
		result.set_not_ok(SOCKET_ERROR);
		goto send_end;
	}

send_end:
	return(result);
}

/*! \brief Transmits a string with an appended \ref set_endline() "newline" over the socket.
 *
 * This function is primarily used for sending data to a connected client
 * when we act as a TCP-server.
 *
 * \param handle Socket handle
 * \param data String to transmit
 * \return #NO_ERRORS, #SOCKET_ERROR_NOT_CONNECTED or #SOCKET_ERROR
 */
RH tcpsocket::sendline(int handle, std::string data)
{
	data += "\n";
	return send(handle, data);
}

/*! \brief Reads data from the socket.
 *
 * \param[out] buffer Buffer for storing incoming data
 * \param max_length Size of buffer
 * \param timeout_ms Timeout in miliseconds
 * \return #NO_ERRORS, SOCKET_ERROR or SOCKET_RECEIVE_TIMEOUT
 *
 */
RH_INT tcpsocket::fetch_data(char * const buffer, unsigned max_length, unsigned timeout_ms)
{
	RH_INT result;
	result.set_ok();
	result.set_value(0);
	struct ::pollfd ufds[1];
	ufds[0].fd = socket_handle;
	ufds[0].events = POLLIN;
	int rv;
	rv = ::poll(ufds, 1, timeout_ms);
	if (rv == -1) result.set_not_ok(SOCKET_ERROR);
	if (rv ==  0) result.set_ok(SOCKET_RECEIVE_TIMEOUT); // Timeout is not an error
	if ((rv > 0) && (ufds[0].revents & POLLIN) && !(ufds[0].revents & POLLHUP))
	{
//if (remote_port_ == 4001) std::cout << "#" << rv << "$" << ufds[0].revents << "$"; std::cout.flush();
		int bytes_received = 0;
		bytes_received = ::recv(socket_handle, buffer, max_length, 0);
//if (remote_port_ == 4001) std::cout << bytes_received; std::cout.flush();
		result.set_value(bytes_received);
//		if (bytes_received >= 0)
//		{
//			buffer[bytes_received] = '\0';
//		}
	}
	else if (ufds[0].revents & POLLHUP)
	{
		result.set_not_ok(SOCKET_ERROR);
	}
	return(result);
}

/*! \brief Fetches the next line (terminated by CR or CRLF) of incoming text.
 *
 *  CR and LF are never included in the returned textline.
 *
 *  Text are fetched from the \ref inbuffer buffer. If there's not
 *  enough characters there, more data is read from the socket.
 *
 *  \param max_length Max bytes to return
 *  \param timeout_ms Timeout in miliseconds before returning anyway
 *  \return Either #NO_ERRORS with value() set to the textline just read, or one
 *          of #SOCKET_ERROR_NOT_CONNECTED, #SOCKET_ERROR or #SOCKET_RECEIVE_TIMEOUT
 *
 */
RH_STRING tcpsocket::readline(unsigned max_length, unsigned timeout_ms)
{
	RH_STRING result;
	result.set_ok();

	if (!is_connected)
	{
		result.set_not_ok(SOCKET_ERROR_NOT_CONNECTED);
	}
	else
	{
		char *buffer;
		buffer = new char[max_length+1];
		RH read_result;
		read_result = get_next_line(buffer, max_length, timeout_ms);
		if (read_result.is_not_ok())
		{
			result.set_not_ok();
			result.set_exitstatus(read_result.id(), read_result.text());
		}
		if (result.is_ok())
		{
			result.set_value(buffer);
		}
		delete[] buffer;
	}

	return(result);
}

/*! \brief Thread that dispatches incoming message events.
 *
 * This thread is started by calling #start_tcpsocket_client_listener().
 * Incoming messages are dispatched to the specified
 * \ref tcpsocket_callback "callback" function.
 *
 * The proper way to terminate this thread is to call #stop_tcpsocket_client_listener().
 *
 */
void tcpsocket::thread_entry()
{

	vout(VOUT_DEBUG) << "[" << whoami() << "] listener thread for " << destination_ << ":" << remote_port_ << " started" << std::endlc;
	RH_STRING rcv;
	tcpsocket_client_listener_running = true;
	while (tcpsocket_client_listener_running)
	{
//		std::cout << "!" << destination_ << ":" << remote_port_; std::cout.flush();
		rcv = readline(1000);
//		std::cout << "! "; std::cout.flush();
		if (rcv.is_ok())
		{
			vout(VOUT_DEBUG) << "[" << whoami() << "] received string \"" << ascii_safe(rcv.value(), true) << "\"" << std::endlc;
			tcpsocketclient_callback(this, rcv.value());
		}
		else if (rcv != SOCKET_RECEIVE_TIMEOUT)
		{
			vout(VOUT_DEBUG) << ansi::red << ansi::bright << "[" << whoami() << " thread_entry()] " <<rcv.text() << std::endlc;
			tcpsocket_client_listener_running = false;
		}
	}
	vout(VOUT_DEBUG) << "[" << whoami() << "] exiting listener thread for " << destination_ << ":" << remote_port_  << std::endlc;
}

/*! \brief Starts a thread that listens for incoming messages
 *
 * It starts a separate \ref thread_entry() "thread" that does all the work. It
 * does not return until it has verified that the listener thread actually is running.
 *
 * \param callback Pointer to callback function that handles incoming messages
 * \return Always returns #NO_ERRORS
 */
RH tcpsocket::start_tcpsocket_client_listener(tcpsocket_callback callback)
{
	RH result;
	result.set_ok();

	tcpsocketclient_callback = callback;

	tcpsocket_client_listener_running = false;
	run();
	while (!tcpsocket_client_listener_running);


	return result;
}

/*! \brief Stops a running message listener.
 * It does not return until the listener actually has stopped.
 * \return Always returns #NO_ERRORS
 */
RH tcpsocket::stop_tcpsocket_client_listener()
{
	RH result;
	result.set_ok();

	if (tcpsocket_client_listener_running)
	{
		tcpsocket_client_listener_running = false;
		socket_going_down = true;
		wait();
	}

	return result;
}

/*! \brief Common initialization called by all constructors.
*/
void telnetserver::common_constructor()
{
}

/*! \brief Default constructor.
*/
telnetserver::telnetserver()
{
	common_constructor();
}

/*! \brief Constructor with specified destination and port.
 *
 */
telnetserver::telnetserver(std::string destination, unsigned port)
{
	common_constructor();
	telnet_server_running = false;
}

/*! \brief Constructor with specified destination and port.
 *
 */
telnetserver::telnetserver(std::string destination, std::string port)
{
	common_constructor();
	telnet_server_running = false;
}

/*! \brief Constructor with specified destination only.
 *
 */
telnetserver::telnetserver(std::string destination)
{
	common_constructor();
	telnet_server_running = false;
}


/*! \brief Destructor.
*/
telnetserver::~telnetserver()
{
	disconnect();
}

/*! \brief Set us up to act as a TCP-server and allow incoming connections.
 *
 * #set_local_port() must have been called to specifiy which port
 * we shall listen on.
 *
 * The flags #use_ipv4 and #use_ipv6 specifies which type of addresses
 * to listen for. Both can be allowed at the same time.
 *
 * \return Either #NO_ERRORS or one of #SOCKET_ERROR_MISSING_LISTEN_PORT,
 *         #SOCKET_ERROR, #SOCKET_ERROR_COULD_NOT_BIND or an error
 *         message from gai_strerror()
 *
 */
RH telnetserver::setup_server(std::string banner)
{
	RH result;
	result.set_ok();

	bool use_only_ipv4, use_only_ipv6, use_both_ipv4_and_ipv6;

	if (local_port_ == 0)
	{
		result = SOCKET_ERROR_MISSING_LISTEN_PORT;
		return(result);
	}

	use_only_ipv4 = (use_ipv4 && !use_ipv6);
	use_only_ipv6 = (!use_ipv4 && use_ipv6);
	use_both_ipv4_and_ipv6 = (use_ipv4 && use_ipv6);

	telnet_server_banner = banner;

#ifdef PLATFORM_WINDOWS

#endif
#ifdef PLATFORM_LINUX
	struct ::addrinfo socket_hints;
	::memset(&socket_hints, 0, sizeof (socket_hints));
	if (use_both_ipv4_and_ipv6)
		socket_hints.ai_family = AF_INET6;
	if (use_only_ipv4)
		socket_hints.ai_family = AF_INET;
	if (use_only_ipv6)
		socket_hints.ai_family = AF_INET6;
	socket_hints.ai_socktype = SOCK_STREAM;
	socket_hints.ai_flags    = AI_PASSIVE;
	int rv;
	struct ::addrinfo *servinfo;
	if ((rv = ::getaddrinfo(NULL, int_to_string(local_port_).c_str(),
	                        &socket_hints, &servinfo)) != 0)
	{
		result.set_not_ok(::gai_strerror(rv));
		return(result);
	}

	struct ::addrinfo *p;
	int yes;
	yes = 1;
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((socket_handle = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			continue;
		}
		if (::setsockopt(socket_handle, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		{
			result.set_not_ok(SOCKET_ERROR);
			result.set_not_ok(result.text() + " (setsockopt)");
			::freeaddrinfo(servinfo);
			return(result);
		}
		if (::bind(socket_handle, p->ai_addr, p->ai_addrlen) == -1)
		{
			::close(socket_handle);
			continue;
		}
		break;
	}

	if (p == NULL)
	{
		result.set_not_ok(SOCKET_ERROR_COULD_NOT_BIND);
		::freeaddrinfo(servinfo);
		return(result);
	}

	int flags = ::fcntl(socket_handle, F_GETFL, 0);
	if (flags == -1)
	{
		result.set_not_ok(SOCKET_ERROR);
		result.set_not_ok(result.text() + " (fcntl get)");
		::freeaddrinfo(servinfo);
		return(result);
	}
	flags |= O_NONBLOCK;
	::fcntl(socket_handle, F_SETFL, flags);
	if (flags == -1)
	{
		result.set_not_ok(SOCKET_ERROR);
		result.set_not_ok(result.text() + " (fcntl set)");
		::freeaddrinfo(servinfo);
		return(result);
	}

	is_connected = true;

	::freeaddrinfo(servinfo);
#endif

	return(result);

}

/*! \brief Thread that accepts incoming connections and dispatches message events.
 *
 * This thread is started by calling #start_telnet_server().
 * Incoming messages from connected clients are dispatched to the specified
 * \ref tcpsocket_callback "callback" function.
 *
 * The proper way to terminate this thread is to call #stop_telnet_server().
 *
 */
void telnetserver::thread_entry()
{

	::socklen_t sin_size;
	int epoll_handle;
	struct ::epoll_event event;
	struct ::epoll_event *events = NULL;

	if (::listen(socket_handle, 10) == -1)
	{
//		result.set_not_ok(SOCKET_ERROR);
//		result.set_not_ok(result.text() + " (listen)");
//		return(result);
	}

	epoll_handle = ::epoll_create(1);
	if (epoll_handle == -1)
	{
//		result.set_not_ok(SOCKET_ERROR);
//		result.set_not_ok(result.text() + " (epoll_create)");
//		return(result);
	}
	memset(&event, 0, sizeof event); // To please valgrind
	event.data.fd = socket_handle;
	event.events = EPOLLIN | EPOLLET;
	if (::epoll_ctl(epoll_handle, EPOLL_CTL_ADD, socket_handle, &event) == -1)
	{
//		result.set_not_ok(SOCKET_ERROR);
//		result.set_not_ok(result.text() + " (epoll_ctl)");
//		return(result);
	}
	events = (struct ::epoll_event*)calloc(MAX_EPOLL_EVENTS, sizeof event);

	vout(VOUT_VERBOSER) << "Listening for telnet connections on port " << local_port_ << std::endlc;

	telnet_server_running = true;
	while(telnet_server_running)
	{
		int n, i;
		n = ::epoll_wait(epoll_handle, events, MAX_EPOLL_EVENTS, 1);
		for (i = 0; i < n; i++)
		{
			if ((  events[i].events & EPOLLERR)
			 || (  events[i].events & EPOLLHUP)
			 || (!(events[i].events & EPOLLIN )))
			{
				::close(events[i].data.fd);
				continue;
			}
			else if (events[i].data.fd == socket_handle)
			{
				// Received new connection; accept it if possible
				while(1)
				{
					struct ::sockaddr in_addr;
					::socklen_t in_len;
					int new_handle;
					char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
					in_len = sizeof in_addr;
					new_handle = accept(socket_handle, &in_addr, &in_len);
					if (new_handle == -1)
					{
						if ((errno == EAGAIN)
						 || (errno == EWOULDBLOCK))
						{
							// We have processed all incoming connections
							break;
						}
						else
						{
							// Some other error
							break;
						}
					}

					if (::getnameinfo(&in_addr, in_len,
					                  hbuf, sizeof hbuf,
					                  sbuf, sizeof sbuf,
					                  NI_NUMERICHOST | NI_NUMERICSERV) == 0)
					{
						vout(VOUT_VERBOSER) << "Accepted incoming telnet connection from ";
						vout(VOUT_VERBOSER) << hbuf << " on port " << sbuf << std::endlc;
						send(new_handle, telnet_server_banner);
					}
					int flags = ::fcntl(new_handle, F_GETFL, 0);
					if (flags == -1)
					{
//						result.set_not_ok(SOCKET_ERROR);
//						result.set_not_ok(result.text() + " (fcntl set)");
//						return(result);
					}
					flags |= O_NONBLOCK;
					::fcntl(new_handle, F_SETFL, flags);
					if (flags == -1)
					{
//						result.set_not_ok(SOCKET_ERROR);
//						result.set_not_ok(result.text() + " (fcntl set)");
//						return(result);
					}
					event.data.fd = new_handle;
					event.events = EPOLLIN | EPOLLET;
					if (::epoll_ctl(epoll_handle, EPOLL_CTL_ADD, new_handle, &event) == -1)
					{
//						result.set_not_ok(SOCKET_ERROR);
//						result.set_not_ok(result.text() + " (epoll_handle)");
//						return(result);
					}
				}
				continue;
			}
			else
			{
				// Data has been received and is waiting to be read
				bool done = false;
				struct ::sockaddr in_addr;
				::socklen_t in_len;
				in_len = sizeof in_addr;
				char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
				getsockname(events[i].data.fd, &in_addr, &in_len);
				::getnameinfo(&in_addr, in_len,
				               hbuf, sizeof hbuf,
				               sbuf, sizeof sbuf,
				               NI_NUMERICHOST | NI_NUMERICSERV);
				while (1)
				{
					ssize_t count;
					char buf[512];
					count = ::read(events[i].data.fd, buf, sizeof buf);
					if (count == -1)
					{
						// If errno == EAGAIN we have read all data
						if (errno != EAGAIN)
						{
							// Something went wrong
							done = true;
						}
						break;
					}
					else if (count == 0)
					{
						// End of file; remote closed the connection
						vout(VOUT_VERBOSER) << "Client " << hbuf << " closed the connection" << std::endlc;
						done = 1;
						break;
					}
					buf[count] = '\0';
					telnet_callback(this, events[i].data.fd, std::string(hbuf), std::string(buf));
				}
				if (done)
				{
					::close(events[i].data.fd);
				}
			}
		}


	}
	if (events != NULL) free(events);
	telnet_server_running = false;
}

/*! \brief Starts a TCP-server.
 *
 * #setup_server() must be used before calling this function.
 *
 * It starts a separate \ref thread_entry() "thread" that does all the work. It
 * does not return until it has verified that the server actually is running.
 *
 * \param callback Pointer to callback function that handles incoming messages
 * \return Always returns #NO_ERRORS
 */
RH telnetserver::start_telnet_server(telnetserver_callback callback)
{
	RH result;
	result.set_ok();

	telnet_callback = callback;

	telnet_server_running = false;
	run();
	while (!telnet_server_running);

	return result;
}

/*! \brief Stops a running TCP-server.
 * It does not return until the server actually has stopped.
 * \return Always returns #NO_ERRORS
 */
RH telnetserver::stop_telnet_server()
{
	RH result;
	result.set_ok();

	telnet_server_running = false;
	wait();

	return result;
}

/*! \brief Default constructor.
*/
udpsocket::udpsocket()
{

}

/*! \brief Constructor with specified destination and port.
 *
 */
udpsocket::udpsocket(std::string destination, unsigned port)
{
	destination_ = destination;
	remote_port_ = port;
}

/*! \brief Constructor with specified destination only.
 *
 */
udpsocket::udpsocket(std::string destination)
{
	destination_ = destination;
	remote_port_ = 0;
}



/*! \brief Destructor.
*/
udpsocket::~udpsocket()
{

}

/*! \brief Reads data from the socket.
 *
 * \note Currently, the udpsocket class does not have any functionality that requires
 *       reading data. Hence, this method does nothing but return OK. This may change
 *       in the future.
 *
 * \param[out] buffer Buffer for storing incoming data
 * \param max_length Size of buffer
 * \param timeout_ms Timeout in miliseconds
 * \return #NO_ERRORS, SOCKET_ERROR or SOCKET_RECEIVE_TIMEOUT
 *
 */
RH_INT udpsocket::fetch_data(char * const buffer, unsigned max_length, unsigned timeout_ms)
{
	RH_INT result;
	result.set_value(0);
	return(result);
}


/*! \brief Default constructor.
*/
websocket::websocket()
{
	common_constructor();
}

/*! \brief Constructor with specified destination and port.
 *
 */
websocket::websocket(std::string destination, std::string port)
{
	common_constructor();
	destination_ = destination;
	int i;
	string_to_int(port, i);
	remote_port_ = i;
}

/*! \brief Constructor with specified destination and port.
 *
 */
websocket::websocket(std::string destination, unsigned port)
{
	common_constructor();
	destination_ = destination;
	remote_port_ = port;
}

/*! \brief Constructor with specified destination, port and remote path.
 *
 */
websocket::websocket(std::string destination, std::string port, std::string path)
{
	common_constructor();
	destination_ = destination;
	int i;
	string_to_int(port, i);
	remote_port_ = i;
	remote_path_ = path;
}

/*! \brief Constructor with specified destination, port and remote path.
 *
 */
websocket::websocket(std::string destination, unsigned port, std::string path)
{
	common_constructor();
	destination_ = destination;
	remote_port_ = port;
	remote_path_ = path;
}

/*! \brief Constructor with specified destination only.
 *
 */
websocket::websocket(std::string destination)
{
	common_constructor();
	destination_ = destination;
	remote_port_ = 0;
}

/*! \brief Destructor.
*/
websocket::~websocket()
{
}

/*! \brief Common initialization called by all constructors.
*/
void websocket::common_constructor()
{
	tcpsocket::common_constructor();
	remote_path_ = "";
	use_mask = true;
	websocket_is_connected_ = false;
}

/*! \brief Established a websocket connection to a remote server.
 *
 * This is a very simple handshake implementation with little error checking
 * which could use some improvement.
 *
 * \return OK or NOT_OK
 */
RH websocket::connect()
{
	RH result = tcpsocket::connect();
	if (result.is_ok())
	{
		// Perform websocket handshake
		set_endline("\r\n");
		vout(VOUT_DEBUG) << "Initiating websocket handshake" << std::endlc;
		sendline(format_string("GET ws://%s:%d%s HTTP/1.1", destination_.c_str(), remote_port_, remote_path_.c_str()));
		sendline(format_string("Host: %s:%d", destination_.c_str(), remote_port_));
		sendline(format_string("Upgrade: websocket"));
		sendline(format_string("Connection: Upgrade"));
		sendline(format_string("Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw=="));
		sendline(format_string("Sec-WebSocket-Version: 13"));
		sendline(format_string(""));

		RH_STRING rcv = readline(100);
		if (rcv.is_ok())
		{
			vout(VOUT_DEBUG) << "[" << whoami() << "] received string: \"" << ascii_safe(rcv.value(), true) << "\"" << std::endlc;
			if (rcv.value().substr(0, 12) == "HTTP/1.1 101")
			{
				while ((rcv.value().length() != 0) && rcv.is_ok())
				{
					rcv = readline(100);
					if (rcv.is_ok())
					{
						vout(VOUT_DEBUG) << "[" << whoami() << "] received string: \"" << ascii_safe(rcv.value(), true) << "\""  << std::endlc;
						// Should do some checking here
					}
					else
					{
						vout(VOUT_ERROR) << "[" << whoami() << "] handshake failed" << std::endlc;
					}
				}
			}
		}
		else
		{
			vout(VOUT_DEBUG) << ansi::red << "[" << whoami() << "] error: " << rcv.text() << std::endlc;
			result.set_not_ok();
		}
	}

	if (result.is_ok())
	{
		websocket_is_connected_ = true;
	}

	return result;
}

/*! \brief Disconnects from a \ref connect() "connected" websocket session.
 *
 */
void websocket::disconnect()
{

	tcpsocket::disconnect();
	websocket_is_connected_ = false;
}

/*! \brief Tells if a websocket \ref connect() "connection" is established.
 *
 * This is a higher level connection than the one that #tcpsocket uses.
 * \return TRUE if connected
 */
bool websocket::connected()
{
	return websocket_is_connected_;
}

/*! \brief Reads a new data chunk from the websocket.
 *
 * Unless a timeout occurres, an entire data chunk is read.
 *
 * \param timeout_ms Timeout in miliseconds
 * \return #NO_ERRORS with the read string as a value(), or one of
 *         #SOCKET_ERROR_NOT_CONNECTED or #SOCKET_ERROR_INVALID_MESSAGE.
 */
RH_STRING websocket::read_data(int timeout_ms)
{
	RH_STRING result;
	result.set_ok();
	result.set_value("");

	std::string received_string = "";

	if (!is_connected)
	{
		result.set_not_ok(SOCKET_ERROR_NOT_CONNECTED);
	}
	else
	{
		bool uses_mask;
		unsigned bytes_received = 0;
		uint8_t *buffer;
		// \todo buffer size (1500) should NOT be hardcoded
		buffer = new uint8_t[1501];
		RH_INT fetch_result = fetch_data((char*)buffer, 1500, timeout_ms);
		if ((fetch_result.is_ok()) && (fetch_result.id() != SOCKET_RECEIVE_TIMEOUT))
		{
			bytes_received = fetch_result.value();
			wsheader_type ws;
			const uint8_t * data = (uint8_t *) &buffer[0];
			ws.fin = (data[0] & 0x80) == 0x80;
			ws.opcode = (wsheader_type::opcode_type) (data[0] & 0x0f);
			ws.mask = (data[1] & 0x80) == 0x80;
			uses_mask = ws.mask;
			ws.N0 = (data[1] & 0x7f);
			ws.header_size = 2 + (ws.N0 == 126? 2 : 0) + (ws.N0 == 127? 6 : 0) + (ws.mask? 4 : 0);
			//if (rxbuf.size() < ws.header_size) { return; /* Need: ws.header_size - rxbuf.size() */ }
			int i;
			if (ws.N0 < 126)
			{
				ws.N = ws.N0;
				i = 2;
			}
			else if (ws.N0 == 126)
			{
				ws.N = 0;
				ws.N |= ((uint64_t) data[2]) << 8;
				ws.N |= ((uint64_t) data[3]) << 0;
				i = 4;
			}
			else if (ws.N0 == 127)
			{
				ws.N = 0;
				ws.N |= ((uint64_t) data[2]) << 56;
				ws.N |= ((uint64_t) data[3]) << 48;
				ws.N |= ((uint64_t) data[4]) << 40;
				ws.N |= ((uint64_t) data[5]) << 32;
				ws.N |= ((uint64_t) data[6]) << 24;
				ws.N |= ((uint64_t) data[7]) << 16;
				ws.N |= ((uint64_t) data[8]) << 8;
				ws.N |= ((uint64_t) data[9]) << 0;
				i = 10;
			}
			if (ws.mask)
			{
				ws.masking_key[0] = ((uint8_t) data[i+0]) << 0;
				ws.masking_key[1] = ((uint8_t) data[i+1]) << 0;
				ws.masking_key[2] = ((uint8_t) data[i+2]) << 0;
				ws.masking_key[3] = ((uint8_t) data[i+3]) << 0;
			}
			else
			{
				ws.masking_key[0] = 0;
				ws.masking_key[1] = 0;
				ws.masking_key[2] = 0;
				ws.masking_key[3] = 0;
			}
			//if (rxbuf.size() < ws.header_size+ws.N) { return; /* Need: ws.header_size+ws.N - rxbuf.size() */ }
#			ifdef DEBUG
			if (verbosity >= VOUT_DEBUG2)
			{
				vout(VOUT_DEBUG2) << "[" << whoami() << "] received data ";
				if (ws.mask)
				{
					vout(VOUT_DEBUG2) << "(mask=on): ";
					uint8_t *data_ptr = buffer;
					int debug_i;
					char hex[6];
					for (debug_i = 0; debug_i < bytes_received; debug_i++)
					{
						sprintf(hex, "0x%02x ", *(data_ptr++));
						vout(VOUT_DEBUG2) << hex;
					}
				}
				else
				{
					vout(VOUT_DEBUG2) << ": ";
					uint8_t *data_ptr = buffer;
					int debug_i;
					char hex[6];
					for (debug_i = 0; debug_i < ws.header_size; debug_i++)
					{
						sprintf(hex, "0x%02x ", *(data_ptr++));
						vout(VOUT_DEBUG2) << hex;
					}
					vout(VOUT_DEBUG2) << "\"";
					for (debug_i = ws.header_size; debug_i < bytes_received; debug_i++)
					{
						vout(VOUT_DEBUG2) << *(data_ptr++);
					}
					vout(VOUT_DEBUG2) << "\"";
				}
			}
#			endif

			if (ws.opcode == wsheader_type::TEXT_FRAME && ws.fin)
			{
				if (ws.mask)
				{
					size_t i;
					for (i = 0; i != ws.N; ++i)
					{
						buffer[i + ws.header_size] ^= ws.masking_key[i & 0x3];
					}
				}
				*(buffer + ws.header_size + ws.N) = '\0';
				received_string = std::string((char*)buffer + ws.header_size);
				size_t key_index = i & 0x03;
				while (bytes_received < (ws.header_size + ws.N))
				{
					// We've not received the entire message yet.
					// Loop-read until we have.
					fetch_result = fetch_data((char*)buffer, 1500, timeout_ms);
					if (fetch_result.is_ok())
					{
						bytes_received += fetch_result.value();
						if (ws.mask)
						{
							for (size_t i = 0; i != fetch_result.value(); ++i)
							{
								buffer[i] ^= ws.masking_key[key_index++];
								key_index &= 0x03;
							}
						}
						*(buffer + fetch_result.value()) = '\0';
						received_string += std::string((char*)buffer);
					}
#					ifdef DEBUG
					if (verbosity >= VOUT_DEBUG2)
					{
						if (ws.mask)
						{
							uint8_t *data_ptr = buffer;
							int debug_i;
							char hex[6];
							for (debug_i = 0; debug_i < bytes_received; debug_i++)
							{
								sprintf(hex, "0x%02x ", *(data_ptr++));
								vout(VOUT_DEBUG2) << hex;
							}
						}
						else
						{
							vout(VOUT_DEBUG2) << ": ";
							uint8_t *data_ptr = buffer;
							int debug_i;
							for (debug_i = ws.header_size; debug_i < bytes_received; debug_i++)
							{
								vout(VOUT_DEBUG2) << *(data_ptr++);
							}
						}
					}
#					endif
				}
			}
			else if (ws.opcode == wsheader_type::PING) { }
			else if (ws.opcode == wsheader_type::PONG) { }
//			else if (ws.opcode == wsheader_type::CLOSE) { close(); }
			else
			{
				result.set_not_ok(SOCKET_ERROR_INVALID_MESSAGE);
			}

		}

		if ((fetch_result.is_ok()) && (fetch_result.id() != SOCKET_RECEIVE_TIMEOUT))
		{
#			ifdef DEBUG
			if (verbosity >= VOUT_DEBUG2)
			{
				if (!uses_mask)
				{
					vout(VOUT_DEBUG2) << "\"";
				}
				vout(VOUT_DEBUG2) << std::endlc;
			}
#			endif
			result.set_value(received_string);
		}
		else
		{
			result.set_not_ok(fetch_result.id());
		}
		delete[] buffer;
	}



	return(result);
}

/*! \brief Thread that dispatches incoming message events.
 *
 * This thread is started by calling #start_websocket_client_listener().
 * Incoming messages are dispatched to the specified
 * \ref websocket_callback "callback" function.
 *
 * The proper way to terminate this thread is to call #stop_websocket_client_listener().
 *
 */
void websocket::thread_entry()
{
	vout(VOUT_DEBUG) << "[" << whoami() << "] listener thread started" << std::endlc;
	RH_STRING rcv;
	websocket_client_listener_running = true;

	while (websocket_client_listener_running)
	{
		rcv = read_data(250);
		if (rcv.is_ok())
		{
			vout(VOUT_DEBUG) << "[" << whoami() << "] received string \"" << ascii_safe(rcv.value(), true) << "\"" << std::endlc;
			websocketclient_callback(this, rcv.value());
		}
	}
	vout(VOUT_DEBUG) << "[" << whoami() << "] exiting listener thread" << std::endlc;
}

/*! \brief Starts a thread that listens for incoming messages
 *
 * It starts a separate \ref thread_entry() "thread" that does all the work. It
 * does not return until it has verified that the listener thread actually is running.
 *
 * \param callback Pointer to callback function that handles incoming messages
 * \return Always returns #NO_ERRORS
 */
RH websocket::start_websocket_client_listener(websocket_callback callback)
{
	RH result;
	result.set_ok();

	websocketclient_callback = callback;

	websocket_client_listener_running = false;
	run();
	while (!websocket_client_listener_running);


	return result;
}

/*! \brief Stops a running message listener.
 * It does not return until the listener actually has stopped.
 * \return Always returns #NO_ERRORS
 */
RH websocket::stop_websocket_client_listener()
{
	RH result;
	result.set_ok();

	websocket_client_listener_running = false;
	wait();

	return result;
}

/*! \brief Sends a message over the websocket.
 *
 * The proper header is added, and the message is "scrambled" (masked)
 * if #use_mask is set to TRUE.
 *
 * \param message Message to send
 * \return #NO_ERRORS if all is OK, otherwise either
 *         #SOCKET_ERROR_NOT_CONNECTED or #SOCKET_ERROR.
 */
RH websocket::send(std::string message)
{
	RH result;
	result.set_ok();

	if (!is_connected)
	{
		result.set_not_ok(SOCKET_ERROR_NOT_CONNECTED);
		return(result);
	}

	// TODO:
	// Masking key should (must) be derived from a high quality random
	// number generator, to mitigate attacks on non-WebSocket friendly
	// middleware:
	const uint8_t masking_key[4] = { 0x12, 0x34, 0x56, 0x78 };

	std::vector<uint8_t> header;
	uint64_t message_size = message.size();
	header.assign(2 + (message_size >= 126 ? 2 : 0) + (message_size >= 65536 ? 6 : 0) + (use_mask ? 4 : 0), 0);
	header[0] = 0x80 | wsheader_type::TEXT_FRAME;
	if (message_size < 126)
	{
		header[1] = (message_size & 0xff) | (use_mask ? 0x80 : 0);
		if (use_mask)
		{
			header[2] = masking_key[0];
			header[3] = masking_key[1];
			header[4] = masking_key[2];
			header[5] = masking_key[3];
		}
	}
	else if (message_size < 65536)
	{
		header[1] = 126 | (use_mask ? 0x80 : 0);
		header[2] = (message_size >> 8) & 0xff;
		header[3] = (message_size >> 0) & 0xff;
		if (use_mask)
		{
			header[4] = masking_key[0];
			header[5] = masking_key[1];
			header[6] = masking_key[2];
			header[7] = masking_key[3];
		}
	}
	else
	{
	// TODO: run coverage testing here
		header[1] = 127 | (use_mask ? 0x80 : 0);
		header[2] = (message_size >> 56) & 0xff;
		header[3] = (message_size >> 48) & 0xff;
		header[4] = (message_size >> 40) & 0xff;
		header[5] = (message_size >> 32) & 0xff;
		header[6] = (message_size >> 24) & 0xff;
		header[7] = (message_size >> 16) & 0xff;
		header[8] = (message_size >>  8) & 0xff;
		header[9] = (message_size >>  0) & 0xff;
		if (use_mask)
		{
			header[10] = masking_key[0];
			header[11] = masking_key[1];
			header[12] = masking_key[2];
			header[13] = masking_key[3];
		}
	}

	uint8_t *data = new uint8_t[header.size() + message.length()];
	uint8_t *data_ptr = data;
	unsigned header_length = 0;
	unsigned data_length = 0;

	std::vector<uint8_t>::iterator header_itr = header.begin();
	while (header_itr != header.end())
	{
		*(data_ptr++) = *(header_itr++);
		header_length += 1;
		data_length += 1;
	}
	if (!use_mask)
	{
		std::string::iterator message_itr = message.begin();
		while (message_itr != message.end())
		{
			*(data_ptr++) = *(message_itr++);
			data_length += 1;
		}
	}
	else
	{
		std::string::iterator message_itr = message.begin();
		int key_index = 0;
		while (message_itr != message.end())
		{
			*(data_ptr++) = *(message_itr++) ^ masking_key[key_index];
			data_length += 1;
			key_index += 1;
			key_index &= 0x03;
		}
	}

#ifdef DEBUG
	if (verbosity >= VOUT_DEBUG2)
	{
		vout(VOUT_DEBUG2) << "[" << whoami() << "] sending data ";
		if (use_mask)
		{
			vout(VOUT_DEBUG2) << "(mask=on): ";
			data_ptr = data;
			int debug_i;
			char hex[6];
			for (debug_i = 0; debug_i < data_length; debug_i++)
			{
				sprintf(hex, "0x%02x ", *(data_ptr++));
				vout(VOUT_DEBUG2) << hex;
			}
		}
		else
		{
			vout(VOUT_DEBUG2) << ": ";
			data_ptr = data;
			int debug_i;
			char hex[6];
			for (debug_i = 0; debug_i < header_length; debug_i++)
			{
				sprintf(hex, "0x%02x ", *(data_ptr++));
				vout(VOUT_DEBUG2) << hex;
			}
			vout(VOUT_DEBUG2) << "\"";
			for (debug_i = header_length; debug_i < data_length; debug_i++)
			{
				vout(VOUT_DEBUG2) << *(data_ptr++);
			}
			vout(VOUT_DEBUG2) << "\"";
		}

		vout(VOUT_DEBUG2) << std::endlc;
	}
#endif

	if (::send(socket_handle, data, data_length, 0) == -1)
	{
		result.set_not_ok(SOCKET_ERROR);
	}

	delete[] data;
	return(result);
}

/*! \brief Returns full string uf current URL.
 * \return URL-string
 */
std::string websocket::url()
{
	std::string url = "ws://";
	url += destination_ + ":" + int_to_string(remote_port_) + remote_path_;
	return url;
}
