#ifndef FIXED_H
#define FIXED_H

#ifdef _MSC_VER
#pragma warning (disable: 4097)
#endif

#include <iostream>
#include <cmath>

#include "precision_trait.h"
#include "check_overflow.h"
#include "double_precision.h"

namespace util {

	struct fixed_fraction {};
	struct fixed_unrounded {};

	/**
		\brief A fixed number class that uses an integer that fits the
		desired precision

		The precision is given by the template parameters n1 and n2, which are
		the number of bits in front of the point and the number of bits following.
		Thus, fixed <2,4> is a number in the range [-2,1.9375] and works with
		steps of size 0.0625.

		The precision will be automatically converted to a sensible precision
		when using the operators. For example, adding 2 fixed <16,16> will
		return a fixed <16,16> object.
		The product of a fixed <2,4> and a fixed <5,7> will be a fixed <7,11>
		object.

		If the number of required bits is not available, the largest integer
		type available will be picked and the number of bits will be divided
		equally.
		Thus, a fixed <48,16> number on a system that has only 32 bits integers
		will have precision 24.8.

		Conversion of one fixed number type to another will happen automatically.
		All conversions are produced at compile-time to yield the best performance
		in optimised build.
		Non-optimised builds, however, may be much slower due to large numbers
		of extra function calls.

		By default any overflow errors are detected and overflow_exceptions may be thrown.
		i is guaranteed always to fit in the desired precision (i.e. fixed<4,0> can
		contain only 4-bits values even if the internal integer type used is larger.
		If an overflow_exception is thrown on construction, the fixed has the value 0.

		\see overflow_exception
		\see int_precision_trait
	 */

	template <int n1, int n2>
		class fixed
	{
	public:
		/// \brief The integer type used for the internal representation
		typedef typename int_precision_trait <n1 + n2>::type int_type;

		enum {
			/// \brief The actual number of bits used for the integer part
			integer_bits = fixed_precision_trait <n1, n2>::integer_bits,
			/// \brief The actual number of bits used for the fraction part
			fraction_bits = fixed_precision_trait <n1, n2>::fraction_bits
		};

	private:
		int_type i;
	public:
		/** \brief Initialise with value 0.

			Internally, get() is called to make sure it is there in debug builds.
			This may be useful when debugging.
			The call will be removed by the optimisation pass of the compiler
			for release builds.
		*/
		fixed () : i (0)
		{
			get();
		}

		/** \brief Initialise with another fixed's value.

			The integer is shifted as needed, rounding to the nearest
			value available in this fixed.
		*/
		template <int _n1, int _n2>
			fixed (const fixed <_n1, _n2> & f) : i (0)
		{
			typedef fixed <_n1, _n2> operand_type;

			i = check_overflow_fit <integer_bits + fraction_bits, int_type>::check (
				int_type (shift_fixed_trait <
				operand_type::integer_bits, operand_type::fraction_bits,
				integer_bits, fraction_bits>::get (f.get_i())));
		}

		/** \brief Initialise with another fixed's value.

			The integer is shifted as needed, rounding down.
		*/
		template <int _n1, int _n2>
			fixed (const fixed <_n1, _n2> & f, const fixed_unrounded & _unrounded)
			: i (0)
		{
			typedef fixed <_n1, _n2> operand_type;

			i = check_overflow_fit <integer_bits + fraction_bits, int_type>::check (
				int_type (shift_fixed_trait <
				operand_type::integer_bits, operand_type::fraction_bits,
				integer_bits, fraction_bits>::get_unrounded (f.get_i())));
		}

		/// \brief Initialise with another fixed's value.
		fixed (const fixed & f)
			: i (0)
		{
			i = check_overflow_fit <integer_bits + fraction_bits, int_type>::check (f.i);

			get();
		}

		/** \brief Initialise with an explicit value.

			The value must be shifted already, i.e., it must contain a
			fraction representation.
			The fixed_fraction struct passed is merely syntactic.
			You may use fixed <n1, n2> (i, fixed_fraction()).
		*/
		fixed (int_type _i, fixed_fraction _f)
			: i (0)
		{
			i = check_overflow_fit <integer_bits + fraction_bits, int_type>::check (_i);

			get();

			// Prevent compiler warnings
			_f = _f;
		}

		/** \brief Initialise with an integer value.

			The value is shifted to get the fixed number.
		*/
		fixed (int _i)
			: i (0)
		{
			check_overflow_shift <0, fraction_bits, int_type>::check (
				check_overflow_conversion <int_type> (_i));

			i = check_overflow_fit <integer_bits + fraction_bits, int_type>::check (
				int_type (int_type (_i) << fraction_bits));

			get();
		}

		/** \brief Initialise with an unsigned integer value.

			The value is shifted to get the fixed number.
		*/
		fixed (unsigned int _i)
			: i (0)
		{
			check_overflow_shift <0, fraction_bits, int_type>::check (
				check_overflow_conversion_unsigned <int_type> (_i));

			i = check_overflow_fit <integer_bits + fraction_bits, int_type>::check (
				int_type (int_type (_i) << fraction_bits));

			get();
		}

		/** \brief Initialise with a double value.

			An overflow_exception is thrown if the double value is too large.
		*/
		explicit fixed (double d)
			: i (0)
		{
			check_overflow_conversion <int_type> (d * (int_type (1) << fraction_bits));

			i = check_overflow_fit <integer_bits + fraction_bits, int_type>::check (
				(d < 0) ?
				int_type (d * (int_type (1) << fraction_bits) - .5)
				: int_type (d * (int_type (1) << fraction_bits) + .5));

			get();
		}

		// Assignment operators

		/** \brief Assign to another fixed's value.

			The integer is shifted as needed, rounding to the nearest
			value available in this fixed.
		*/
		template <int _n1, int _n2>
			fixed & operator = (const fixed <_n1, _n2> & f)
		{
			typedef fixed <_n1, _n2> operand_type;
			i = check_overflow_fit <integer_bits + fraction_bits, int_type>::check (
				int_type (shift_fixed_trait <
				operand_type::integer_bits, operand_type::fraction_bits,
				integer_bits, fraction_bits>::get (f.get_i())));

			return *this;
		}

		/** \brief Assign to an integer value.

			The value is shifted to get the fixed number.
		*/
		fixed & operator = (int _i)
		{
			check_overflow_conversion <int_type> (_i);

			i = check_overflow_fit <integer_bits + fraction_bits, int_type>::check (
				shift_trait <0, fraction_bits>::get (int_type (_i)));

			return *this;
		}

		/** \brief Add another fixed's value to this.

			Any other types than fixed <n1, n2> should be converted to fixed first anyway,
			so no overloaded operators += are available for for example ints or doubles.

			An overflow_exception is thrown if an overflow occurs.
		*/
		fixed & operator += (const fixed & o) {
			check_overflow_addition <int_type> (i, o.i);
			i = check_overflow_fit <integer_bits + fraction_bits, int_type>::check (
				i + o.i);

			return *this;
		}

		/** \brief Subtract another fixed's value from this.

			Any other types than fixed <n1, n2> should be converted to fixed first anyway,
			so no overloaded operators -= are available for for example ints or doubles.

			An overflow_exception is thrown if an overflow occurs.
		*/
		fixed & operator -= (const fixed & o) {
			check_overflow_addition <int_type> (i, -o.i);
			i = check_overflow_fit <integer_bits + fraction_bits, int_type>::check (
				i - o.i);

			return *this;
		}

		/** \brief Multiply this with an integer value.

			An overflow_exception is thrown if an overflow occurs.
		*/
		fixed & operator *= (int o) {
			check_overflow_conversion <int_type> (o);
			check_overflow_multiplication <int_type> (i, int_type (o));
			i = check_overflow_fit <integer_bits + fraction_bits, int_type>::check (
				i * o);

			return *this;
		}

		/** \brief Multiply this with another fixed's value.

			The result is rounded to the nearest value available.

			An overflow_exception is thrown if an overflow occurs.
		*/
		template <int _n1, int _n2>
			fixed & operator *= (const fixed <_n1, _n2> & f)
		{
			typedef fixed <_n1, _n2> operand_type;
			typedef typename fixed <n1 + _n1, n2 + _n2>::int_type large_int_type;

			enum {
				this_bits = integer_bits + fraction_bits,
				operand_bits = operand_type::integer_bits + operand_type::fraction_bits
			};

			i = check_overflow_fit <integer_bits + fraction_bits, int_type>::check (
				multiply_and_shift <operand_type::fraction_bits, 0, 
				static_comparison <this_bits, operand_bits>::max, this_bits, this_bits + operand_bits>
				::get (i, f.get_i()));

			return *this;
		}

		/** \brief Divide this by an integer value.

			The result is rounded to the nearest value available.
		*/
		fixed & operator /= (int o) {
			if (i < 0)
				i = (i - (o/2)) / o;
			else
				i = (i + (o/2)) / o;
			return *this;
		}

		/** \brief Divide this by an integer value.

			The result is rounded to the nearest value available.

			An overflow_exception is thrown if an overflow occurs.
		*/
		template <int _n1, int _n2>
			fixed & operator /= (const fixed <_n1, _n2> & f)
		{
			typedef fixed <_n1, _n2> operand_type;

			i = check_overflow_fit <integer_bits + fraction_bits, int_type>::check (
				divide_and_shift <integer_bits, fraction_bits,
				operand_type::integer_bits, operand_type::fraction_bits,
				integer_bits, fraction_bits>::get (i, f.get_i()));
		}

		// const operators

		/** \brief Return the product of this and an integer value.

			The return type has extra integer bits depending on the size
			of the integer.
		*/
		fixed <n1 + int_bits_trait <int>::precision, n2>
			operator * (int o) const
		{
			typedef int operand_type;
			typedef fixed <n1 + int_bits_trait <operand_type>::precision, n2> return_type;
			typedef typename return_type::int_type return_int_type;

			check_overflow_conversion <return_int_type> (o);

			enum {
				this_bits = integer_bits + fraction_bits,
				//operand_bits = operand_type::integer_bits + operand_type::fraction_bits,
				operand_bits = int_bits_trait <operand_type>::precision,
				return_bits = return_type::integer_bits + return_type::fraction_bits
			};

			return return_type (
				multiply_and_shift <fraction_bits, return_type::fraction_bits,
				static_comparison <this_bits, operand_bits>::max, return_bits, this_bits + operand_bits>
				::get (i, o), fixed_fraction());
		}

		/** \brief Return the product of this and another fixed value.

			The return type has the total number of integer bits and
			the total number of fraction bits, to make sure the full
			possible range and precision are used.
			This means that the integer size increases quickly when doing
			a number of multiplications in one statement.

			Assigning the result to another fixed will, however, reduce the
			number of bits to the number specified for that fixed.
			Beware of overflow errors ocurring caused by the automatic conversion
			to a certain fixed type, especially when the number of total bits
			required exceeds the maximum number of bits available and the excess
			integer and fraction bits are reduced.
		*/
		template <int _n1, int _n2>
			fixed <n1 + _n1, n2 + _n2>
			operator * (const fixed <_n1, _n2> & f) const
		{
			typedef fixed <_n1, _n2> operand_type;
			typedef fixed <n1 + _n1, n2 + _n2> return_type;
			typedef typename return_type::int_type return_int_type;

			enum {
				this_bits = integer_bits + fraction_bits,
				operand_bits = operand_type::integer_bits + operand_type::fraction_bits,
				return_bits = return_type::integer_bits + return_type::fraction_bits
			};

			return return_type (
				multiply_and_shift <fraction_bits + operand_type::fraction_bits,
				return_type::fraction_bits, static_comparison <this_bits, operand_bits>::max,
				return_bits, this_bits + operand_bits>
				::get (i, f.get_i()), fixed_fraction());
		}

		/** \brief Return this divided by an integer value.

			The return type has extra fraction bits depending on the size
			of the integer.
		*/
		fixed <n1, n2 + int_bits_trait <int>::precision>
			operator / (int o) const
		{
			typedef int operand_type;
			typedef fixed <n1, n2 + int_bits_trait <operand_type>::precision> return_type;
			typedef typename return_type::int_type return_int_type;

			return return_type (
				divide_and_shift <integer_bits, fraction_bits,
				int_bits_trait <operand_type>::precision, 0,
				return_type::integer_bits, return_type::fraction_bits>::get (i, o),
				fixed_fraction());
		}

		/** \brief Return the this divided by another fixed number.

			The return type of n1.n2 / _n1._n2 has (n1+_n2).(n2+_n1) as precision.
			This must be merely a rule of thumb because it is possible that
			the actual values wildly differ from what is expected.
			In many cases, however, this will provide a sufficiently good precision.

			An overflow_exception may occur if the return value does not fit.
			Beware that multiple divisions and multiplications may yield integer
			sizes that are not available; thus, overflow_exceptions may be thrown
			sooner than expected.
		*/
		template <int _n1, int _n2>
			fixed <n1 + _n2, n2 + _n1>
			operator / (const fixed <_n1, _n2> & f) const
		{
			typedef fixed <_n1, _n2> operand_type;
			typedef fixed <n1 + _n2, n2 + _n1> return_type;
			typedef typename return_type::int_type return_int_type;
			typedef typename fixed <integer_bits,
				operand_type::fraction_bits + return_type::fraction_bits>::int_type
				large_int_type;

			return return_type (
				divide_and_shift <integer_bits, fraction_bits,
				operand_type::integer_bits, operand_type::fraction_bits,
				return_type::integer_bits, return_type::fraction_bits>::get (i, f.get_i()),
				fixed_fraction());
		}

		/** \brief Return this divided by another fixed number.

			The return type of n1.n2 / _n1._n2 has (n1+_n2).(n2+_n1) as precision.
			This must be merely a rule of thumb because it is possible that
			the actual values wildly differ from what is expected.
			In many cases, however, this will provide a sufficiently good precision.
		*/
		template <int _n1, int _n2>
			fixed <n1 + _n2, n2 + _n1>
			divide_unrounded (const fixed <_n1, _n2> & f) const
		{
			typedef fixed <_n1, _n2> operand_type;
			typedef fixed <n1 + _n2, n2 + _n1> return_type;
			typedef typename return_type::int_type return_int_type;
			typedef typename fixed <integer_bits,
				operand_type::fraction_bits + return_type::fraction_bits>::int_type
				large_int_type;

			return return_type (
				divide_and_shift <integer_bits, fraction_bits,
				operand_type::integer_bits, operand_type::fraction_bits,
				return_type::integer_bits, return_type::fraction_bits, false>::get (i, f.get_i()),
				fixed_fraction());
		}

		/** \brief Return this added to the integer number.

			If the integer has more bits than the integer bits of this
			fixed, the result has more integer bits as well.

			An overflow_exception may be thrown when an overflow occurs.
		*/
		fixed <static_comparison <n1, int_bits_trait<int>::precision>::max, n2>
			operator + (int o) const
		{
			typedef fixed <static_comparison <n1, int_bits_trait<int>::precision>::max, n2>
				return_type;
			typedef typename return_type::int_type return_int_type;
			check_overflow_conversion <return_int_type> (o);
			check_overflow_shift <0, return_type::fraction_bits, return_int_type>::check (o);
			check_overflow_addition <return_int_type> (
				shift_trait <fraction_bits, return_type::fraction_bits>::get (i),
				(return_int_type (o) << return_type::fraction_bits));

			return return_type (
				shift_trait <fraction_bits, return_type::fraction_bits>::get (i)
				+ (return_int_type (o) << return_type::fraction_bits), fixed_fraction());
		}

		/** \brief Return this added to the unsigned integer number.

			If the integer has more bits than the integer bits of this
			fixed, the result has more integer bits as well.

			An overflow_exception may be thrown when an overflow occurs.
		*/
		fixed <static_comparison <n1, int_bits_trait<int>::precision + 1>::max, n2>
			operator + (unsigned int o) const
		{
			typedef fixed <static_comparison <n1, int_bits_trait<int>::precision + 1>::max, n2>
				return_type;
			typedef typename return_type::int_type return_int_type;
			check_overflow_conversion <return_int_type> (o);
			check_overflow_shift <0, return_type::fraction_bits, return_int_type>::check (o);
			check_overflow_addition <return_int_type> (
				shift_trait <fraction_bits, return_type::fraction_bits>::get (i),
				(return_int_type (o) << return_type::fraction_bits));

			return return_type (
				shift_trait <fraction_bits, return_type::fraction_bits>::get (i)
				+ (return_int_type (o) << return_type::fraction_bits), fixed_fraction());
		}

		/** \brief Return this added to the other fixed.

			If the other fixed has more integer or fraction bits than this
			fixed, the result will have a higher precision.

			An overflow_exception may be thrown when an exception occurs.
		*/
		template <int _n1, int _n2>
			fixed <static_comparison <n1, _n1>::max, static_comparison <n2, _n2>::max>
			operator + (const fixed <_n1, _n2> & f) const
		{
			typedef fixed <_n1, _n2> operand_type;
			typedef fixed <static_comparison <n1, _n1>::max, static_comparison <n2, _n2>::max> return_type;
			typedef typename return_type::int_type return_int_type;

			check_overflow_conversion <return_int_type> (f.get_i());

			return return_type (check_overflow_addition <return_int_type> (
				shift_trait <fraction_bits, return_type::fraction_bits>::get (return_int_type (i)),
				shift_trait <operand_type::fraction_bits, return_type::fraction_bits>::get (return_int_type (f.get_i()))),
				fixed_fraction());
		}

		/** \brief Return this with the integer number subtracted.

			If the integer has more bits than the integer bits of this
			fixed, the result has more integer bits as well.

			An overflow_exception may be thrown when an overflow occurs.
		*/
		fixed <static_comparison <n1, int_bits_trait<int>::precision>::max, n2>
			operator - (int o) const
		{
			typedef fixed <static_comparison <n1, int_bits_trait<int>::precision>::max, n2>
				return_type;
			typedef typename return_type::int_type return_int_type;
			check_overflow_conversion <return_int_type> (o);
			check_overflow_shift <0, return_type::fraction_bits, return_int_type>::check (-o);
			check_overflow_addition <return_int_type> (
				shift_trait <fraction_bits, return_type::fraction_bits>::get (i),
				(return_int_type (-o) << return_type::fraction_bits));

			return return_type (
				shift_trait <fraction_bits, return_type::fraction_bits>::get (i)
				+ (return_int_type (-o) << return_type::fraction_bits), fixed_fraction());
		}

		/** \brief Return this with the other fixed subtracted.

			If the other fixed has more integer or fraction bits than this
			fixed, the result will have a higher precision.

			An overflow_exception may be thrown when an exception occurs.
		*/
		template <int _n1, int _n2>
			fixed <static_comparison <n1, _n1>::max, static_comparison <n2, _n2>::max>
			operator - (const fixed <_n1, _n2> & f) const
		{
			typedef fixed <_n1, _n2> operand_type;
			typedef fixed <static_comparison <n1, _n1>::max, static_comparison <n2, _n2>::max> return_type;
			typedef typename return_type::int_type return_int_type;

			check_overflow_conversion <return_int_type> (-f.get_i());
			check_overflow_shift <0, fraction_bits, return_int_type>::check (-f.get_i());
			check_overflow_addition <return_int_type> (i, ((-f.get_i()) << fraction_bits));

			return return_type (shift_trait <fraction_bits, return_type::fraction_bits>::get (return_int_type (i)) +
				shift_trait <operand_type::fraction_bits, return_type::fraction_bits>::get (return_int_type (- f.get_i())),
				fixed_fraction());
		}

		/** \brief Return the negation of this.
		*/
		fixed operator - () const {
			return fixed (int_type (-i), fixed_fraction());
		}

		// comparing operators

		/** \brief Return true iff the integer is equal to this.
		*/
		bool operator == (int _i) const
		{
			return (int_type (_i) << fraction_bits) == i &&
				((int_type (_i) << fraction_bits) >> fraction_bits) == _i;
		}

		/** \brief Return true iff the fixed value is equal to this.
		*/
		template <int _n1, int _n2>
			bool operator == (const fixed <_n1, _n2> & f) const
		{
			typedef fixed <_n1, _n2> operand_type;

			if (shift_trait <operand_type::fraction_bits, fraction_bits>::yields_overflow (int_type (f.get_i())))
			{	// f.i is out of bounds
				return false;
			}
			if (shift_trait <operand_type::fraction_bits, fraction_bits>::yields_underflow (f.get_i()))
			{
				return false;
			}

			return i == shift_trait <operand_type::fraction_bits, fraction_bits>::get (f.get_i());
		}

		/** \brief Return false iff the integer is equal to this.
		*/
		bool operator != (int i) const {
			return !(*this == i);
		}

		/** \brief Return false iff the fixed value is equal to this.
		*/
		template <int _n1, int _n2>
			bool operator != (const fixed <_n1, _n2> & f) const
		{
			return !(*this == f);
		}

		/** \brief Return true iff this is smaller than the integer.
		*/
		bool operator < (int o) const
		{
			if (((int_type (o) << fraction_bits) >> fraction_bits) != o)
				// o is out of bound for this type
				return o > 0;
			return i < (int_type (o) << fraction_bits);
		}

		/** \brief Return true iff this is smaller than the fixed.
		*/
		template <int _n1, int _n2>
			bool operator < (const fixed <_n1, _n2> & f) const
		{
			typedef fixed <_n1, _n2> operand_type;

			if (shift_trait <operand_type::fraction_bits, fraction_bits>::yields_overflow (int_type (f.get_i())))
			{	// f.i is out of bounds
				// f is greater than this iff it's greater than 0.
				return f.get_i() > 0;
			}
			if (shift_trait <operand_type::fraction_bits, fraction_bits>::yields_underflow (f.get_i()))
			{
				// if i and (f.i >> n) are equal, it is because some bits were chopped
				// off when shifting, so f is slightly greater than this.
				return i <= shift_trait <operand_type::fraction_bits, fraction_bits>::get_unrounded (f.get_i());
			}

			return i < shift_trait <operand_type::fraction_bits, fraction_bits>::get (f.get_i());
		}

		/** \brief Return true iff this is greater than the integer.
		*/
		bool operator > (int o) const {
			return !(*this == o || *this < o);
		}

		/** \brief Return true iff this is greater than the fixed.
		*/
		template <int _n1, int _n2>
			bool operator > (const fixed <_n1, _n2> & f) const
		{
			return !(*this == f || *this < f);
		}

		/** \brief Return true iff this is smaller than or equal to the integer.
		*/
		bool operator <= (int o) const {
			return *this == o || *this < o;
		}

		/** \brief Return true iff this is smaller than or equal to the fixed.
		*/
		template <int _n1, int _n2>
			bool operator <= (const fixed <_n1, _n2> & f) const
		{
			return *this == f || *this < f;
		}

		/** \brief Return true iff this is greater than or equal to the integer.
		*/
		bool operator >= (int o) const {
			return *this == o || *this > o;
		}

		/** \brief Return true iff this is greater than or equal to the fixed.
		*/
		template <int _n1, int _n2>
			bool operator >= (const fixed <_n1, _n2> & f) const
		{
			return *this == f || *this > f;
		}

		// const get methods

		/** \brief Return the value this has as a double.
		*/
		double get() const {
			return double (i) / (int_type (1) << fraction_bits);
		}

		/** \brief Return the raw integer.

			This is equal to *this * (2 ** fraction_bits).
		*/
		int_type get_i() const {
			return i;
		}

		/** \brief Return the first integer value lower than this.
		*/
		int_type get_integer() const {
			return int_type (i >> fraction_bits);
		}

		/** \brief Return the first fixed with an integer value lower than this.
		*/
		fixed get_floor() const {
			return fixed (i & (int_type (-1) << fraction_bits), fixed_fraction());
		}

		/** \brief Return the first fixed with an integer value higher than this.
		*/
		fixed get_ceiling() const {
			check_overflow_addition <int_type> (i, ~(int_type (-1) << fraction_bits));

			return fixed (int_type ((i + ~(int_type (-1) << fraction_bits))
				& (int_type (-1) << fraction_bits)), fixed_fraction());
		}

		/** \brief Return the fraction part of this.
		*/
		fixed get_fraction() const {
			return fixed (int_type (i & ~(int_type (-1) << fraction_bits)), fixed_fraction());
		}

		/** \brief Return the integer nearest to this.
		*/
		int_type round() const {
			// The simple implementation would allow overflow errors to occur
//			return (i + (1 << (fraction_bits - 1))) >> fraction_bits;

			if (i & (1 << (fraction_bits - 1)))
				return (i >> fraction_bits) + 1;
			else
				return (i >> fraction_bits);
		}
	};

	/*** Operators ***/

	template <int n1, int n2>
		inline std::ostream & operator << (std::ostream & o, fixed <n1, n2> f)
	{
		typedef fixed <n1, n2> fixed_type;

		if (f < 0) {
			o << "-";
			f = -f;
		}

		o << f.get_integer() << ".";
		f = f.get_fraction();
		typename fixed_type::int_type left =
			(fixed_type::int_type (1) << fixed_type::fraction_bits);

		while (left != 0) {
			left /= 10;
			f *= 10;
			if (!left)
				f += fixed_type (0.5);
			o << char ('0' + f.get_integer());
			f = f.get_fraction();
		}

		return o;
	}

	/*** int and fixed operators ***/

	template <int n1, int n2>
		inline fixed <n1 + int_bits_trait <int>::precision, n2>
		operator * (int i, const fixed <n1, n2> & f)
	{
		return f * i;
	}

	template <int n1, int n2>
		inline fixed <n1 + int_bits_trait <int>::precision, n2>
		operator * (unsigned int i, const fixed <n1, n2> & f)
	{
		return f * i;
	}

	template <int n1, int n2>
		inline fixed <n1, n2 + int_bits_trait <int>::precision>
		operator / (int i, const fixed <n1, n2> & f)
	{
		return fixed <int_bits_trait <int>::precision, 0> (i) / f;
	}

	template <int n1, int n2>
		inline fixed <static_comparison <n1, int_bits_trait<int>::precision>::max, n2>
		operator + (int i, const fixed <n1, n2> & f)
	{
		return f + i;
	}

	template <int n1, int n2>
		inline fixed <static_comparison <n1, int_bits_trait<int>::precision>::max, n2>
		operator - (int i, const fixed <n1, n2> & f)
	{
		return - f + i;
	}

	template <int n1, int n2>
		inline bool operator == (int i, const fixed <n1, n2> & f)
	{
		return f == i;
	}

	template <int n1, int n2>
		inline bool operator != (int i, const fixed <n1, n2> & f)
	{
		return f != i;
	}

	template <int n1, int n2>
		inline bool operator < (int i, const fixed <n1, n2> & f)
	{
		return f > i;
	}

	template <int n1, int n2>
		inline bool operator > (int i, const fixed <n1, n2> & f)
	{
		return f < i;
	}

	template <int n1, int n2>
		inline bool operator <= (int i, const fixed <n1, n2> & f)
	{
		return f >= i;
	}

	template <int n1, int n2>
		inline bool operator >= (int i, const fixed <n1, n2> & f)
	{
		return f <= i;
	}

	/*** fixed and double operators ***/
	
	template <int n1, int n2>
		inline double operator * (const fixed <n1, n2> & f, double d)
	{
		return f.get() * d;
	}

	template <int n1, int n2>
		inline double operator / (const fixed <n1, n2> & f, double d)
	{
		return f.get() / d;
	}

	template <int n1, int n2>
		inline double operator + (const fixed <n1, n2> & f, double d)
	{
		return f.get() + d;
	}

	template <int n1, int n2>
		inline double operator - (const fixed <n1, n2> & f, double d)
	{
		return f.get() - d;
	}

	template <int n1, int n2>
		inline bool operator == (const fixed <n1, n2> & f, double d)
	{
		return f.get() == d;
	}

	template <int n1, int n2>
		inline bool operator != (const fixed <n1, n2> & f, double d)
	{
		return f.get() != d;
	}

	template <int n1, int n2>
		inline bool operator < (const fixed <n1, n2> & f, double d)
	{
		return f.get() < d;
	}

	template <int n1, int n2>
		inline bool operator > (const fixed <n1, n2> & f, double d)
	{
		return f.get() > d;
	}

	template <int n1, int n2>
		inline bool operator <= (const fixed <n1, n2> & f, double d)
	{
		return f.get() <= d;
	}

	template <int n1, int n2>
		inline bool operator >= (const fixed <n1, n2> & f, double d)
	{
		return f.get() >= d;
	}
	
	/*** double and fixed operators ***/
	
	template <int n1, int n2>
		inline double operator * (double d, const fixed <n1, n2> & f)
	{
		return d * f.get();
	}

	template <int n1, int n2>
		inline double operator / (double d, const fixed <n1, n2> & f)
	{
		return d / f.get();
	}

	template <int n1, int n2>
		inline double operator + (double d, const fixed <n1, n2> & f)
	{
		return d + f.get();
	}

	template <int n1, int n2>
		inline double operator - (double d, const fixed <n1, n2> & f)
	{
		return d - f.get();
	}

	template <int n1, int n2>
		inline bool operator == (double d, const fixed <n1, n2> & f)
	{
		return d == f.get();
	}

	template <int n1, int n2>
		inline bool operator != (double d, const fixed <n1, n2> & f)
	{
		return d != f.get();
	}

	template <int n1, int n2>
		inline bool operator < (double d, const fixed <n1, n2> & f)
	{
		return d < f.get();
	}

	template <int n1, int n2>
		inline bool operator > (double d, const fixed <n1, n2> & f)
	{
		return d > f.get();
	}

	template <int n1, int n2>
		inline bool operator <= (double d, const fixed <n1, n2> & f)
	{
		return d <= f.get();
	}

	template <int n1, int n2>
		inline bool operator >= (double d, const fixed <n1, n2> & f)
	{
		return d >= f.get();
	}
}

/*** Overloaded functions ***/
// Must be outside the namespace to make them globally accessible.

template <int n1, int n2>
	inline double sqrt (const util::fixed <n1, n2> & f)
{
	return sqrt (f.get());
}

template <int n1, int n2>
	inline util::fixed <n1, n1> floor (const util::fixed <n1, n2> & f)
{
	return f.get_floor();
}

template <int n1, int n2>
	inline util::fixed <n1, n2> fraction (const util::fixed <n1, n2> & f)
{
	return f.get_fraction();
}

template <int n1, int n2>
	inline util::fixed <n1, n2> abs (const util::fixed <n1, n2> & f)
{
	if (f.get_i() < 0)
		return -f;
	else
		return f;
}

template <int n1, int n2>
	inline typename util::fixed <n1, n2>::int_type
	round (const util::fixed <n1, n2> & f)
{
	return f.round();
}


#endif	// FIXED_H

