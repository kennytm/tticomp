#ifndef CHECK_OVERFLOW_H
#define CHECK_OVERFLOW_H

/**
	\file check_overflow.h \brief Defines functions that check whether various
	may yield overflows and, if so, throw overflow_exception objects.
*/


#include "precision_trait.h"

/** \brief Check for overflows in util::fixed operations.

  %#undef this to throw no exceptions
*/
#define CHECK_OVERFLOWS

namespace util {

	/// \brief Is thrown when an overflow is detected.
	class overflow_exception {};

	/// \brief Check for overflows when shifting.
	///
	/// This is defined as a struct so that MSVC 6 won't mess up the template parameters.

	template <int n_source, int n_dest, typename int_type>
		struct check_overflow_shift
	{
		/// \brief Check whether the integer may be shifted without overflow.
		static void check (const int_type & i) {
#ifdef CHECK_OVERFLOWS
			if (shift_trait <n_source, n_dest>::yields_overflow (i))
				throw overflow_exception();
#endif
		}

		/// \brief Check whether the two integers may be shifted right
		/// forming one integer without overflow.
		static void check (const int_type & i0, const int_type & i1)
		{
#ifdef CHECK_OVERFLOWS
			if (shift_trait <n_source, n_dest>::yields_overflow (i0, i1))
				throw overflow_exception();
#endif
		}
	};

	/// \brief Check for overflows when converting from source_type to dest_type.
	///
	/// Both types must be signed
	template <typename dest_type, typename source_type>
		inline dest_type check_overflow_conversion (const source_type & i)
	{
#ifdef CHECK_OVERFLOWS
		if (i < get_min_int <dest_type>() || i > get_max_int <dest_type>())
			throw overflow_exception();
#endif

		return dest_type (i);
	}

	/// \brief Check for overflows when converting from source_type to dest_type.
	///
	/// source_type must be unsigned. dest_type must be signed.
	template <typename dest_type, typename source_type>
		inline dest_type check_overflow_conversion_unsigned (const source_type & i)
	{
#ifdef CHECK_OVERFLOWS
		if (i > source_type (get_max_int <dest_type>()))
			throw overflow_exception();
#endif

		return dest_type (i);
	}

	/// \brief Check for overflows when multiplying two numbers.
	template <typename int_type>
		inline int_type check_overflow_multiplication (const int_type & i1, const int_type & i2)
	{
#ifdef CHECK_OVERFLOWS
		if (i1 != 0 && i2 != 0) {
			if (i1 < 0) {
				if (i2 < 0) {
					if (get_max_int <int_type>() / i1 > i2)
						throw overflow_exception();
				} else {
					// Dividing min_int by -1 will return an invalid value!
					if (i1 == -1) {
						if (i2 == get_min_int <int_type>())
							// Can't be negated
							throw overflow_exception();
					} else {
						if (get_min_int <int_type>() / i1 < i2)
							throw overflow_exception();
					}
				}
			} else {
				if (i2 < 0) {
					if (get_min_int <int_type>() / i1 > i2)
						throw overflow_exception();
				} else {
					if (get_max_int <int_type>() / i1 < i2)
						throw overflow_exception();
				}
			}
		}
#endif
		return int_type (i1 * i2);
	}

	/// \brief Check for overflows when adding two numbers.
	template <typename int_type>
		inline int_type check_overflow_addition (const int_type & i1, const int_type & i2)
	{
#ifdef CHECK_OVERFLOWS
		if (i2 < 0) {
			if (get_min_int <int_type>() - i2 > i1)
				throw overflow_exception();
		} else {
			if (get_max_int <int_type>() - i2 < i1)
				throw overflow_exception();
		}
#endif
		return int_type (i1 + i2);
	}

	/// \brief Check whether the number fits in n number of bits.
	///
	/// Throw an overflow_exception otherwise.
	template <int n, typename int_type>
		struct check_overflow_fit
	{
		static int_type check (const int_type & i)
		{
#ifdef CHECK_OVERFLOWS
			if (i < 0) {
				if ((i & (int_type (-1) << (n-1))) != (int_type (-1) << (n-1)))
					throw overflow_exception();
			} else {
				if (i & (int_type (-1) << (n-1)))
					throw overflow_exception();
			}
#endif
			return i;
		}
	};

}

#endif	// CHECK_OVERFLOW_H
