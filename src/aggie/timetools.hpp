/*
/*! \file aggie.hpp
 *
 * \brief A class containing stopwatch and a countdown timer.
 *
 * \date 2013
 * \author   Forsvarets Forskningsinstitutt (FFI)\n
 *           http://www.ffi.no \n
 *           - Terje Mikal Mjelde [tmo]\n
 *             terje-mikal-mjelde@ffi.no
 */

#ifndef __TIMETOOLS_HPP
#define __TIMETOOLS_HPP

#include "platform.h"

#include <vector>
#include "resulthandler.hpp"

#if PLATFORM == WINDOWS
#include <windows.h>
#endif
#if PLATFORM == LINUX
#include <sys/time.h>
#endif

class timetools
{
public:
	timetools();
	virtual ~timetools();
	typedef unsigned int handle;
	typedef unsigned int time_in_ms;

	// Countdown timers
	handle add_countdown_timer(time_in_ms, bool delete_after_use = false);
	RH set_countdown_timer_timeout(handle, time_in_ms);
	RH start_countdown_timer(handle);
	RH restart_countdown_timer(handle);
	RH check_countdown_timer(handle);
	RH delete_countdown_timer(handle);
	bool countdown_timer_expired(handle);
	bool countdown_timer_running(handle);

	// Stopwatch
	handle add_stopwatch(bool delete_after_use = false);
	RH start_stopwatch(handle);
	RH stop_stopwatch(handle);
	RH restart_stopwatch(handle);
	RH_UINT get_stopwatch_elapsed_time_in_ms(handle);
	RH delete_stopwatch(handle);

private:
	handle next_handle;;
	typedef struct
	{
		handle handle_;
		time_in_ms time_in_ms_;
		time_in_ms starttime_;
		bool delete_after_use_;
	} timetools_data;
	std::vector<timetools_data*> timers;
	typedef std::vector<timetools_data*>::iterator timer;
	timer get_timer(handle);
	time_in_ms now_in_ms();
};

#endif /* __TIMETOOLS_HPP */
