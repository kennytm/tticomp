#ifndef PRECISION_TRAITS
#define PRECISION_TRAITS

/**
	\file precision_trait.h \brief Defines classes that supply information about integer
	type size at compile-time.
*/

#include "static_test.h"

namespace util {

//	template <int n_source, int n_dest, typename int_type>
//		inline void check_overflow_shift (const int_type & i);

	template <int n_source, int n_dest, typename int_type> struct check_overflow_shift;
	template <int n> struct int_precision_trait;
	template <typename I> struct int_bits_trait;

}

/*** Define integer types ***/

#ifdef _MSC_VER

namespace util {
	/// 64-bits integer type
	typedef __int64 long_long_int;
	/// 64-bits unsigned integer type
	typedef unsigned __int64 unsigned_long_long_int;
}

#ifndef _STLPORT_VERSION

#include <iostream>
#include <string>

// MSVC does not define an operator << (ostream &, __int64). Strange.
// Define our own (which is pretty slow).

// It needs to be in a namespace because otherwise
// util::operator << (const fixed &) will get confused. Again: strange...

namespace util {

	inline std::ostream & operator << (std::ostream & o, long_long_int i)
	{
		if (i < 0) {
			std::cout << "-";
			i = -i;
		}
		std::string s;
		do {
			s = std::string (1, char ('0' + (i%10))) + s;

			i /= 10;
		} while (i);
		std::cout << s;
		return o;
	}
}

#endif

#else
namespace util {
	typedef long long long_long_int;
	typedef unsigned long long unsigned_long_long_int;
}
#endif

namespace util {

	/*** integer number precision calculations ***/

	/** \brief Indicates how many bits an integer type uses.

		Only its specialisations must be used.
		The specialisations are different for every architecture and may be
		different for various compilers as well (e.g., in how 64-bits types
		are called).
		The easiest way to produce specialisations is to use the
		#DECLARE_PRECISION_BITS macro.
	*/
	template <typename I> struct int_bits_trait {
		static_assert <false> _this_must_never_be_used_because_there_should_be_a_explicit_specialisation;

		enum {
			/// \brief The number of bits of type I.
			precision = 0
		};
	};

	/** \brief Indicates which integer type may be used for the specified number of bits.

		The default implementation uses a defined integer with precision of at least n bits.
		The specialisations are different for every architecture and may be
		different for various compilers as well (e.g., in how 64-bits types
		are called).
		The easiest way to produce specialisations is to use the
		#DECLARE_PRECISION_BITS macro.
	*/
	template <int n> struct int_precision_trait {
		/// \brief The smallest integer type that gives n-bits precision.
		typedef typename int_precision_trait <n-1>::higher_precision_type type;
		/** \brief The next integer type.

			Used internally.
			The largest type should point have higher_precision_type set to
			itself.
		*/
		typedef typename int_precision_trait <n-1>::higher_precision_type higher_precision_type;
		typedef typename int_precision_trait <n-1>::lower_precision_type_help lower_precision_type;
		typedef typename int_precision_trait <n-1>::lower_precision_type_help lower_precision_type_help;

		enum {
			/// \brief Whether the precision requested is available.
			precision_available = int_precision_trait <n-1>::higher_precision_available,
			/// \brief Whether a precision higher than requested is available.
			higher_precision_available = int_precision_trait <n-1>::higher_precision_available
		};
	};

	/** \brief Declare util::int_precision_trait and util::int_bits_trait specialisations easily.

		\param bit_num			The number of bits of the type.
		\param int_type			The actual type.
		\param next_type		The type that has a higher precision, or
								int_type if there is none.
		\param next_available	Whether int_type is different from next_type
	*/
#define DECLARE_PRECISION_BITS(bit_num,int_type,unsigned_int_type,next_type,next_available) \
		template <> struct int_precision_trait <bit_num> { \
			typedef int_type type; \
			typedef next_type higher_precision_type; \
			typedef int_type lower_precision_type_help; \
			typedef int_precision_trait <static_comparison <bit_num-1, 0>::max> \
				::lower_precision_type_help lower_precision_type; \
			enum { \
				precision_available = true, \
				higher_precision_available = next_available \
			}; \
		};\
	\
		template <> struct int_bits_trait <int_type> { \
		enum { \
				precision = bit_num \
			}; \
			typedef unsigned_int_type unsigned_type; \
		};
		

	DECLARE_PRECISION_BITS(0, void, void, char, true)
	DECLARE_PRECISION_BITS(8, char, unsigned char, short, true)
	DECLARE_PRECISION_BITS(16, short, unsigned short, int, true)
	DECLARE_PRECISION_BITS(32, int, unsigned int, long_long_int, true)
	DECLARE_PRECISION_BITS(64, long_long_int, unsigned_long_long_int, long_long_int, false)

	/** \brief Return the lowest value a signed integer can have.

		\param int_type The integer type
	*/
	template <typename int_type>
		inline int_type get_min_int()
	{
		return int_type (int_type (1) << (int_bits_trait <int_type>::precision - 1));
	}

	/** \brief Return the highest value a signed integer can have.

		\param int_type The integer type
	*/
	template <typename int_type>
		inline int_type get_max_int()
	{
		return int_type (get_min_int <int_type> () - 1);
	}


/*	template <typename int_type, int n>
		inline int_type get_lsb_mask()
	{
		return (int_type (1) << n) - 1;
	}*/

	/*** fixed number precision calculations ***/

	template <int n1, int n2> class fixed;

#if defined (_MSC_VER) && (_MSC_VER < 1300)

	template <int n1, int n2, typename int_type, bool precision_available>
		struct fixed_precision_trait_help
	{
		template <bool _precision_available> struct _internal {
			enum {
				available_bits = int_bits_trait <int_type>::precision,
				fraction_bits = n2,
				integer_bits = n1
			};
		};

		template <> struct _internal <false> {
			enum {
				available_bits = int_bits_trait <int_type>::precision,
				fraction_bits = (n2 * available_bits) / (n1 + n2),
				integer_bits = available_bits - fraction_bits
			};
		};

		enum {
			available_bits = _internal <precision_available>::available_bits,
			fraction_bits = _internal <precision_available>::fraction_bits,
			integer_bits = _internal <precision_available>::integer_bits,
		};
	};

#else	// defined (_MSC_VER) && (_MSC_VER < 1300)

	template <int n1, int n2, typename int_type, bool precision_available>
		struct fixed_precision_trait_help
	{
		enum {
			available_bits = int_bits_trait <int_type>::precision,
			fraction_bits = n2,
			integer_bits = n1
		};
	};

	template <int n1, int n2, typename int_type>
		struct fixed_precision_trait_help <n1, n2, int_type, false>
	{
		enum {
			available_bits = int_bits_trait <int_type>::precision,
			fraction_bits = (n2 * available_bits) / (n1 + n2),
			integer_bits = available_bits - fraction_bits
		};
	};

#endif	// defined (_MSC_VER) && (_MSC_VER < 1300)

	/// \brief Calculates the actual bit size of fixed numbers at compile-time.
	template <int n1, int n2> struct fixed_precision_trait
	{
		typedef typename int_precision_trait <n1+n2>::type int_type;

		enum {
			/// \brief Whether an integer type with the requested number of bits
			/// is available.
			precision_available = int_precision_trait <n1 + n2>::precision_available,

			/// \brief The number of bits available.
			available_bits = fixed_precision_trait_help <n1, n2, int_type, precision_available>
				::available_bits,

			/// \brief The number of bits to be used for the fractional part.
			fraction_bits = fixed_precision_trait_help <n1, n2, int_type, precision_available>
				::fraction_bits,

			/// \brief The number of bits to be used for the integer part.
			integer_bits = fixed_precision_trait_help <n1, n2, int_type, precision_available>
				::integer_bits
		};
	};


	/*** shift_traits ***/

	// static_asserts are used for the specialisations;
	// MSVC does not like them to be initialised with = {} so they are used
	// by assigning them to themselves (to prevent warnings).

#if defined (_MSC_VER) && (_MSC_VER < 1300)

	template <int n_source, int n_dest, bool equal, typename int_type>
		struct shift_trait_greater_equal
	{
		template <bool _equal> struct _internal
		{
			static int_type get (const int_type & i)
			{
				static_assert <(n_source == n_dest)> _relative_size;
				_relative_size = _relative_size;

				return i;
			}

			static int_type get (const int_type & i0, const int_type & i1)
			{
				return i1;
			}

			static int_type get_unrounded (const int_type & i)
			{
				static_assert <(n_source == n_dest)> _relative_size;
				_relative_size = _relative_size;

				return i;
			}

			static bool yields_overflow (const int_type & i)
			{
				// No, obviously not, since i is not shifted
				return false;
			}

			static bool yields_overflow (const int_type & i0, const int_type & i1)
			{
				if (i0) {
					if (i0 != -1 || i1 >= 0)
						return true;
				}
				return false;
			}

			static bool yields_underflow (const int_type & i)
			{
				// No, obviously not, since i is not shifted
				return false;
			}
		};

		template <> struct _internal <false>
		{
			static int_type get (const int_type & i)
			{
				static_assert <(n_source > n_dest)> _relative_size;
				_relative_size = _relative_size;

				// Round to nearest n_dest int
				return (i + (int_type (1) << (n_source - n_dest - 1))) >> (n_source - n_dest);
			}

			static int_type get (const int_type & i0, const int_type & i1)
			{
				return (i0 << (int_bits_trait <int_type>::precision - (n_source - n_dest)))
					| (((i1 + (int_type (1) << (n_source - n_dest - 1))) >> (n_source - n_dest))
					& ~(int_type (-1) << ((int_bits_trait <int_type>::precision - (n_source - n_dest)))));
			}

			static int_type get_unrounded (const int_type & i)
			{
				static_assert <(n_source > n_dest)> _relative_size;
				_relative_size = _relative_size;

				// Round to nearest n_dest int
				return i >> (n_source - n_dest);
			}

			static bool yields_overflow (const int_type & i)
			{
				// No, obviously not since i is shifted right
				return false;
			}

			static bool yields_overflow (const int_type & i0, const int_type & i1)
			{
				if (i0 >> (n_source - n_dest))
				{	// i0 has bits where it shouldn't but maybe it's a negative number?
					if ((i0 >> (n_source - n_dest)) != -1 || get (i0, i1) >= 0)
						return true;
				}
				return false;
			}

			static bool yields_underflow (const int_type & i)
			{
				//return i != 0 && (i & get_lsb_mask <int_type, (n_source - n_dest)> ()) != 0;
				static_assert <false> _not_used;
			}
		};

		static int_type get (const int_type & i)
		{
			return _internal <equal>::get (i);
		}

		static int_type get (const int_type & i0, const int_type & i1)
		{
			return _internal <equal>::get (i0, i1);
		}

		static int_type get_unrounded (const int_type & i)
		{
			return _internal <equal>::get_unrounded (i);
		}

		static bool yields_overflow (const int_type & i0, const int_type & i1)
		{
			return _internal <equal>::yields_overflow (i0, i1);
		}

		static bool yields_underflow (const int_type & i)
		{
			return _internal <equal>::yields_underflow (i);
		}
	};

	template <int n_source, int n_dest, bool greater_equal, bool equal, typename int_type>
		struct shift_trait_help
	{
		template <bool _greater_equal> struct _internal
		{
			static int_type get (const int_type & i)
			{
				static_assert <(n_source >= n_dest)> _relative_size;
				_relative_size = _relative_size;

				return shift_trait_greater_equal <n_source, n_dest, equal, int_type>
					::get (i);
			}

			static int_type get (const int_type & i0, const int_type & i1)
			{
				static_assert <(n_source >= n_dest)> _relative_size;
				_relative_size = _relative_size;

				return shift_trait_greater_equal <n_source, n_dest, equal, int_type>
					::get (i0, i1);
			}

			static int_type get_unrounded (const int_type & i)
			{
				static_assert <(n_source >= n_dest)> _relative_size;
				_relative_size = _relative_size;

				return shift_trait_greater_equal <n_source, n_dest, equal, int_type>
					::get_unrounded (i);
			}

			static bool yields_overflow (const int_type & i)
			{
				i;
				// No, obviously not, since i is either shifted right, or not at all
				return false;
			}

			static bool yields_overflow (const int_type & i0, const int_type & i1)
			{
				return shift_trait_greater_equal <n_source, n_dest, equal, int_type>
					::yields_overflow (i0, i1);
			}

			static bool yields_underflow (const int_type & i)
			{
				return shift_trait_greater_equal <n_source, n_dest, equal, int_type>
					::yields_underflow (i);
			}
		};

		template <> struct _internal <false>
		{
			static int_type get (const int_type & i)
			{
				static_assert <(n_source < n_dest)> _relative_size;
				_relative_size = _relative_size;

				return i << (n_dest - n_source);
			}

			static int_type get (const int_type & i0, const int_type & i1)
			{
				return i1 << (n_dest - n_source);
			}

			static int_type get_unrounded (const int_type & i)
			{
				static_assert <(n_source < n_dest)> _relative_size = {};

				return i << (n_dest - n_source);
			}

			static bool yields_overflow (const int_type & i)
			{
				if (i < 0)
					return (get_min_int <int_type>() >> (n_dest - n_source)) > i;
				else
					return (get_max_int <int_type>() >> (n_dest - n_source)) < i;
			}

			static bool yields_overflow (const int_type & i0, const int_type & i1)
			{
				if (i0 == 0)
					return yields_overflow (i1);
				else {
					if (i0 == -1 && i1 < 0)
						return yields_overflow (i1);
					else
						return true;
				}
			}

			static bool yields_underflow (const int_type & i)
			{
				// No, obviously not, since i is shifted left
				return false;
			}
		};

		static int_type get (const int_type & i)
		{
			return _internal <greater_equal>::get (i);
		}

		static int_type get (const int_type & i0, const int_type & i1)
		{
			return _internal <greater_equal>::get (i0, i1);
		}

		static int_type get_unrounded (const int_type & i)
		{
			return _internal <greater_equal>::get_unrounded (i);
		}

		static bool yields_overflow (const int_type & i)
		{
			return _internal <greater_equal>::yields_overflow (i);
		}

		static bool yields_overflow (const int_type & i0, const int_type & i1)
		{
			return _internal <greater_equal>::yields_overflow (i0, i1);
		}

		static bool yields_underflow (const int_type & i)
		{
			return _internal <greater_equal>::yields_underflow (i);
		}
	};

#else	// defined (_MSC_VER) && (_MSC_VER < 1300)

	template <int n_source, int n_dest, bool greater_equal, bool equal, typename int_type>
		struct shift_trait_help
	{
		static int_type get (const int_type & i)
		{
			static_assert <(n_source == n_dest)> _relative_size = {};

			return i;
		}

		static int_type get (const int_type & i0, const int_type & i1)
		{
			return i1;
		}

		static int_type get_unrounded (const int_type & i)
		{
			static_assert <(n_source == n_dest)> _relative_size = {};

			return i;
		}

		static bool yields_overflow (const int_type & i)
		{
			// No, obviously not, since i is not shifted
			return false;
		}

		static bool yields_overflow (const int_type & i0, const int_type & i1)
		{
			if (i0) {
				if (i0 != -1 || i1 >= 0)
					return true;
			}
			return false;
		}

		static bool yields_underflow (const int_type & i)
		{
			// No, obviously not, since i is not shifted
			return false;
		}
	};

	template <int n_source, int n_dest, typename int_type>
		struct shift_trait_help <n_source, n_dest, true, false, int_type>
	{
		static int_type get (const int_type & i)
		{
			static_assert <(n_source > n_dest)> _relative_size = {};

			// Round to nearest n_dest int
			return (i + (int_type (1) << (n_source - n_dest - 1))) >> (n_source - n_dest);
		}

		static int_type get (const int_type & i0, const int_type & i1)
		{
			return (i0 << (int_bits_trait <int_type>::precision - (n_source - n_dest)))
				| (((i1 + (int_type (1) << (n_source - n_dest - 1))) >> (n_source - n_dest))
				& ~(int_type (-1) << ((int_bits_trait <int_type>::precision - (n_source - n_dest)))));
		}

		static int_type get_unrounded (const int_type & i)
		{
			static_assert <(n_source > n_dest)> _relative_size;
			_relative_size = _relative_size;

			// Round to nearest n_dest int
			return i >> (n_source - n_dest);
		}

		static bool yields_overflow (const int_type & i)
		{
			// No, obviously not since i is shifted right
			return false;
		}

		static bool yields_overflow (const int_type & i0, const int_type & i1)
		{
			if (i0 >> (n_source - n_dest))
			{	// i0 has bits where it shouldn't but maybe it's a negative number?
				if ((i0 >> (n_source - n_dest)) != -1 || get (i0, i1) >= 0)
					return true;
			}
			return false;
		}

		static bool yields_underflow (const int_type & i)
		{
			//return i != 0 && (i & get_lsb_mask <int_type, (n_source - n_dest)> ()) != 0;
			static_assert <false> _not_used;
		}
	};

	template <int n_source, int n_dest, typename int_type>
		struct shift_trait_help <n_source, n_dest, false, false, int_type>
	{
		static int_type get (const int_type & i)
		{
			static_assert <(n_source < n_dest)> _relative_size = {};

			return i << (n_dest - n_source);
		}

		static int_type get (const int_type & i0, const int_type & i1)
		{
			return i1 << (n_dest - n_source);
		}

		static int_type get_unrounded (const int_type & i)
		{
			static_assert <(n_source < n_dest)> _relative_size = {};

			return i << (n_dest - n_source);
		}

		static bool yields_overflow (const int_type & i)
		{
			if (i < 0)
				return (get_min_int <int_type>() >> (n_dest - n_source)) > i;
			else
				return (get_max_int <int_type>() >> (n_dest - n_source)) < i;
		}

		static bool yields_overflow (const int_type & i0, const int_type & i1)
		{
			if (i0 == 0)
				return yields_overflow (i1);
			else {
				if (i0 == -1 && i1 < 0)
					return yields_overflow (i1);
				else
					return true;
			}
		}

		static bool yields_underflow (const int_type & i)
		{
			// No, obviously not, since i is shifted left
			return false;
		}
	};

#endif	// defined (_MSC_VER) && (_MSC_VER < 1300)

	/** \brief Gives information about shifting integers for the util::fixed class.

		\param n_source	The number of fraction bits of the source type.
		\param n_dest	The number of fraction bits of the destination type.
	*/
	template <int n_source, int n_dest> struct shift_trait
	{
		/// \brief Shift an integer and round if necessary.
		template <typename int_type>
			static int_type get (const int_type & i)
		{
			check_overflow_shift <n_source, n_dest, int_type>::check (i);

			return shift_trait_help <n_source, n_dest, (n_source >= n_dest),
				(n_source == n_dest), int_type>::get (i);
		}

		/// \brief Shift two integers and round, returning one integer.
		template <typename int_type>
			static int_type get (const int_type & i0, const int_type & i1)
		{
			check_overflow_shift <n_source, n_dest, int_type>::check (i0, i1);

			return shift_trait_help <n_source, n_dest, (n_source >= n_dest),
				(n_source == n_dest), int_type>::get (i0, i1);
		}

		/// \brief Shift an integer without rounding.
		template <typename int_type>
			static int_type get_unrounded (const int_type & i)
		{
			check_overflow_shift <n_source, n_dest, int_type>::check (i);

			return shift_trait_help <n_source, n_dest, (n_source >= n_dest),
				(n_source == n_dest), int_type>::get_unrounded (i);
		}

		/// \brief Return whether the shift would result in an overflow.
		template <typename int_type>
			static bool yields_overflow (const int_type & i)
		{
			return shift_trait_help <n_source, n_dest, (n_source >= n_dest),
				(n_source == n_dest), int_type>::
				yields_overflow (i);
		}

		/// \brief Return whether the shift would result in an overflow.
		template <typename int_type>
			static bool yields_overflow (const int_type & i0, const int_type & i1)
		{
			return shift_trait_help <n_source, n_dest, (n_source >= n_dest),
				(n_source == n_dest), int_type>::
				yields_overflow (i0, i1);
		}

		/// \brief Return whether the shift would result in an underflow
		/// (i.e. whether the result is equal to 0 while the input is not).
		template <typename int_type>
			static bool yields_underflow (const int_type & i)
		{
			return shift_trait_help <n_source, n_dest, (n_source >= n_dest),
				(n_source == n_dest), int_type>::
				yields_underflow (i);
		}
	};


	/** \brief Shift an integer number for the util::fixed class.

		Automatically finds the type with the precision needed for
		shifting a fixed number to another fixed number.
		Overflow checking is done on the way.

		\param n1_source The number of integer bits of the source type.
		\param n2_source The number of fraction bits of the source type.
		\param n1_dest The number of integer bits of the destination type.
		\param n2_dest The number of fraction bits of the destination type.
	*/

	template <int n1_source, int n2_source, int n1_dest, int n2_dest>
		struct shift_fixed_trait
	{
		typedef typename int_precision_trait <static_comparison <
			n1_source + n2_source, n1_dest + n2_dest>::max>::type int_type;

		static int_type get (const int_type & i)
		{
			return shift_trait <n2_source, n2_dest>::get (i);
		}

		static int_type get_unrounded (const int_type & i)
		{
			return shift_trait <n2_source, n2_dest>::get_unrounded (i);
		}
	};

}

#endif	// PRECISION_TRAITS
