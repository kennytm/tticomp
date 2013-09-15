#ifndef STATIC_TEST_H
#define STATIC_TEST_H

// In some cases this is the only solution for the "typename or no typename" problem.
#ifdef _MSC_VER
#define TYPENAME
#else
#define TYPENAME typename
#endif

namespace util {

	/*** static_assert ***/

	/** \brief Provide static (i.e. compile-time) assertion ability

		The template class with parameter true is defined.
		If the struct is used with parameter false (static_assert <1==0> a;)
		then the compiler will produce an error because static_assert <false>
		has not been defined.
	*/
	template <bool b> struct static_assert;
	template <> struct static_assert <true> {};

	/*** static_comparison ***/

#ifdef _MSC_VER

	template <int n1, int n2, bool greater>
		struct static_comparison_help
	{
		template <bool _greater> struct _internal {
			enum {
				max = n1, min = n2
			};
		};

		template <> struct _internal <false> {
			enum {
				max = n2, min = n1
			};
		};

		enum {
			max = _internal <greater>::max,
			min = _internal <greater>::min
		};
	};


#else	// _MSC_VER

	template <int n1, int n2, bool greater>
		struct static_comparison_help
	{
		enum {
			max = n1, min = n2
		};
	};

	template <int n1, int n2>
		struct static_comparison_help <n1, n2, false>
	{
		enum {
			max = n2, min = n1
		};
	};

#endif	// _MSC_VER

	/// \brief Compare two integers at compile-time.
	template <int n1, int n2> struct static_comparison {
		enum {
			/// \brief The greatest of the two integers.
			max = static_comparison_help <n1, n2, (n1 > n2)>::max,
			/// \brief The smallest of the two integers.
			min = static_comparison_help <n1, n2, (n1 > n2)>::min
		};
	};

	/** \brief Return one of the parameters

		Both parameters will be evaluated.
		This is useful mostly for doing static optimisations:
		(is_negative ? -1 : 1)
		may be written
		static_take_one <is_negative>::get (-1, 1)
		if the value of is_negative is known at compile time.
		This may prevent compiler warnings.
	*/
	template <bool take_first>
		struct static_take_one
	{
		template <typename T>
			static T get (const T & first, const T & second)
		{
			return second;
		}
	};

	template <>
		struct static_take_one <true>
	{
		template <typename T>
			static T get (const T & first, const T & second)
		{
			return first;
		}
	};
}

#endif	// STATIC_TEST_H
