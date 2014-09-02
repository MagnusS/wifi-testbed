/*! \file vout.cpp
 *  \copydoc vout.hpp
 */

#define VOUT_ALLOC
#include "vout.hpp"
#undef VOUT_ALLOC

/*! \brief Output stream which leads nowhere (i.e. discards anything
 * sent there).
*/
struct nullstream : std::ostream
{
    nullstream() : std::ostream(0) { }
};

/*! \brief Output stream.
 *
 * Acts as a filter placed in the output stream which allows data output to be
 * associated with a level. If that level is lower than \ref verbosity
 * then the output is redirected to given stream (usually std::cout).
 * If not, the output is redirected to \ref nullstream which in effect
 * discards it.
 * \param verbosity_level Level associated with current data
 * \param out Output stream to redirect data to
 * \return Reference to selected output stream
*/
std::ostream& vout(int verbosity_level, std::ostream& out)
{
    static nullstream blackhole;

    int v = verbosity_level;
#	ifndef DEBUG
    	if (v >= VOUT_DEBUG)
    	{
    		v = VOUT_MAX + 1;  // Make sure we're always above the set verbosity
    	}
#	endif

    std::ostream& outpath = v > verbosity ? blackhole : out;

#	ifdef DEBUG
    	if (verbosity_level >= VOUT_DEBUG)
    	{
    		outpath << ansi::yellow;
    	}
#	endif

    if (verbosity_level == VOUT_ERROR)
    {
    	outpath << ansi::bright << ansi::red ;
    }


    return(outpath);
}
