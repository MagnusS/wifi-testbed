/*! \file vout.hpp
 * \brief Simple output stream which allows level-restricted output.
 * \date 2010
 * \author   Forsvarets Forskningsinstitutt (FFI)\n
 *           http://www.ffi.no \n
 *           - Terje Mikal Mjelde [tmo]\n
 *             terje-mikal-mjelde@ffi.no
 */

#ifndef __VOUT_HPP
#define __VOUT_HPP

#include "platform.h"
#include "color_streams.h"

#include <iostream>

#ifdef VOUT_ALLOC
#	define I(x)      x
#	define EXPORTED  /* empty */
#else
#	define I(x)      /*empty */
#	define EXPORTED  extern
#endif


/*! \brief Predefined constants for easier classification of output levels.
*/
enum
{
	VOUT_QUIET = 0,
	VOUT_ERROR,
	VOUT_INFO,  // Default
	VOUT_VERBOSE,
	VOUT_VERBOSER,
	VOUT_VERBOSEST,
	VOUT_DEBUG,
	VOUT_DEBUG2,
	VOUT_END, VOUT_MAX = VOUT_END - 1
};

std::ostream& vout(int verbosity_level = VOUT_INFO, std::ostream& out = std::cout);

//struct out_t
//{
//    template<typename T>
//    out_t&
//    operator<< (T&& x)
//    {
//    	std::cout << x;
//    	return *this;
//    };
//};
//
//out_t vvout;
//
//
//template<typename _CharT, typename _Traits>
//    inline std::basic_ostream<_CharT, _Traits>&
//    vvout(std::basic_ostream<_CharT, _Traits>& __os)
//    { return __os << "DEBUG"; }

//template<typename _CharT, typename _Traits>
//	inline std::basic_ostream<_CharT, _Traits>&
//	endc(std::basic_ostream<_CharT, _Traits>& __os)
//	{ return __os << ansi::reset; }



/*! \brief Global variable which should be set to required output level.
*/
EXPORTED int verbosity I( = VOUT_INFO );

#undef I
#undef EXPORTED
#undef VOUT_ALLOC

#endif // __VOUT_HPP
