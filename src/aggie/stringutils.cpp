/*! \file stringutils.cpp
 *  \copydoc stringutils.hpp
 */

 #include "stringutils.hpp"

/*! \brief Converts string to integer.
 * \param s_value string to convert
 * \param i_value resulting integer
 * \return TRUE if success
 * \bug Does not accept negative values
*/
int string_to_int(std::string s_value, int &i_value)
{
	int error = 1;
	trim(s_value);
	if (s_value.find_first_not_of("1234567890") == std::string::npos)
	{
		i_value = atoi(s_value.c_str());
		error = 0;
	}
	return(error);
}

/*! \brief Converts string to unsigned integer.
 * \param s_value string to convert
 * \param i_value resulting integer
 * \return TRUE if success
 * \bug Does not accept negative values
*/
int string_to_unsigned(std::string s_value, unsigned &i_value)
{
	int error = 1;
	trim(s_value);
	if (s_value.find_first_not_of("1234567890") == std::string::npos)
	{
		i_value = atoi(s_value.c_str());
		error = 0;
	}
	return(error);
}

/*! \brief Converts string to long integer.
 * \param s_value string to convert
 * \param l_value resulting long integer
 * \return TRUE if success
 * \bug Does not accept negative values
*/
int string_to_long(std::string s_value, long &l_value)
{
	int error = 1;
	trim(s_value);
	if (s_value.find_first_not_of("1234567890") == std::string::npos)
	{
		l_value = atol(s_value.c_str());
		error = 0;
	}
	return(error);
}

/*! \brief Converts string to float.
 *
 * Accepts both "." and "," as decimal point.
 * \param s_value string to convert
 * \param f_value resulting float
 * \return TRUE if success
 * \bug Does not accept negative values
*/
int string_to_float(std::string s_value, float &f_value)
{
	int error = 1;
	trim(s_value);
	if (s_value.find_first_not_of(".,1234567890") == std::string::npos)
	{
		int decimal_points = 0;
		for (int i=0; i<s_value.length(); i += 1)
		{
			if (s_value[i] == ',') { s_value[i] = '.'; }
			if (s_value[i] == '.') { decimal_points += 1; }
		}
		if (decimal_points < 2)
		{
			f_value = atof(s_value.c_str());
			error = 0;
		}
	}
	return(error);
}

/*! \brief Converts integer to string.
 * \param i_value integer to convert
 * \param s_value resulting string
 * \return copy of result string
*/
std::string int_to_string(int i_value, std::string &s_value)
{
	std::stringstream tmp_str;
	tmp_str << i_value;
	s_value = tmp_str.str();
	return(s_value);
}

/*! \brief Converts integer to string.
 * \param i_value integer to convert
 * \return Result string
*/
std::string int_to_string(int i_value)
{
	std::stringstream tmp_str;
	std::string s_value;
	tmp_str << i_value;
	s_value = tmp_str.str();
	return(s_value);
}

/*! \brief Converts long integer to string.
 * \param l_value long integer to convert
 * \param s_value resulting string
 * \return copy of result string
*/
std::string long_to_string(long l_value, std::string &s_value)
{
	std::stringstream tmp_str;
	tmp_str << l_value;
	s_value = tmp_str.str();
	return(s_value);
}

/*! \brief Converts float to string.
 * \param f_value float to convert
 * \param s_value resulting string
 * \return copy of result string
*/
std::string float_to_string(float f_value, std::string &s_value)
{
	std::stringstream tmp_str;
	tmp_str << f_value;
	s_value = tmp_str.str();
	return(s_value);
}

/*! \brief Converts double to string.
 * \param d_value double to convert
 * \param s_value resulting string
 * \return copy of result string
*/
std::string double_to_string(double d_value, std::string &s_value)
{
	std::stringstream tmp_str;
	tmp_str << d_value;
	s_value = tmp_str.str();
	return(s_value);
}

/*! \brief Converts string to double.
 *
 * Accepts both "." and "," as decimal point.
 * \param s_value string to convert
 * \param d_value resulting double
 * \return TRUE if success
 * \bug Does not accept negative values
*/
int string_to_double(std::string s_value, double &d_value)
{
	int error = 1;
	trim(s_value);
	if (s_value.find_first_not_of(".,1234567890") == std::string::npos)
	{
		int decimal_points = 0;
		for (int i=0; i<s_value.length(); i += 1)
		{
			if (s_value[i] == ',') { s_value[i] = '.'; }
			if (s_value[i] == '.') { decimal_points += 1; }
		}
		if (decimal_points < 2)
		{
			d_value = atof(s_value.c_str());
			error = 0;
		}
	}
	return(error);
}

/*! \brief Strips (trims) string of specified character from beginning
 * of string.
 * \param str string to modify
 * \param chars_to_remove characters to remove
 * \return Copy of modified string
*/
std::string ltrim(std::string &str, const char *chars_to_remove)
{
	int first_number_pos = str.find_first_not_of(chars_to_remove);
	str.erase(0, first_number_pos);
	return(str);
}

/*! \brief Strips (trims) string of specified character from end
 * of string.
 * \param str string to modify
 * \param chars_to_remove characters to remove
 * \return Copy of modified string
*/
std::string rtrim(std::string &str, const char *chars_to_remove)
{
	int last_number_pos = str.find_last_not_of(chars_to_remove);
	str.erase(last_number_pos+1);
	return(str);
}

/*! \brief Strips (trims) string of specified character from both
 * beginning and end of string.
 * \param str string to modify
 * \param chars_to_remove characters to remove
 * \return Copy of modified string
*/
std::string trim(std::string &str, const char *chars_to_remove)
{
	ltrim(str, chars_to_remove);
	rtrim(str, chars_to_remove);
	return(str);
}

/*! \brief Converts string to all upper case letters.
 *
 * Supports scandinavian and some accented characters (ISO8859-1).
 * \param str String to convert
 * \return copy of converted string
*/
std::string to_upper(std::string str)
{
	for (int i=0; i<str.length(); i += 1)
	{
		char c = str[i];
		if ((c >= 0xe0) && (c <= 0xfe)) { c-= 0x20; }
		if ((c >= 'a') && (c <= 'z')) { c -= ('a' - 'A'); }
		str[i] = c;
	}

	return(str);
}

/*! \brief Converts string to all lower case letters.
 *
 * Supports scandinavian and some accented characters (ISO8859-1).
 * \param str String to convert
 * \return copy of converted string
*/
std::string to_lower(std::string str)
{
	for (int i=0; i<str.length(); i += 1)
	{
		char c = str[i];
		if ((c >= 'A') && (c <= 'Z')) { c += ('a' - 'A'); }
		if ((c >= 0xc0) && (c <= 0xde)) { c+= 0x20; }
		str[i] = c;
	}

	return(str);
}

/*! \brief Acts as a \c sprintf() function for std::strings.
 * \param format Pointer to string to format. Can contain all \c printf()
 * placeholders (e.g. %%i, %%s, %%5.2f ...). This string contains the result
 * upon return.
 * \param ... variables to insert into placeholders
 * \return formatted string
*/
std::string format_string(std::string *format, ...)
{
	va_list ap;
	va_start(ap, format);
	char *formatted_string;
	std::string result;
	int strlength = vsnprintf(formatted_string, 0, format->c_str(), ap);
	va_end(ap);

	formatted_string = new char[strlength+1];
	va_start(ap, format);
	vsprintf(formatted_string, format->c_str(), ap);
	va_end(ap);

	result = std::string(formatted_string);
	*format = result;
	delete[] formatted_string;

	return(result);
}

/*! \brief Acts as a \c sprintf() function for std::strings.
 * \param format String to format. Can contain all \c printf() placeholders
 * (e.g. %%i, %%s, %%5.2f ...)
 * \param ... Variables to insert into placeholders
 * \return Formatted string
*/
std::string format_string(std::string format, ...)
{
	va_list ap;
	va_start(ap, format);
	char *formatted_string;
	std::string result;
	int strlength = vsnprintf(formatted_string, 0, format.c_str(), ap);
	va_end(ap);

	formatted_string = new char[strlength+1];
	va_start(ap, format);
	vsprintf(formatted_string, format.c_str(), ap);
	va_end(ap);
	result = std::string(formatted_string);
	delete[] formatted_string;

	return(result);
}

/*! \brief Converts all non-printable characters in a string to hex-representations.
 *
 * Characters between 0x20 (space) and 0x7e (~) remain unchanged, while all others
 * are replaced with a "[0x??]"-representation.
 *
 * \param str String to make ascii-safe
 * \return ascii-safe string
 */
std::string ascii_safe(std::string str, bool print_unsafe)
{
	std::string result = "";
	char ascii[5];

	for (int i = 0; i < str.length(); i++)
	{
		if ((str[i] >= ' ') && (str[i] <= '~'))
		{
			result += str[i];
		}
		else if (print_unsafe)
		{
			sprintf(ascii, "%02x", str[i]);
			result += "[0x" + std::string(ascii) + "]";
		}
	}

	return(result);
}
