/*! \file stringutils.hpp
 *
 * \brief Useful string functions.
 *
 * \date 2010
 * \author   Forsvarets Forskningsinstitutt (FFI)\n
 *           http://www.ffi.no \n
 *           - Terje Mikal Mjelde [tmo]\n
 *             terje-mikal-mjelde@ffi.no
 */

#ifndef __STRINGUTILS_HPP
#define __STRINGUTILS_HPP

#include <string>
#include <iostream>
#include <sstream>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

int string_to_int(std::string, int&);
int string_to_unsigned(std::string, unsigned&);
int string_to_long(std::string, long&);
int string_to_float(std::string, float&);
int string_to_double(std::string, double&);
std::string int_to_string(int, std::string&);
std::string int_to_string(int);
std::string long_to_string(long, std::string&);
std::string float_to_string(float, std::string&);
std::string double_to_string(double, std::string&);
std::string repeat_pattern(int count, std::string pattern);
std::string ltrim(std::string&, const char *chars_to_remove = " \t\n");
std::string rtrim(std::string&, const char *chars_to_remove = " \t\n");
std::string trim(std::string&, const char *chars_to_remove = " \t\n");
std::string to_upper(std::string);
std::string to_lower(std::string);
std::string format_string(std::string, ...);
std::string format_string(std::string*, ...);
std::string ascii_safe(std::string str, bool print_unsafe);

#endif // __STRINGUTILS_HPP

