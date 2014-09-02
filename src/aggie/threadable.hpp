/*! \file threadable.hpp
 *
 *  \brief Thread interface
 *
 * \date 2013
 * \author   Forsvarets Forskningsinstitutt (FFI)\n
 *           http://www.ffi.no \n
 *           - Terje Mikal Mjelde [tmo]\n
 *             terje-mikal-mjelde@ffi.no
 */


#ifndef THREADABLE_HPP_
#define THREADABLE_HPP_

#include "platform.h"

#ifdef PLATFORM_WINDOWS

#endif
#ifdef PLATFORM_LINUX
#	include <pthread.h>
#endif


/*! \brief Makes subclasses threadable.
 * \todo Make it Windows-compatible
 */
class threadable
{
public:
   threadable() : _thread(0) {/* empty */}
   virtual ~threadable() {/* empty */}

   /** Returns true if the thread was successfully started,
    * false if there was an error starting the thread
    */
   bool run()
   {
      return (pthread_create(&_thread, NULL, thread_entry_func, this) == 0);
   }

   /** Will not return until the internal thread has exited. */
   void wait()
   {
      (void) pthread_join(_thread, NULL);
   }

protected:
   /** Implement this method in your subclass with the
    * code you want your thread to run.
    */
   virtual void thread_entry() = 0;

private:
   static void * thread_entry_func(void * This)
   {
	   ((threadable *)This)->thread_entry(); return NULL;
   }

   pthread_t _thread; //!< Thread handle
};

#endif /* THREADABLE_HPP_ */
