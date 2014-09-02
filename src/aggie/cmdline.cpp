/*! \file cmdline.cpp
 * \copydoc cmdline.hpp
 */

#include "cmdline.hpp"

/*! \brief Common initialization called by all constructors.
*/
void cmdline::common_constructor()
{
	enable_halt_on_error();
}

/*! \brief Default constructor.
*/
cmdline::cmdline()
{
	common_constructor();
}

/*! \brief Destructor.
*/
cmdline::~cmdline()
{
	destroy();
}

/*! \brief Free all memory
*/
void cmdline::destroy()
{
	std::vector<arg*>::iterator p_arg = argtable.begin();
	while (p_arg != argtable.end())
	{
		arg *tmp_ptr = *p_arg;
		argtable.erase(p_arg);
		delete(tmp_ptr);
	}
}

/*! \brief Constructor which points the class to the actual command line
 * arguments given.
 * \param argc The same argc as given to the \ref main() function
 * \param argv The same argv as given to the \ref main() function
*/
cmdline::cmdline(int argc, char **argv)
{
	common_constructor();
	set(argc, argv);
}

/*! \brief Enables the \ref halt_on_errors flag.
*/
void cmdline::enable_halt_on_error()
{
	halt_on_errors = 1;
}

/*! \brief Disables the \ref halt_on_errors flag.
*/
void cmdline::disable_halt_on_error()
{
	halt_on_errors = 0;
}

/*! \brief Copies command-line arguments to internal buffer.
 * \param argc The same argc as given to the \ref main() function
 * \param argv The same argv as given to the \ref main() function
*/
void cmdline::set(int argc, char **argv)
{
	command_line.clear();
	for (int i=1; i<argc; i++)
	{
		command_line.push_back(std::string(argv[i]));
	}
}

/*! \brief Prints any parsing errors (i.e. unrecognized arguments)
 * to stderr.
*/
void cmdline::print_errors()
{
	if (has_errors())
	{
		std::vector<std::string> errors = error_messages();
		std::vector<std::string>::iterator p_error = errors.begin();
		while(p_error != errors.end())
		{
			std::cerr << *p_error << std::endl;
			p_error += 1;
		}
	}
}

/*! \brief Prints usage instruction as defined in arguments table to stdout.
 * \todo Add support for \\n newline in strings.
*/
void cmdline::print_usage()
{
	std::vector<arg*>::iterator p_arg = argtable.begin();
	while (p_arg != argtable.end())
	{
		if ((*p_arg)->visible())
		{
			std::string indent = "  ";
			std::string st = (*p_arg)->short_token_;
			if (st == "-") { st = ""; }
			std::string lt = (*p_arg)->long_token_;
			if (lt == "--") { lt = ""; }
			std::string tokens = st;
			if ((st != "") && (lt != "")) { tokens += ","; }
			tokens += lt;
			std::string spaces = "";
			for (int i=20; i>tokens.length(); i--) { spaces += " "; }
			std::cout << indent << tokens << spaces;
			if (spaces.length()==0)
			{
				std::cout << std::endl << indent << "                  ";
			}
			std::cout << (*p_arg)->description_ << std::endl;
		}
		p_arg += 1;
	}
}

/*! \brief Initializes a token with specified values.
 * \param new_token Pointer to token
 * \param s_token Short token name (e.g. "x" for "-x")
 * \param l_token Long token name (e.g. "help" for "--help")
 * \param min_count Minimum required occurences (0 means argument is optional,
 * 1 means it is required. Higher values are allowed)
 * \param max_count Maximum allowed occurences (must be equal or higher
 * than \c min_count)
 * \param description Short description of the purpose of the argument. This
 * is displayed by the \ref print_usage() function.
 * \param visible Specifies if this entry should be listed by #print_usage()
*/
void cmdline::set_token_data(arg *new_token,
                                 std::string s_token, std::string l_token,
                                 int min_count, int max_count,
                                 std::string description, bool visible)
{
	new_token->count_       = 0;
	new_token->callback_function = NULL;
	new_token->min_count_   = min_count;
	new_token->max_count_   = max_count;
	new_token->short_token_ = (s_token.length() > 0 ? "-"+s_token : "");
	new_token->long_token_  = (l_token.length() > 0 ? "--"+l_token : "");
	new_token->description_ = description;
	new_token->visible_     = visible;
}

/*! \brief Adds a new \e int token to the list of recognized tokens.
 * \param s_token Short token name (e.g. "x" for "-x")
 * \param l_token Long token name (e.g. "help" for "--help")
 * \param min_count Minimum required occurences (0 means argument is optional,
 * 1 means it is required. Higher values are allowed)
 * \param max_count Maximum allowed occurences (must be equal to or higher
 * than \c min_count, or -1 for infinite maximum)
 * \param description Short description of the purpose of the argument. This
 * is displayed by the \ref print_usage() function.
 * \param visible Specifies if this entry should be listed by #print_usage()
 * \return Pointer to the newly instantiated class containing token information.
*/
cmdline::arg_int *cmdline::add_token_int(std::string s_token, std::string l_token,
                                 int min_count, int max_count,
                                 std::string description, bool visible)

{
	arg_int *new_token = new(arg_int);
	set_token_data(new_token, s_token, l_token, min_count, max_count, description, visible);
	argtable.push_back(new_token);
	return(new_token);
}

/*! \brief Adds a new \e long token to the list of recognized tokens.
 * \copydetails add_token_int
 */
cmdline::arg_long *cmdline::add_token_long(std::string s_token, std::string l_token,
                                 int min_count, int max_count,
                                 std::string description, bool visible)

{
	arg_long *new_token = new(arg_long);
	set_token_data(new_token, s_token, l_token, min_count, max_count, description, visible);
	argtable.push_back(new_token);
	return(new_token);
}

/*! \brief Adds a new \e uint token to the list of recognized tokens.
 * \copydetails add_token_int
 */
cmdline::arg_uint *cmdline::add_token_uint(std::string s_token, std::string l_token,
                                 int min_count, int max_count,
                                 std::string description, bool visible)

{
	arg_uint *new_token = new(arg_uint);
	set_token_data(new_token, s_token, l_token, min_count, max_count, description, visible);
	argtable.push_back(new_token);
	return(new_token);
}

/*! \brief Adds a new \e float token to the list of recognized tokens.
 * \copydetails add_token_int
 */
cmdline::arg_float *cmdline::add_token_float(std::string s_token, std::string l_token,
                                 int min_count, int max_count,
                                 std::string description, bool visible)

{
	arg_float *new_token = new(arg_float);
	set_token_data(new_token, s_token, l_token, min_count, max_count, description, visible);
	argtable.push_back(new_token);
	return(new_token);
}

/*! \brief Adds a new \e double token to the list of recognized tokens.
 * \copydetails add_token_int
 */
cmdline::arg_double *cmdline::add_token_double(std::string s_token, std::string l_token,
                                 int min_count, int max_count,
                                 std::string description, bool visible)

{
	arg_double *new_token = new(arg_double);
	set_token_data(new_token, s_token, l_token, min_count, max_count, description, visible);
	argtable.push_back(new_token);
	return(new_token);
}

/*! \brief Adds a new \e std::string token to the list of recognized tokens.
 * \copydetails add_token_int
 */
cmdline::arg_string *cmdline::add_token_string(std::string s_token, std::string l_token,
                                 int min_count, int max_count,
                                 std::string description, bool visible)

{
	arg_string *new_token = new(arg_string);
	set_token_data(new_token, s_token, l_token, min_count, max_count, description, visible);
	argtable.push_back(new_token);
	return(new_token);
}

/*! \brief Adds a new \e flag token to the list of recognized tokens.
 * \copydetails add_token_int
 */
cmdline::arg_flag *cmdline::add_token_flag(std::string s_token, std::string l_token,
                                 int min_count, int max_count,
                                 std::string description, bool visible)

{
	arg_flag *new_token = new(arg_flag);
	set_token_data(new_token, s_token, l_token, min_count, max_count, description, visible);
	argtable.push_back(new_token);
	return(new_token);
}

/*! \brief Parses the command line arguments.
 *
 * - Updates the internal state of any recognized tokens (switches) found.
 * - Adds arguments without dash (- or --) prefixes to the \ref loose_tokens
 * vector.
 * - Generates error strings (if needed) and adds them to the \ref errors vector.
 * - Validates all arguments according to the rules given by the \c
 * add_token_* methods-
 * - If no errors are found, calls any callback functions that have been
 * defined.
*/
int cmdline::parse()
{
	int success = 1;
	int do_not_advance = 0;

	arg *current_token = NULL;
	std::string previous_arg = "";

	if (!argtable.empty())
	{
		std::vector<std::string>::iterator p_cmdl = command_line.begin();
		while (p_cmdl != command_line.end())
		{
			// Iterate through all given command line options
			std::string current_arg = *p_cmdl;
			if (current_arg[0] == '-')
			{
				// Current option is a switch
				if (current_token == NULL)
				{
					// Previous option was not a switch
					current_token = find_token(current_arg);
					if (!current_token)
					{
						// Unknown switch
						loose_tokens.push_back(current_arg);
						success = 0;
					}
					else
					{
						// Previous option was a switch
						if (current_token->requires_value())
						{
							// ... that did require a value
							//  - simply continue
						}
						else
						{
							// ... that did not require a value
							current_token->count_ += 1;
							current_token = NULL;
						}
					}
				}
				else
				{
					// We're looking for a value for the previous switch
					if (current_token->requires_value())
					{
						if (current_token->can_have_negative_number()
							&& (current_arg.find_first_not_of("-1234567890.") == std::string::npos))
						{
							// Negative number passed as value to option that allows it
							if (current_token->add_value(current_arg))
							{
								current_token->count_ += 1;
								current_token = NULL;
							}
						} else {
							// Previous switch required a value, but current option
							//  is a switch, so we have an error
							current_token->errors.push_back(std::string("Option ") + previous_arg + std::string(": Value required"));
							success = 0;
							current_token = NULL;
							do_not_advance = 1;
						}
					}
					else
					{
						// We should never arrive here, but just in case...
						// Previous option did not require a value, but we've
						//  still kept the current_token variable. This
						//  shouldn't happen.
						current_token = NULL;
					}
				}
			}
			else
			{
				// Current option is not a switch
				if (current_token == NULL)
				{
					// We have no switch to associate this value with
					loose_args.push_back(current_arg);
				}
				else
				{
					// Previous switch required a value, so this option
					//  must be it's value
					if (current_token->add_value(current_arg))
					{
						current_token->count_ += 1;
						current_token = NULL;
					}
					else
					{
						// Something went wrong when associating value with switch
						current_token->errors.push_back(
						  "Option " + previous_arg + ": Invalid value '" + current_arg + "'"
						);
						current_token = NULL;
						success = 0;
					}
				}
			}

			if (do_not_advance)
			{
				// Re-parse current option
				do_not_advance = 0;
			}
			else
			{
				p_cmdl += 1;
				previous_arg = current_arg;
			}
		}
	}

	if (current_token)
	{
		if (current_token->requires_value())
		{
			current_token->errors.push_back(std::string("Option ") + previous_arg + std::string(": Value required"));
			success = 0;
		}
	}

	success &= validate();

	if (success)
	{
		execute_callbacks();
	}

	return(success);
}

/*! \brief Validates recognized arguments.
 *
 * Checks if the count of each type of argument is within allowed range.
 * \return TRUE if all checks passed as valid, or FALSE if one or more errors
 * were found.
*/
int cmdline::validate()
{
	int success = 1;

	std::vector<arg*>::iterator p_arg = argtable.begin();
	while (p_arg != argtable.end())
	{
		if ((*p_arg)->count_ < (*p_arg)->min_count_)
		{
			success = 0;
			if ((*p_arg)->min_count_ == 1)
			{
				(*p_arg)->errors.push_back("Parameter " +
				    (*p_arg)->short_token_ + "required");
			}
			else
			{
				(*p_arg)->errors.push_back("More " +
				    (*p_arg)->short_token_ + "required");
			}
		}
		if ( ((*p_arg)->max_count_ != -1)
		  && ((*p_arg)->count_ > (*p_arg)->max_count_) )
		{
		  //std::cout << (*p_arg)->count_;
			success = 0;
			(*p_arg)->errors.push_back("Too many " + (*p_arg)->short_token_
					+ " (max " + int_to_string((*p_arg)->max_count_) + ")");
		}
		p_arg += 1;
	}

	if (loose_tokens.size() > 0)
	{
		success = 0;
		for (int i=0; i<loose_tokens.size(); i+=1)
		{
			errors.push_back("Unrecognized option: '" + loose_tokens[i] + "'");
		}
	}

	if (halt_on_errors && has_errors())
	{
		print_errors();
		exit(1);
	}

	return(success);
}

/*! \brief Loops through all found arguments and calls any callback
 * functions associated with them.
*/
void cmdline::execute_callbacks()
{
	std::vector<arg*>::iterator p_arg = argtable.begin();
	while (p_arg != argtable.end())
	{
		if ((*p_arg)->callback_function)
		{
			if ((*p_arg)->count_ > 0)
			{
				(*p_arg)->callback_function(this);
			}
		}
		p_arg += 1;
	}
}

/*! \brief Returns a vector of found command line arguments not associated
 * with a token.
 * \return Vector
*/
std::vector<std::string> cmdline::main_arguments()
{
	return(loose_args);
}

/*! \brief Checks if there are any errors.
 * \return TRUE if there are errors
*/
int cmdline::has_errors()
{
	int error_count = errors.size();

	std::vector<arg*>::iterator p_arg = argtable.begin();
	while (p_arg != argtable.end())
	{
		error_count += (*p_arg)->errors.size();
		p_arg += 1;
	}

	return(error_count != 0);
}

/*! \brief Retrieves all error messages.
 * \return Vector containing error messages
*/
std::vector<std::string> cmdline::error_messages()
{
	std::vector<std::string> messages;

	std::vector<std::string>::iterator p_global_errors = errors.begin();
	while (p_global_errors != errors.end())
	{
		messages.push_back(*p_global_errors);
		p_global_errors += 1;
	}

	std::vector<arg*>::iterator p_arg = argtable.begin();
	while (p_arg != argtable.end())
	{
		if ((*p_arg)->errors.size() > 0)
		{
			for (int i=0; i<(*p_arg)->errors.size(); i+=1)
			{
				messages.push_back((*p_arg)->errors[i]);
			}
		}
		p_arg += 1;
	}

	return(messages);
}

/*! \brief Retrieves a link to the class containing a specific token.
 * \return Link to token class, or NULL if none found.
*/
cmdline::arg *cmdline::find_token(std::string token)
{
	arg *found_token = NULL;
	std::vector<arg*>::iterator p_arg = argtable.begin();
	while ((found_token == NULL) && (p_arg != argtable.end()) )
	{
		if (
		   (((*p_arg)->short_token_.length() > 0) && ((*p_arg)->short_token_ == token))
		 || (((*p_arg)->long_token_.length() > 0) && ((*p_arg)->long_token_ == token)))
		{
			found_token = *p_arg;
		}
		p_arg += 1;
	}
	return(found_token);
}

/*! \brief Strips whitespace from the beginning and end of a string.
 * \param str Reference to string (can be modified)
 * \return Copy of the modified string
 * \todo Should be replaced by the same function in \c stringhtuils.cpp
*/
std::string cmdline::trim(std::string &str)
{
	str.erase(str.find_last_not_of("\t\n "+1));
	str.erase(str.find_first_not_of("\t\n "));
	return(str);
}

int cmdline::arg_int::operator[](int i)
{
	int tmp_value = 0;
	if ((i >= 0) && (i < value_.size())) { tmp_value = value_[i]; }

	return(tmp_value);
}

int cmdline::arg_int::value()
{
	return((value_.size() > 0 ) ? value_[0] : 0);
}

int cmdline::arg_int::requires_value()
{
	return(1);
}

int cmdline::arg_int::can_have_negative_number()
{
	return(1);
}

int cmdline::arg_int::add_value(std::string new_value)
{
	int success = 0;
	int int_value;
	if (new_value.find_first_not_of("-1234567890.") == std::string::npos)
	{
		int_value = atoi(new_value.c_str());
		value_.push_back(int_value);
		success = 1;
	}
	return(success);
}

unsigned int cmdline::arg_uint::operator[](int i)
{
	unsigned int tmp_value = 0;
	if ((i >= 0) && (i < value_.size())) { tmp_value = value_[i]; }

	return(tmp_value);
}

unsigned int cmdline::arg_uint::value()
{
	return((value_.size() > 0 ) ? value_[0] : 0);
}

int cmdline::arg_uint::requires_value()
{
	return(1);
}

int cmdline::arg_uint::can_have_negative_number()
{
	return(0);
}

int cmdline::arg_uint::add_value(std::string new_value)
{
	int success = 0;
	unsigned int uint_value;
	if (new_value.find_first_not_of("1234567890") == std::string::npos)
	{
		uint_value = atoi(new_value.c_str());
		value_.push_back(uint_value);
		success = 1;
	}
	return(success);
}

long cmdline::arg_long::operator[](int i)
{
	long tmp_value = 0;
	if ((i >= 0) && (i < value_.size())) { tmp_value = value_[i]; }

	return(tmp_value);
}

long cmdline::arg_long::value()
{
	return((value_.size() > 0 ) ? value_[0] : 0);
}

int cmdline::arg_long::requires_value()
{
	return(1);
}

int cmdline::arg_long::can_have_negative_number()
{
	return(1);
}

int cmdline::arg_long::add_value(std::string new_value)
{
	int success = 0;
	long long_value;
	if (new_value.find_first_not_of("1234567890") == std::string::npos)
	{
		long_value = atol(new_value.c_str());
		value_.push_back(long_value);
		success = 1;
	}
	return(success);
}

unsigned long cmdline::arg_ulong::operator[](int i)
{
	unsigned long tmp_value = 0;
	if ((i >= 0) && (i < value_.size())) { tmp_value = value_[i]; }

	return(tmp_value);
}

unsigned long cmdline::arg_ulong::value()
{
	return((value_.size() > 0 ) ? value_[0] : 0);
}

int cmdline::arg_ulong::requires_value()
{
	return(1);
}

int cmdline::arg_ulong::can_have_negative_number()
{
	return(0);
}

int cmdline::arg_ulong::add_value(std::string new_value)
{
	int success = 0;
	unsigned long long_value;
	if (new_value.find_first_not_of("1234567890") == std::string::npos)
	{
		long_value = atol(new_value.c_str());
		value_.push_back(long_value);
		success = 1;
	}
	return(success);
}

std::string cmdline::arg_string::operator[](int i)
{
	std::string tmp_value = "";
	if ((i >= 0) && (i < value_.size())) { tmp_value = value_[i]; }

	return(tmp_value);
}

std::string cmdline::arg_string::value()
{
	return((value_.size() > 0 ) ? value_[0] : "");
}

int cmdline::arg_string::requires_value()
{
	return(1);
}

int cmdline::arg_string::can_have_negative_number()
{
	return(0);
}

int cmdline::arg_string::add_value(std::string new_value)
{
	int success = 0;
	value_.push_back(new_value);
	success = 1;
	return(success);
}

float cmdline::arg_float::operator[](int i)
{
	float tmp_value = 0;
	if ((i >= 0) && (i < value_.size())) { tmp_value = value_[i]; }

	return(tmp_value);
}

float cmdline::arg_float::value()
{
	return((value_.size() > 0 ) ? value_[0] : 0);
}

int cmdline::arg_float::can_have_negative_number()
{
	return(1);
}

int cmdline::arg_float::requires_value()
{
	return(1);
}

int cmdline::arg_float::add_value(std::string new_value)
{
	int success = 0;
	float float_value;
	if (new_value.find_first_not_of(".,1234567890") == std::string::npos)
	{
		int decimal_points = 0;
		for (int i=0; i<new_value.length(); i += 1)
		{
			if (new_value[i] == ',') { new_value[i] = '.'; }
			if (new_value[i] == '.') { decimal_points += 1; }
		}
		if (decimal_points < 2)
		{
			float_value = atof(new_value.c_str());
			value_.push_back(float_value);
			success = 1;
		}
	}
	return(success);
}

double cmdline::arg_double::operator[](int i)
{
	double tmp_value = 0;
	if ((i >= 0) && (i < value_.size())) { tmp_value = value_[i]; }

	return(tmp_value);
}

double cmdline::arg_double::value()
{
	return(value_[0]);
}

int cmdline::arg_double::requires_value()
{
	return(1);
}

int cmdline::arg_double::can_have_negative_number()
{
	return(1);
}

int cmdline::arg_double::add_value(std::string new_value)
{
	int success = 0;
	double double_value;
	if (new_value.find_first_not_of(".,1234567890") == std::string::npos)
	{
		int decimal_points = 0;
		for (int i=0; i<new_value.length(); i += 1)
		{
			if (new_value[i] == ',') { new_value[i] = '.'; }
			if (new_value[i] == '.') { decimal_points += 1; }
		}
		if (decimal_points < 2)
		{
			double_value = atof(new_value.c_str());
			value_.push_back(double_value);
			success = 1;
		}
	}
	return(success);
}

int cmdline::arg_flag::requires_value()
{
	return(0);
}

int cmdline::arg_flag::can_have_negative_number()
{
	return(0);
}

bool cmdline::arg_flag::is_set()
{
	return(count_ > 0);
}

int cmdline::arg_flag::add_value(std::string new_value)
{
	int success = 0;
	success = 1;
	return(success);
}

void cmdline::arg::set_callback(void (*callback_function_)(cmdline*))
{
	callback_function = callback_function_;
}

std::string cmdline::arg::error()
{
	std::string first_error = "";
	if (errors.size() > 0)
	{
		first_error = errors[0];
	}
	return(first_error);
}

int cmdline::arg::count()
{
	return(count_);
}

int cmdline::arg::min_count()
{
	return(min_count_);
}

int cmdline::arg::max_count()
{
	return(max_count_);
}

std::string cmdline::arg::short_token()
{
	return(short_token_);
}

std::string cmdline::arg::long_token()
{
	return(long_token_);
}

std::string cmdline::arg::description()
{
	return(description_);
}

bool cmdline::arg::visible()
{
	return(visible_);
}
