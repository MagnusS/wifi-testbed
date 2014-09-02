/*! \file timetools.cpp
 * \copydoc timetools.hpp
 */

#include "timetools.hpp"
#include <iostream>

timetools::timetools()
{
	next_handle = 0;
}

timetools::~timetools()
{
	timer p_timer = timers.begin();
	while (p_timer != timers.end())
	{
		timetools_data *tmp_ptr = *p_timer;
		timers.erase(p_timer);
		delete(tmp_ptr);

	}
}

timetools::handle timetools::add_countdown_timer(time_in_ms time_in_ms_, bool delete_after_use_)
{
	next_handle += 1;
	timetools_data *new_timer = new timetools_data;
	new_timer->handle_ = next_handle;
	new_timer->time_in_ms_ = time_in_ms_;
	new_timer->starttime_ = 0;
	new_timer->delete_after_use_ = delete_after_use_;
	timers.push_back(new_timer);

	return(next_handle);
}

RH timetools::set_countdown_timer_timeout(handle handle_, time_in_ms time_in_ms_)
{
	RH result;
	result.set_ok();

	timer timer_ = get_timer(handle_);
	if (timer_ != timers.end())
	{
		(*timer_)->starttime_ = time_in_ms_;
	}
	else
	{
		result = TIMETOOLS_ERROR_INVALID_HANDLE;
	}
	return(result);
}

RH timetools::start_countdown_timer(handle handle_)
{
	RH result;
	result.set_ok();

	timer timer_ = get_timer(handle_);
	if (timer_ != timers.end())
	{
		(*timer_)->starttime_ = now_in_ms();
	}
	else
	{
		result = TIMETOOLS_ERROR_INVALID_HANDLE;
	}
	return(result);
}

RH timetools::restart_countdown_timer(handle handle_)
{
	return(start_countdown_timer(handle_));
}

RH timetools::check_countdown_timer(handle handle_)
{
	RH result;
	result = TIMETOOLS_TIMER_RUNNING;
	timer timer_ = get_timer(handle_);
	if (timer_ != timers.end())
	{
		if (now_in_ms() >= ((*timer_)->starttime_ + (*timer_)->time_in_ms_))
		{
			//std::cout << "Timer handle" << handle_ << ": now=" << now_in_ms() << " starttime=" << (*timer_)->starttime_ << " timeout=" << (*timer_)->time_in_ms_ << " diff=" << now_in_ms() - (*timer_)->starttime_ << std::endl;
			result = TIMETOOLS_TIMER_EXPIRED;
			if ((*timer_)->delete_after_use_)
			{
				delete *timer_;
				timers.erase(timer_);
			}
		}
	}
	else
	{
		result = TIMETOOLS_ERROR_INVALID_HANDLE;
	}
	return(result);

}

bool timetools::countdown_timer_expired(handle handle_)
{
	return(check_countdown_timer(handle_) != TIMETOOLS_TIMER_RUNNING);
}

bool timetools::countdown_timer_running(handle handle_)
{
	return(check_countdown_timer(handle_) == TIMETOOLS_TIMER_RUNNING);
}

RH timetools::delete_countdown_timer(handle handle_)
{
	RH result;
	result.set_ok();

	timer timer_ = get_timer(handle_);
	if (timer_ != timers.end())
	{
		delete *timer_;
		timers.erase(timer_);
	}
	else
	{
		result = TIMETOOLS_ERROR_INVALID_HANDLE;
	}
	return(result);
}

timetools::handle timetools::add_stopwatch(bool delete_after_use_)
{
	next_handle += 1;
	timetools_data *new_timer = new timetools_data;
	new_timer->handle_ = next_handle;
	new_timer->time_in_ms_ = 0;
	new_timer->starttime_ = 0;
	new_timer->delete_after_use_ = delete_after_use_;
	timers.push_back(new_timer);

	return(next_handle);
}

RH timetools::start_stopwatch(handle handle_)
{
	RH result;
	result.set_ok();

	timer timer_ = get_timer(handle_);
	if (timer_ != timers.end())
	{
		(*timer_)->starttime_ = now_in_ms();
		(*timer_)->time_in_ms_ = 0;
//		std::cout << "Stopwatch " << handle_ << " start: starttime=" << (*timer_)->starttime_ << "  time_in_ms=" << (*timer_)->time_in_ms_ << std::endl;
	}
	else
	{
		result = TIMETOOLS_ERROR_INVALID_HANDLE;
	}
	return(result);
}

RH timetools::stop_stopwatch(handle handle_)
{
	RH result;
	result.set_ok();

	timer timer_ = get_timer(handle_);
	if (timer_ != timers.end())
	{
		(*timer_)->time_in_ms_ = now_in_ms() - (*timer_)->starttime_;
//		std::cout << "Stopwatch " << handle_ << " stop: starttime=" << (*timer_)->starttime_ << "  time_in_ms=" << (*timer_)->time_in_ms_ << std::endl;
	}
	else
	{
		result = TIMETOOLS_ERROR_INVALID_HANDLE;
	}
	return(result);
}

RH timetools::restart_stopwatch(handle handle_)
{
	RH result;
	result.set_ok();

	timer timer_ = get_timer(handle_);
	if (timer_ != timers.end())
	{
//		std::cout << "Stopwatch " << handle_ << " restart (before): starttime=" << (*timer_)->starttime_ << "  time_in_ms=" << (*timer_)->time_in_ms_ << std::endl;
		(*timer_)->starttime_ = now_in_ms();
		(*timer_)->time_in_ms_ = 0;
//		std::cout << "Stopwatch " << handle_ << " restart (after): starttime=" << (*timer_)->starttime_ << "  time_in_ms=" << (*timer_)->time_in_ms_ << std::endl;
	}
	else
	{
		result = TIMETOOLS_ERROR_INVALID_HANDLE;
	}
	return(result);
}

RH_UINT timetools::get_stopwatch_elapsed_time_in_ms(handle handle_)
{
	RH_UINT result;
	result.set_ok();
	timer timer_ = get_timer(handle_);
	if (timer_ != timers.end())
	{
		if ((*timer_)->time_in_ms_ == 0)
		{
			// Stopwatch hasn't been stopped, so return current time - starttime
			result.set_value((unsigned)(now_in_ms() - (*timer_)->starttime_));
//			std::cout << "Stopwatch " << handle_ << " gettime (running): starttime=" << (*timer_)->starttime_ << "  time_in_ms=" << (*timer_)->time_in_ms_ << "  elapsedtime=" << result.value( )<< std::endl;
		}
		else
		{
//			std::cout << "Stopwatch " << handle_ << " gettime (stopped): starttime=" << (*timer_)->starttime_ << "  time_in_ms=" << (*timer_)->time_in_ms_ << std::endl;
			result.set_value((*timer_)->time_in_ms_);
		}
	}
	else
	{
		result = TIMETOOLS_ERROR_INVALID_HANDLE;
	}
	return(result);
}

RH timetools::delete_stopwatch(handle handle_)
{
	RH result;
	result.set_ok();

	timer timer_ = get_timer(handle_);
	if (timer_ != timers.end())
	{
		delete *timer_;
		timers.erase(timer_);
	}
	else
	{
		result = TIMETOOLS_ERROR_INVALID_HANDLE;
	}
	return(result);
}




timetools::timer timetools::get_timer(handle handle_)
{
	timer timer_ = timers.begin();
	while ((timer_ != timers.end()) && ((*timer_)->handle_ != handle_))
	{
		timer_ += 1;
	}
	return(timer_);
}

timetools::time_in_ms timetools::now_in_ms()
{
#if PLATFORM == WINDOWS
	return((time_in_ms)(::GetTickCount()));
#endif
#if PLATFORM == LINUX
	struct timeval tv;
	::gettimeofday(&tv, NULL);
	return((time_in_ms)((tv.tv_sec) * 1000 + (tv.tv_usec) / 1000));
#endif
}


