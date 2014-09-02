/*! \file cmdline.cpp
 *
 * \brief Class for parsing command line arguments.
 *
 * Supports arguments of the following kind:
 * - -h ( = short form)
 * - \-\-help ( = long form)
 * - -n 7 ( = argument with a value)
 *
 * Supports the following value types:
 * - int/uint
 * - long/ulong
 * - float/double
 * - std::string
 *
 * Arguments that does not take a value are called \e flags.
 *
 * A minimum and maximum count of each arguments can be given.
 *
 * Callback functions can be defined, which are called automatically
 * when the associated argument has been used.
 *
 * \date 2010
 * \author   Forsvarets Forskningsinstitutt (FFI)\n
 *           http://www.ffi.no \n
 *           - Terje Mikal Mjelde [tmo]\n
 *             terje-mikal-mjelde@ffi.no
 */

#ifndef __CMDLINE_HPP
#define __CMDLINE_HPP


#include "platform.h"
#include "stringutils.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>

//! \brief Class for parsing command line arguments.
//! \copydetails cmdline.cpp
class cmdline
{
public:
	cmdline();
	cmdline(int argc, char **argv);
	~cmdline();
	void destroy();
	void common_constructor();
	void enable_halt_on_error();
	void disable_halt_on_error();
	void set(int argc, char **argv);
	void print_usage();
	void print_errors();
	//! \brief Generic superclass for all token subclasses.
	class arg
	{
		friend class cmdline;
	public:
		virtual ~arg() {};
		int count();
		int min_count();
		int max_count();
		std::string short_token();
		std::string long_token();
		std::string description();
		std::string error();
		bool visible();
		virtual int requires_value() = 0;
		virtual int can_have_negative_number() = 0;
		void set_callback(void (*)(cmdline*));
	protected:
		std::string generic_errors();
		int count_; //!< How many of this token are there on the command line
		int min_count_; //!< How many \e must there be (minimum occurences)
		int max_count_; //!< Maximum allowed occurences
		std::string short_token_; //!< Short token (without dash)
		std::string long_token_; //!< Long token (without dashes)
		std::string description_; //!< Help text
		std::vector<std::string> errors; //!< List of errors found for this token
		virtual int add_value(std::string new_value) = 0; //!< Add new value from command line
		void (*callback_function)(cmdline*); //!< Pointer to callback function, or NULL if non-existent.
		bool visible_;
	};
	//! \brief Token subclass for tokens that takes an integer as parameter.
	class arg_int : public arg
	{
	public:
		int operator[](int i); //!< Returns value at index \c i
		int value(); //!< Returns first value found (same as \c arg_int[0])
		int requires_value(); //!< Returns TRUE
		int can_have_negative_number(); //!< Returns TRUE
	protected:
		int add_value(std::string new_value);
	private:
		std::vector<int> value_; //!< Vector of values given on command line
	};
	//! \brief Token subclass for tokens that takes an unsigned integer as parameter.
	class arg_uint : public arg
	{
	public:
		unsigned int operator[](int i); //!< Returns value at index \c i
		unsigned int value(); //!< Returns first value found (same as \c arg_int[0])
		int requires_value(); //!< Returns TRUE
		int can_have_negative_number(); //!< Returns FALSE
	protected:
		int add_value(std::string new_value);
	private:
		std::vector<unsigned int> value_; //!< Vector of values given on command line
	};
	//! \brief Token subclass for tokens that takes a long as parameter.
	class arg_long : public arg
	{
	public:
		long operator[](int i); //!< Returns value at index \c i
		long value(); //!< Returns first value found (same as \c arg_int[0])
		int requires_value(); //!< Returns TRUE
		int can_have_negative_number(); //!< Returns TRUE
	protected:
		int add_value(std::string new_value);
	private:
		std::vector<long> value_; //!< Vector of values given on command line
	};
	//! \brief Token subclass for tokens that takes an unsigned long as parameter.
	class arg_ulong : public arg
	{
	public:
		unsigned long operator[](int i); //!< Returns value at index \c i
		unsigned long value(); //!< Returns first value found (same as \c arg_int[0])
		int requires_value(); //!< Returns TRUE
		int can_have_negative_number(); //!< Returns FALSE
	protected:
		int add_value(std::string new_value);
	private:
		std::vector<unsigned long> value_; //!< Vector of values given on command line
	};
	//! \brief Token subclass for tokens that takes a std::string as parameter.
	class arg_string : public arg
	{
	public:
		std::string operator[](int i); //!< Returns value at index \c i
		std::string value(); //!< Returns first value found (same as \c arg_int[0])
		int requires_value(); //!< Returns TRUE
		int can_have_negative_number(); //!< Returns FALSE
	protected:
		int add_value(std::string new_value);
	private:
		std::vector<std::string> value_; //!< Vector of values given on command line
	};
	//! \brief Token subclass for tokens that takes a float as parameter.
	class arg_float : public arg
	{
	public:
		float operator[](int i); //!< Returns value at index \c i
		float value(); //!< Returns first value found (same as \c arg_int[0])
		int requires_value(); //!< Returns TRUE
		int can_have_negative_number(); //!< Returns TRUE
	protected:
		int add_value(std::string new_value);
	private:
		std::vector<float> value_; //!< Vector of values given on command line
	};
	//! \brief Token subclass for tokens that takes a double as parameter.
	class arg_double : public arg
	{
	public:
		double operator[](int i); //!< Returns value at index \c i
		double value(); //!< Returns first value found (same as \c arg_int[0])
		int requires_value(); //!< Returns TRUE
		int can_have_negative_number(); //!< Returns TRUE
	protected:
		int add_value(std::string new_value);
	private:
		std::vector<double> value_; //!< Vector of values given on command line
	};
	//! \brief Token subclass for tokens that takes no parameters.
	class arg_flag : public arg
	{
	public:
		int requires_value(); //!< Returns FALSE
		int can_have_negative_number(); //!< Returns FALSE
		bool is_set(); //!< Returns TRUE if at least given onve
	protected:
		int add_value(std::string new_value);
	private:
	};
	arg_int *add_token_int(
		std::string s_token,
		std::string l_token,
		int min_count = 0, int max_count = -1,
		std::string description = "",
		bool visible = true
		);
	arg_uint *add_token_uint(
		std::string s_token,
		std::string l_token,
		int min_count = 0, int max_count = -1,
		std::string description = "",
		bool visible = true
		);
	arg_long *add_token_long(
		std::string s_token,
		std::string l_token,
		int min_count = 0, int max_count = -1,
		std::string description = "",
		bool visible = true
		);
	arg_ulong *add_token_ulong(
		std::string s_token,
		std::string l_token,
		int min_count = 0, int max_count = -1,
		std::string description = "",
		bool visible = true
		);
	arg_string *add_token_string(
		std::string s_token,
		std::string l_token,
		int min_count = 0, int max_count = -1,
		std::string description = "",
		bool visible = true
		);
	arg_float *add_token_float(
		std::string s_token,
		std::string l_token,
		int min_count = 0, int max_count = -1,
		std::string description = "",
		bool visible = true
		);
	arg_double *add_token_double(
		std::string s_token,
		std::string l_token,
		int min_count = 0, int max_count = -1,
		std::string description = "",
		bool visible = true
		);
	arg_flag *add_token_flag(
		std::string s_token,
		std::string l_token,
		int min_count = 0, int max_count = -1,
		std::string description = "",
		bool visible = true
		);

	int parse();
	std::vector<std::string> main_arguments();
	int has_errors();
	std::vector<std::string> error_messages();
private:
	void set_token_data(
		arg *new_token,
		std::string s_token,
		std::string l_token,
		int min_count, int max_count,
		std::string description,
		bool visible
		);
	arg *find_token(std::string token);

	/*! \brief Vector containing pointers to all recognized (and ergo parsable)
	 * arguments that can be given on the command line.
	 *
	 * Actually, they point to instances of the \arg class, which is
	 * a superclass for the relevant subclass containing token-specific
	 * information.
    */
	std::vector<arg*> argtable;

	/*! \brief Vector containing all unrecognized arguments given on
	 * the command line.
    */
	std::vector<std::string> loose_tokens;

	/*! \brief Vector containing all strings given on
	 * the command line and which are not tied to an option.
    */
	std::vector<std::string> loose_args;

	/*! \brief Vector containing all options and arguments given on the command
	 * line as separate strings.
	 */
	std::vector<std::string> command_line;

	std::string trim(std::string&);

	/*! \brief Vector containing all parsing errors found.
    */
	std::vector<std::string> errors;

	int validate();
	void execute_callbacks();

	/*! \brief Flag that indicates how invalid command line arguments
	 * should be handled.
	 *
	 * If set, the class will print any errors to stderr and halt execution.
	 * Otherwise, errors must be handled by the main program itself.
    */
	int halt_on_errors;
};

#endif // __CMDLINE_HPP

