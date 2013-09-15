#ifndef DOUBLE_PRECISION_MULT
#define DOUBLE_PRECISION_MULT

/**
	\file double_precision.h \brief Defines routines that provide all the
	precision needed when multiplying or dividing two integers.
*/

#include "static_test.h"
#include "precision_trait.h"

namespace util {

#if defined (_MSC_VER) && (_MSC_VER < 1300)

	template <int n_source, int n_dest, int n_argument, int n_return, int n_max,
		bool double_precision_needed>
		struct multiply_and_shift_help
	{
		typedef typename int_precision_trait <n_argument>::type argument_type;
		typedef typename int_precision_trait <n_return>::type return_type;
		typedef typename int_precision_trait <n_max>::type max_type;

		template <bool _double_precision_needed>
			struct _internal
		{
			static return_type do_multiply_and_shift (const argument_type & a, const argument_type & b)
			{
				return check_overflow_conversion <return_type> (
					shift_trait <n_source, n_dest>::get (max_type (a) * b));
			}
		};

		template <>
			struct _internal <true>
		{
			static return_type do_multiply_and_shift (const argument_type & a, const argument_type & b)
			{
				// The max_type type must be as large as the argument_type
				// (otherwise the simpler version could have been called)
				static_assert <(int_bits_trait <argument_type>::precision ==
					int_bits_trait <max_type>::precision)> _assert_max_is_max;
				_assert_max_is_max = _assert_max_is_max;

				typedef typename int_precision_trait <int_bits_trait <argument_type>::precision / 2>::type
					half_int_type;
				typedef typename int_precision_trait <int_bits_trait <argument_type>::precision / 2>::type
					unsigned_half_int_type;

				static_assert <(int_bits_trait <argument_type>::precision ==
					int_bits_trait <half_int_type>::precision * 2)> _assert_int_size;
				_assert_int_size = _assert_int_size;

				argument_type a0 = a >> (int_bits_trait <argument_type>::precision / 2);
				argument_type a1 = a & ~(argument_type(-1) << (int_bits_trait <argument_type>::precision / 2));

				argument_type b0 = b >> (int_bits_trait <argument_type>::precision / 2);
				argument_type b1 = b & ~(argument_type(-1) << (int_bits_trait <argument_type>::precision / 2));

				if (a1 == a && b1 == b)
					return shift_trait <n_source, n_dest>::get (a * b);
				else
				{
					argument_type r0, r1;
					r0 = a0 * b0;
					r1 = a1 * b1;
					argument_type temp = a1 * b0 + a0 * b1;
					r0 += temp >> (int_bits_trait <argument_type>::precision / 2);
					r1 += temp << (int_bits_trait <argument_type>::precision / 2);

					return return_type (shift_trait <n_source, n_dest>::get (r0, r1));
				}
			}
		};

		static return_type do_multiply_and_shift (const argument_type & a, const argument_type & b) {
			return _internal <double_precision_needed>::do_multiply_and_shift (a, b);
		}
	};

#else // defined (_MSC_VER) && (_MSC_VER < 1300)

	template <int n_source, int n_dest, int n_argument, int n_return, int n_max,
		bool double_precision_needed>
		struct multiply_and_shift_help
	{
		typedef typename int_precision_trait <n_argument>::type argument_type;
		typedef typename int_precision_trait <n_return>::type return_type;
		typedef typename int_precision_trait <n_max>::type max_type;

		static return_type do_multiply_and_shift (const argument_type & a, const argument_type & b)
		{
			return check_overflow_conversion <return_type> (
				shift_trait <n_source, n_dest>::get (max_type (a) * b));
		}
	};

	template <int n_source, int n_dest, int n_argument, int n_return, int n_max>
		struct multiply_and_shift_help <n_source, n_dest, n_argument, n_return, n_max,
		true>
	{
		typedef typename int_precision_trait <n_argument>::type argument_type;
		typedef typename int_precision_trait <n_return>::type return_type;
		typedef typename int_precision_trait <n_max>::type max_type;

		static return_type do_multiply_and_shift (const argument_type & a, const argument_type & b)
		{
			// The max_type type must be as large as the argument_type
			// (otherwise the simpler version could have been called)
			static_assert <(int_bits_trait <argument_type>::precision ==
				int_bits_trait <max_type>::precision)> _assert_max_is_max;
			_assert_max_is_max = _assert_max_is_max;

			typedef typename int_precision_trait <int_bits_trait <argument_type>::precision / 2>::type
				half_int_type;
			typedef typename int_precision_trait <int_bits_trait <argument_type>::precision / 2>::type
				unsigned_half_int_type;

			static_assert <(int_bits_trait <argument_type>::precision ==
				int_bits_trait <half_int_type>::precision * 2)> _assert_int_size;
			_assert_int_size = _assert_int_size;

			argument_type a0 = a >> (int_bits_trait <argument_type>::precision / 2);
			argument_type a1 = a & ~(argument_type(-1) << (int_bits_trait <argument_type>::precision / 2));

			argument_type b0 = b >> (int_bits_trait <argument_type>::precision / 2);
			argument_type b1 = b & ~(argument_type(-1) << (int_bits_trait <argument_type>::precision / 2));

			if (a1 == a && b1 == b)
				return shift_trait <n_source, n_dest>::get (a * b);
			else
			{
				argument_type r0, r1;
				r0 = a0 * b0;
				r1 = a1 * b1;
				argument_type temp = a1 * b0 + a0 * b1;
				r0 += temp >> (int_bits_trait <argument_type>::precision / 2);
				r1 += temp << (int_bits_trait <argument_type>::precision / 2);

				return return_type (shift_trait <n_source, n_dest>::get (r0, r1));
			}
		}
	};

#endif // defined (_MSC_VER) && (_MSC_VER < 1300)

	/// \brief Multiply two numbers and shift right.
	///
	/// The implementation will provide extra precision as needed.
	/// An overflow_exception is only thrown if the final result does not
	/// fit in the return type.
	///
	/// \param n_source The number of fraction bits for the arguments
	/// when they are multiplied, i.e., the sum of the numbers of the fraction bits
	/// for the arguments.
	/// \param n_dest The number of fraction bits for the destination type.
	/// \param n_argument The largest number of bits for the arguments.
	/// \param n_return The largest number of bits for the result.
	/// \param n_max The maximum number of bits needed for the calculation, i.e.
	/// the sum of the number of bits for both arguments.
	///
	/// n_argument, n_return and n_max are used to find the argument, return
	/// and calculation type.
	/// If there is no native integer containing n_max bits, the calculation
	/// is done with two integers, which is probably slow.
	///
	/// \see divide_and_shift
	template <int n_source, int n_dest, int n_argument, int n_return, int n_max>
		struct multiply_and_shift
	{
		typedef typename int_precision_trait <n_argument>::type argument_type;
		typedef typename int_precision_trait <n_return>::type return_type;
		typedef typename int_precision_trait <n_max>::type max_type;

		/// \brief Return the result of the multiplication.
		static return_type get (const argument_type & a, const argument_type & b)
		{
			// The double-precision version is needed only if max_type cannot
			// contain n_max bits.
			return multiply_and_shift_help <n_source, n_dest, n_argument, n_return, n_max,
				(int_bits_trait <max_type>::precision < n_max)>
				::do_multiply_and_shift (a, b);
		}
	};


#if defined (_MSC_VER) && (_MSC_VER < 1300)

	template <int n_dividend_int, int n_dividend_frac, int n_divisor_int, int n_divisor_frac,
		int n_return_int, int n_return_frac, bool do_round, bool extra_bits_needed>
		struct divide_and_shift_help
	{
		enum {
			n_dividend = n_dividend_int + n_dividend_frac,
			n_divisor = n_divisor_int + n_divisor_frac,
			n_return = n_return_int + n_return_frac,
			n_max = n_dividend + (n_return_frac - (n_dividend_frac - n_divisor_frac))
		};
		typedef typename int_precision_trait <n_dividend>::type dividend_type;
		typedef typename int_precision_trait <n_divisor>::type divisor_type;
		typedef typename int_precision_trait <static_comparison <n_dividend, n_divisor>::max>
			::type max_argument_type;
		typedef typename int_precision_trait <n_return>::type return_type;
		typedef typename int_precision_trait <n_max>::type max_type;

		template <bool _extra_bits_needed>
			struct _internal
		{
			static return_type get (const dividend_type & a, const divisor_type & b)
			{
				// a/b returns a number with (n_dividend_frac - n_divisor_frac) fraction bits
				return check_overflow_fit <n_return, return_type>::check (
					check_overflow_conversion <return_type, max_type> (
					((max_type (a) << (n_return_frac - (n_dividend_frac - n_divisor_frac))) +
					static_take_one <do_round>::get (divisor_type (b/2), divisor_type (0))) / b));
			}
		};

		template <>
			struct _internal <true>
		{
			static return_type get (max_argument_type a, divisor_type b)
			{
				bool neg;
				if (a < 0) {
					a = -a;
					neg = true;
				} else
					neg = false;
				if (b < 0) {
					b = -b;
					neg = !neg;
				}
				int fraction_bits_left = n_return_frac - (n_dividend_frac - n_divisor_frac);

				// Round if the last 
				if (fraction_bits_left <= 0)
					a += static_take_one <do_round>::get (
						divisor_type ((b/2) << fraction_bits_left), divisor_type (0));

				// a/b returns a number with (n_dividend_frac - n_divisor_frac) fraction bits
				return_type result = check_overflow_conversion <return_type, max_argument_type> (a / b);

				if (fraction_bits_left <= 0) {
					result >>= fraction_bits_left;
				} else {
					a %= b;
					while (fraction_bits_left)
					{
						// shift result left 1 with overflow checking
						result = shift_trait <0, 1>::get (result);
						a <<= 1;

						fraction_bits_left --;
						// Round when the last bit is in sight
						if (!fraction_bits_left)
							a += static_take_one <do_round>::get (divisor_type (b/2), divisor_type (0));

						result |= return_type (a)/b;
						a %= b;
					}
				}

				if (neg)
					return check_overflow_fit <n_return, return_type>::check (-result);
				else
					return check_overflow_fit <n_return, return_type>::check (result);
			}
		};

		static return_type get (const dividend_type & a, const divisor_type & b)
		{
			return _internal <extra_bits_needed>::get (a, b);
		}
	};
#else	// (_MSC_VER) && (_MSC_VER < 1300)

	template <int n_dividend_int, int n_dividend_frac, int n_divisor_int, int n_divisor_frac,
		int n_return_int, int n_return_frac, bool do_round, bool extra_bits_needed>
		struct divide_and_shift_help
	{
		enum {
			n_dividend = n_dividend_int + n_dividend_frac,
			n_divisor = n_divisor_int + n_divisor_frac,
			n_return = n_return_int + n_return_frac,
			n_max = n_dividend + (n_return_frac - (n_dividend_frac - n_divisor_frac))
		};
		typedef typename int_precision_trait <n_dividend>::type dividend_type;
		typedef typename int_precision_trait <n_divisor>::type divisor_type;
		typedef typename int_precision_trait <n_return>::type return_type;
		typedef typename int_precision_trait <n_max>::type max_type;

		static return_type get (const dividend_type & a, const divisor_type & b)
		{
			// a/b returns a number with (n_dividend_frac - n_divisor_frac) fraction bits
			return check_overflow_fit <n_return, return_type>::check (
				check_overflow_conversion <return_type, max_type> (
				((max_type (a) << (n_return_frac - (n_dividend_frac - n_divisor_frac))) +
				static_take_one <do_round>::get (divisor_type (b/2), divisor_type (0))) / b));
		}
	};

	template <int n_dividend_int, int n_dividend_frac, int n_divisor_int, int n_divisor_frac,
		int n_return_int, int n_return_frac, bool do_round>
		struct divide_and_shift_help <n_dividend_int, n_dividend_frac, n_divisor_int, n_divisor_frac,
		n_return_int, n_return_frac, do_round, true>
	{
		enum {
			n_dividend = n_dividend_int + n_dividend_frac,
			n_divisor = n_divisor_int + n_divisor_frac,
			n_return = n_return_int + n_return_frac
		};
		typedef typename int_precision_trait <n_dividend>::type dividend_type;
		typedef typename int_precision_trait <n_divisor>::type divisor_type;
		typedef typename int_precision_trait <static_comparison <n_dividend, n_divisor>::max>
			::type max_argument_type;
		typedef typename int_precision_trait <n_return>::type return_type;

		static return_type get (max_argument_type a, divisor_type b)
		{
			bool neg;
			if (a < 0) {
				a = -a;
				neg = true;
			} else
				neg = false;
			if (b < 0) {
				b = -b;
				neg = !neg;
			}
			int fraction_bits_left = n_return_frac - (n_dividend_frac - n_divisor_frac);

			// Round if the last 
			if (fraction_bits_left <= 0)
				a += static_take_one <do_round>::get (
					divisor_type ((b/2) << fraction_bits_left), divisor_type (0));

			// a/b returns a number with (n_dividend_frac - n_divisor_frac) fraction bits
			return_type result = check_overflow_conversion <return_type, max_argument_type> (a / b);

			if (fraction_bits_left <= 0) {
				result >>= fraction_bits_left;
			} else {
				a %= b;
				while (fraction_bits_left)
				{
					// shift result left 1 with overflow checking
					result = shift_trait <0, 1>::get (result);
					a <<= 1;

					fraction_bits_left --;
					// Round when the last bit is in sight
					if (!fraction_bits_left)
						a += static_take_one <do_round>::get (divisor_type (b/2), divisor_type (0));

					result |= return_type (a)/b;
					a %= b;
				}
			}

			if (neg)
				return check_overflow_fit <n_return, return_type>::check (-result);
			else
				return check_overflow_fit <n_return, return_type>::check (result);
		}
	};
#endif	// (_MSC_VER) && (_MSC_VER < 1300)

	/// \brief Divide two numbers and shift right.
	///
	/// The implementation will provide extra precision as needed.
	/// An overflow_exception is only thrown if the final result does not
	/// fit in the return type.
	///
	/// \param n_dividend_int The number of integer bits for the first argument.
	/// \param n_dividend_frac The number of fraction bits for the first argument.
	/// \param n_divisor_int The number of integer bits for the second argument.
	/// \param n_divisor_frac The number of fraction bits for the second argument.
	/// \param n_return_int The number of integer bits for the result.
	/// \param n_return_frac The number of fraction bits for the result.
	/// \param do_round Whether or not to round to the nearest integer.
	///
	/// The first 6 parameters are used to find the argument, return
	/// and calculation type.
	/// If there is no native integer containing enough bits, the calculation
	/// is done bit by bit, which is probably \em very slow.
	///
	/// \see multiply_and_shift

	template <int n_dividend_int, int n_dividend_frac, int n_divisor_int, int n_divisor_frac,
		int n_return_int, int n_return_frac, bool do_round = true>
		struct divide_and_shift
	{
		enum {
			n_dividend = n_dividend_int + n_dividend_frac,
			n_divisor = n_divisor_int + n_divisor_frac,
			n_return = n_return_int + n_return_frac,
			n_max = n_dividend + (n_return_frac - (n_dividend_frac - n_divisor_frac))
		};
		typedef typename int_precision_trait <n_dividend>::type dividend_type;
		typedef typename int_precision_trait <n_divisor>::type divisor_type;
		typedef typename int_precision_trait <n_return>::type return_type;
		typedef typename int_precision_trait <n_max>::type max_type;

		/// \brief Return the result of the division.
		static return_type get (const dividend_type & a, const divisor_type & b)
		{
			return divide_and_shift_help <n_dividend_int, n_dividend_frac,
				n_divisor_int, n_divisor_frac, n_return_int, n_return_frac, do_round,
				(int (int_bits_trait <max_type>::precision) < int (n_max))>
				::get (a, b);
		}
	};


} // namespace util

#endif	// DOUBLE_PRECISION_MULT
