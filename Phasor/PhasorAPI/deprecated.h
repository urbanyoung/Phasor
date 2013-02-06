/*! \file deprecated.h
 * \brief Implementations provided for backwards compatibility with old scripts.
 * 
 * All functions here are either deprecated [dep] (and to be removed) or have 
 * been subsequently updated and their usage changed [cng].
 * 
 * All scripts with a version number prior to 01.00.10.059 use these functions.
 * 
 * The functions declared here are implemented in their respective files,
 * where the current versions reside, although in the deprecated namespace.
 */

#include "PhasorAPI.h"

namespace deprecated
{
	// Memory functions
	// --------------------------------------------------------------------
	// 
	/*! \brief [cng] Writes a bit to the specified memory address.
	 *
	 *	\param base_address The base address to use.
	 *	\param address_offset The offset relative to base_address.
	 *	\param bit_offset Which bit to write (starting at 0)
	 *	\param data The value to write (1 or 0)
	 * 
	 * 	Example usage:
	 *  \code
	 *		writebit(0x12345678, 3, 1, 0) -- write 0 to the 2nd bit at 0x12345678 + 3
	 *  \endcode
	 */
	void l_writebit(PHASOR_API_ARGS);

	/*! \brief [cng] Writes a byte to the specified memory address.
	 *
	 *	\param base_address The base address to use.
	 *	\param address_offset The offset relative to base_address.
	 *	\param data The data to write
	 *	
	 *	Example usage:
	 *  \code
	 *		writebyte(0x12345678, 3, 50) -- write 50 to 0x12345678 + 3
	 *  \endcode
	 */
	void l_writebyte(PHASOR_API_ARGS);

	/*! \brief [cng] Writes a word to the specified memory address.
	 *
	 *	\param base_address The base address to use.
	 *	\param address_offset The offset relative to base_address.
	 *	\param data The data to write
	 *	
	 *	Example usage:
	 *  \code
	 *		writeword(0x12345678, 3, 50) -- write 50 to 0x12345678 + 3
	 *  \endcode
	 */
	void l_writeword(PHASOR_API_ARGS);

	/*! \brief [cng] Writes a dword to the specified memory address.
	 *
	 *	\param base_address The base address to use.
	 *	\param address_offset The offset relative to base_address.
	 *	\param data The data to write
	 *	
	 *	Example usage:
	 *  \code
	 *		writedword(0x12345678, 3, 0xbadf00d) -- write 0xbadf00d to 0x12345678 + 3
	 *  \endcode
	 */
	void l_writedword(PHASOR_API_ARGS);

	/*! \brief [cng] Writes a float to the specified memory address.
	 *
	 *	\param base_address The base address to use.
	 *	\param address_offset The offset relative to base_address.
	 *	\param data The data to write
	 *	
	 *	Example usage:
	 *  \code
	 *		writefloat(0x12345678, 3, 1.5) -- write 1.5 to 0x12345678 + 3
	 *  \endcode
	 */
	void l_writefloat(PHASOR_API_ARGS);
}