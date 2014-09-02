/*! \file platform.h
 * \brief Platform definitions.
 *
 * These values are assigned (usually by the Makefile) to the PLATFORM
 * pre-compiler definition, which is used for differentiating between
 * platform-specific code.
 */

#if !defined __PLATFORM_H
#define __PLATFORM_H

#define WINDOWS 1
#define LINUX   2

#ifndef PLATFORM

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__WIN32__)
#define PLATFORM_WINDOWS
#endif
#if defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
#define PLATFORM_LINUX
#endif


#endif // PLATFORM


#if PLATFORM == WINDOWS
#define PLATFORM_WINDOWS
#endif
#if PLATFORM == LINUX
#define PLATFORM_LINUX
#endif

#endif // __PLATFORM_H
