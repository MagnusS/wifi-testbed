/*! \file resulthandler.hpp
 * \brief Generic result handler for functions and class methods.
 *
 * Allows a function or method to return more than just \e True or \e False
 * to indicate success or failure. It contains three different values in
 * addition to a user-selected value type.
 * \date 2010
 * \author   Forsvarets Forskningsinstitutt (FFI)\n
 *           http://www.ffi.no \n
 *           - Terje Mikal Mjelde [tmo]\n
 *             terje-mikal-mjelde@ffi.no
 */


#ifndef __RESULTHANDLER_HPP
#define __RESULTHANDLER_HPP

#include "platform.h"
#include "messagelist.hpp"

#include <string>
#include <vector>

/*! \brief Generic result handler to be used as return value from function
 * and class methods.
 *
 * Objects of this class evaluates to TRUE or FALSE to indicate success or
 * error. In addition it contains an \ref resulthandler.id() "ID" to indicate
 * a status, and a \ref resulthandler.text() "string" for easily reporting
 * status back to the user. The \e ID can be automatically mapped to
 * a string by adding the string to the \ref resulthandler_messages[] array.
 * A user-specified return type is also included, which may contain any data
 * relevant for the function. Typedefs for the most common data types are
 * \ref resulthandler_typedefs "defined".
 */
template <class T>
class resulthandler
{
public:
	typedef enum { KO = 0, NOT_OK = 0, FAIL = 0, FAILED = 0, OK = 1, SUCCESS = 1 } status;

	resulthandler();
	resulthandler(status success);
	resulthandler(status success, int id);
	resulthandler(status success, std::string text);
	resulthandler(status success, int id, std::string text);
	resulthandler(T value, status success);
	resulthandler(T value, status success, int id);
	resulthandler(T value, status success, std::string text);
	resulthandler(T value, status success, int id, std::string text);
	~resulthandler();
	void common_constructor();

	void set_ok();
	void set_not_ok();
	bool is_ok();
	bool is_not_ok();
	void set_exitstatus(int id);
	void set_exitstatus(std::string text);
	void set_exitstatus(int id, std::string text);
	void set_not_ok(int id);
	void set_not_ok(std::string text);
	void set_not_ok(int id, std::string text);
	void set_ok(int id);
	void set_ok(std::string text);
	void set_ok(int id, std::string text);
	void set_value(T value);
	std::string text();
	int  id();
	int  operator==(const int existatus_id) const;
	bool operator==(const status) const;
	bool operator!=(const status) const;
	int  operator!=(const int existatus_id) const;
	int  operator<=(const int existatus_id) const;
	int  operator>=(const int existatus_id) const;
	int  operator<(const int existatus_id) const;
	int  operator>(const int existatus_id) const;
	void operator=(const status new_status);
	void operator=(const int existatus_id);
	void operator=(const std::string exitstatus_text);
	//template <class T2> resulthandler<T>& operator=(const resulthandler<T2>&);
	bool operator()() const;
	bool operator!() const;
	operator void * () const;
	T    value();
private:
	status success_;  //!< Return status that indicates success or failure.
	int    id_; //!< Numerical ID of return status.
	std::string text_;  //!< Textual representation of return status.
	int    messagelist_size;  //!< Size of array containing textual messages.
	T      value_; //!< User specified return value.
};

/*! \defgroup resulthandler_typedefs Common resulthandler data types.
 */
/*@{*/
typedef resulthandler<void*>         RH;
typedef resulthandler<int>           RH_INT;
typedef resulthandler<unsigned>      RH_UINT;
typedef resulthandler<float>         RH_FLOAT;
typedef resulthandler<double>        RH_DOUBLE;
typedef resulthandler<char>          RH_CHAR;
typedef resulthandler<char*>         RH_CHARPTR;
typedef resulthandler<std::string>   RH_STRING;
/*@}*/

/*! \brief Common initialization called by all constructors.
*/
template <class T>
void resulthandler<T>::common_constructor()
{
	success_ = KO;
	id_ =  0;
	text_ = "";
	messagelist_size = 0;
	while (resulthandler_messages[messagelist_size].id != -1)
		messagelist_size += 1;
}

/*! \brief Default constructor.
 */
template <class T>
resulthandler<T>::resulthandler()
{
	common_constructor();
}

/*! \brief Constructor initializing return status (TRUE or FALSE).
*/
template <class T>
resulthandler<T>::resulthandler(status success)
{
	common_constructor();
	success_ = success;
}

/*! \brief Constructor initializing return status (TRUE or FALSE) and
 * result ID.
*/
template <class T>
resulthandler<T>::resulthandler(status success, int id)
{
	common_constructor();
	success_ = success;
	id_ = id;
}

/*! \brief Constructor initializing return status (TRUE or FALSE) and
 * result text.
*/
template <class T>
resulthandler<T>::resulthandler(status success, std::string text)
{
	common_constructor();
	success_ = success;
	text_ = text;
}

/*! \brief Constructor initializing return status (TRUE or FALSE),
 * result ID and text.
*/
template <class T>
resulthandler<T>::resulthandler(status success, int id, std::string text)
{
	common_constructor();
	success_ = success;
	id_ = id;
	text_ = text;
}

/*! \brief Constructor initializing user specified value type and
 * return status (TRUE or FALSE).
*/
template <class T>
resulthandler<T>::resulthandler(T value, status success)
{
	common_constructor();
	value_ = value;
	success_ = success;
}

/*! \brief Constructor initializing user specified value type,
 * return status (TRUE or FALSE) and result ID.
*/
template <class T>
resulthandler<T>::resulthandler(T value, status success, int id)
{
	common_constructor();
	value_ = value;
	success_ = success;
	id_ = id;
}

/*! \brief Constructor initializing user specified value type,
 * return status (TRUE or FALSE) and result text.
*/
template <class T>
resulthandler<T>::resulthandler(T value, status success, std::string text)
{
	common_constructor();
	value_ = value;
	success_ = success;
	text_ = text;
}

/*! \brief Constructor initializing user specified value type,
 * return status (TRUE or FALSE), result ID and text.
*/
template <class T>
resulthandler<T>::resulthandler(T value, status success, int id, std::string text)
{
	common_constructor();
	value_ = value;
	success_ = success;
	id_ = id;
	text_ = text;
}

/*! \brief Destructor.
 */
template <class T>
resulthandler<T>::~resulthandler()
{
}

/*! \brief Sets return value to TRUE.
*/
template <class T>
void resulthandler<T>::set_ok()
{
	success_ = OK;
}

/*! \brief Sets return value to FALSE.
*/
template <class T>
void resulthandler<T>::set_not_ok()
{
	success_ = KO;
}

/*! \brief Checks return value.
 * \return TRUE if return vale is TRUE.
*/
template <class T>
bool resulthandler<T>::is_ok()
{
	return(success_ == OK);
}

/*! \brief Checks return value.
 * \return FALSE if return vale is FALSE.
*/
template <class T>
bool resulthandler<T>::is_not_ok()
{
	return(success_ == KO);
}

/*! \brief Sets the result ID.
*/
template <class T>
void resulthandler<T>::set_exitstatus(int id)
{
	id_ = id;
}

/*! \brief Sets the result text.
*/
template <class T>
void resulthandler<T>::set_exitstatus(std::string text)
{
	text_ = text;
}

/*! \brief Sets the result ID and text.
*/
template <class T>
void resulthandler<T>::set_exitstatus(int id, std::string text)
{
	id_ = id;
	text_ = text;
}

/*! \brief Sets the return value to FALSE and stores a result ID.
*/
template <class T>
void resulthandler<T>::set_not_ok(int id)
{
	id_ = id;
	success_ = KO;
}

/*! \brief Sets the return value to FALSE and stores a result text.
*/
template <class T>
void resulthandler<T>::set_not_ok(std::string text)
{
	text_ = text;
	success_ = KO;
}

/*! \brief Sets the return value to FALSE and stores a result ID and text.
*/
template <class T>
void resulthandler<T>::set_not_ok(int id, std::string text)
{
	id_ = id;
	text_ = text;
	success_ = KO;
}

/*! \brief Sets the return value to TRUE and stores a result ID.
*/
template <class T>
void resulthandler<T>::set_ok(int id)
{
	id_ = id;
	success_ = OK;
}

/*! \brief Sets the return value to TRUE and stores a result ID.
*/
template <class T>
void resulthandler<T>::set_ok(std::string text)
{
	text_ = text;
	success_ = OK;
}

/*! \brief Sets the return value to TRUE and stores a result ID and text.
*/
template <class T>
void resulthandler<T>::set_ok(int id, std::string text)
{
	id_ = id;
	text_ = text;
	success_ = OK;
}

/*! \brief Sets the user specified data type to given value.
*/
template <class T>
void resulthandler<T>::set_value(T value)
{
	value_ = value;
}

/*! \brief Retrieves result text.
 *
 * If a specific result text has been set, this is returned. If not, then
 * the text associated (in \ref resulthandler_messages[]) with it is
 * returned. If there is no associated text, an emtpy string is returned.
 * \return Result text
*/
template <class T>
std::string resulthandler<T>::text()
{
	std::string text("");
	if (text_ != "")
	{
		text = text_;
	}
	else
	{
		int array_index = 0;
		while ((array_index < messagelist_size)
			   && (resulthandler_messages[array_index].id != id_))
		{
			array_index += 1;
		}
		if (array_index < messagelist_size)
		{
			text = resulthandler_messages[array_index].text;
		}
	}
	return(text);
}

/*! \brief Returns result ID.
 * \return Result ID.
*/
template <class T>
int resulthandler<T>::id()
{
	return(id_);
}

/*! \brief Returns TRUE or FALSE based on return value.
 *
 * Allows checking the return value on the following form:
 * \code if (resultvalue()) { ... } \endcode
*/
template <class T>
bool resulthandler<T>::operator()() const
{
	return(success_ == OK);
}

/*! \brief Returns TRUE or FALSE based on inverted return value.
 *
 * Allows checking the return value on the following form:
 * \code if (!resultvalue) { ... } \endcode
*/
template <class T>
bool resulthandler<T>::operator!() const
{
	return(success_ == KO);
}

/*! \brief Returns TRUE or FALSE based on return value.
 *
 * Allows checking the return value on the following form:
 * \code if (resultvalue) { ... } \endcode
*/
template <class T>
resulthandler<T>::operator void * () const
{
	return((void*)(success_ == OK));
}

/*! \brief Checks if return value is equal to comparison value.
 *
 * Allows checking the return value on the following form:
 * \code if (resultvalue == OK) { ... } \endcode
*/
template <class T>
bool resulthandler<T>::operator==(const status) const
{
	return(success_ == OK);
}

/*! \brief Checks if return value is not equal to comparison value.
 *
 * Allows checking the return value on the following form:
 * \code if (resultvalue != OK) { ... } \endcode
*/
template <class T>
bool resulthandler<T>::operator!=(const status) const
{
	return(success_ != OK);
}

/*! \brief Checks if result ID is equal to comparison value.
 *
 * Allows checking the result ID via on the following form:
 * \code if (resultvalue == MY_RESULT_STATUS) { ... } \endcode
*/
template <class T>
int resulthandler<T>::operator==(const int id) const
{
	return(id_ == id);
}

/*! \brief Checks if result ID is not equal to comparison value.
 *
 * Allows checking the result ID on the following form:
 * \code if (resultvalue != MY_RESULT_STATUS) { ... } \endcode
*/
template <class T>
int resulthandler<T>::operator!=(const int id) const
{
	return(id_ != id);
}

/*! \brief Checks if result ID is smaller or equal to comparison value.
 *
 * Allows checking the result ID on the following form:
 * \code if (resultvalue <= MY_RESULT_STATUS) { ... } \endcode
*/
template <class T>
int resulthandler<T>::operator<=(const int id) const
{
	return(id_ <= id);
}

/*! \brief Checks if result ID is greater or equal to comparison value.
 *
 * Allows checking the result ID on the following form:
 * \code if (resultvalue >= MY_RESULT_STATUS) { ... } \endcode
*/
template <class T>
int resulthandler<T>::operator>=(const int id) const
{
	return(id_ >= id);
}

/*! \brief Checks if result ID is smaller than comparison value.
 *
 * Allows checking the result ID on the following form:
 * \code if (resultvalue < MY_RESULT_STATUS) { ... } \endcode
*/
template <class T>
int resulthandler<T>::operator<(const int id) const
{
	return(id_ < id);
}

/*! \brief Checks if result ID is greater than comparison value.
 *
 * Allows checking the result ID on the following form:
 * \code if (resultvalue > MY_RESULT_STATUS) { ... } \endcode
*/
template <class T>
int resulthandler<T>::operator>(const int id) const
{
	return(id_ > id);
}

/*! \brief Assigns new return value.
 *
 * Allows assignment on the following form:
 * \code resultvalue = OK; \endcode
*/
template <class T>
void resulthandler<T>::operator=(const status new_status)
{
	success_ = new_status;
}

/*! \brief Assigns new result ID.
 *
 * Allows assignment on the following form:
 * \code resultvalue = MY_RESULT_STATUS; \endcode
*/
template <class T>
void resulthandler<T>::operator=(const int id)
{
	set_exitstatus(id);
}

/*! \brief Assigns new result text string.
 *
 * Allows assignment on the following form:
 * \code resultvalue = "Something went wrong"; \endcode
*/
template <class T>
void resulthandler<T>::operator=(std::string text)
{
	set_exitstatus(text);
}

//template <class T>
//template <class T2>
//resulthandler<T>& resulthandler<T>::operator=(const resulthandler<T2>& rh)
//{
//	success_ = rh.success_;
//	id_ = rh.id_;
//	text_ = rh.text_;
//	return *this;
//}

/*! \brief Returns data in user specified data type.
*/
template <class T>
T resulthandler<T>::value()
{
	return(value_);
}

#endif // __RESULTHANDLER_HPP


