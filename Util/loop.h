#ifndef LOOP_H
#define LOOP_H

#include <vector>

namespace util {

	template <typename T, typename I>
		class loop_iterator
	{
		I begin, end, i;

	public:
		const I & _get_begin() const { return begin; }
		const I & _get_end() const { return end; }
		const I & _get_i() const { return i; }

		loop_iterator() {}

		template <typename _T, typename _I>
			loop_iterator (const loop_iterator <_T, _I> & it)
			: begin (it._get_begin()), end (it._get_end()), i (it._get_i()) {}

		loop_iterator (const loop_iterator & _i)
			: begin (_i.begin), end (_i.end), i (_i.i) {}

		loop_iterator (I _begin, I _end, I _i)
			: begin (_begin), end (_end), i (_i) {}

		template <typename _T, typename _I>
			loop_iterator & operator = (const loop_iterator <_T, _I> & it)
		{
			begin = it._get_begin();
			end = it._get_end();
			i = it._get_i();
			return *this;
		}

		loop_iterator & operator = (const loop_iterator & it) {
			begin = it.begin;
			end = it.end;
			i = it.i;
			return *this;
		}

		loop_iterator & operator ++ () {
			++ i;
			if (i == end)
				i = begin;
			return *this;
		}
		loop_iterator & operator ++ (int) {
			++ i;
			if (i == end)
				i = begin;
			return *this;
		}

		loop_iterator & operator -- () {
			if (i == begin)
				i = end;
			-- i;
			return *this;
		}
		loop_iterator & operator -- (int) {
			if (i == begin)
				i = end;
			-- i;
			return *this;
		}

		loop_iterator & operator += (ptrdiff_t a) {
			a %= end - begin;
			if (a >= end - i)
				i += a - (end - begin);
			else {
				if (a < begin - i)
					i += a + (end - begin);
				else
					i += a;
			}
			return *this;
		}

		loop_iterator & operator -= (ptrdiff_t a) {
			a %= end - begin;
			if (a <= i - end)
				i -= a + (end - begin);
			else {
				if (a > i - begin)
					i -= a - (end - begin);
				else
					i -= a;
			}
			return *this;
		}

		/*loop_iterator & operator -= (ptrdiff_t a) {
			i = begin + ((i - begin - a) % (end - begin));
			return *this;
		}

		loop_iterator operator + (ptrdiff_t a) const {
			return loop_iterator (begin, end, begin + ((i - begin + a) % (end - begin)));
		}*/

		template <typename _T, typename _I>
			bool operator == (const loop_iterator <_T, _I> & it) const
		{
			return i == it._get_i();
		}

		template <typename _T, typename _I>
			bool operator != (const loop_iterator <_T, _I> & it) const
		{
			return i != it._get_i();
		}

	//	bool operator == (const loop_iterator & it) const { return i == it.i; }
	//	bool operator != (const loop_iterator & it) const { return i != it.i; }

		T & operator * () {
			return *i;
		}
		const T & operator * () const {
			return *i;
		}

		T * operator -> () {
			return &*i;
		}
		const T * operator -> () const {
			return &*i;
		}
	};

	template <typename T, class C = std::vector <T> >
		class loop
	{
		C container;

	public:
		typedef typename C::iterator internal_iterator;
		typedef typename C::const_iterator const_internal_iterator;

		typedef loop_iterator <T, internal_iterator> iterator;
		typedef loop_iterator <const T, const_internal_iterator> const_iterator;

	public:
		loop() {}

		inline iterator begin() {
			return iterator (container.begin(), container.end(), container.begin());
		}
		const_iterator begin() const {
			return const_iterator (container.begin(), container.end(), container.begin());
		}

		size_t size() const {
			return container.size();
		}

		bool empty() const {
			return container.empty();
		}

		void push (const T & e) {
			container.push_back (e);
		}

		iterator insert (iterator pos, const T & e) {
			typename C::iterator new_position = container.insert (pos._get_i(), e);
			return iterator (container.begin(), container.end(), new_position);
		}

		iterator erase (const iterator & pos) {
			typename C::iterator new_position = container.erase (pos._get_i());
			return iterator (container.begin(), container.end(), new_position);
		}

		T & operator [] (size_t index) {
			return container [index];
		}
		const T & operator [] (size_t index) const {
			return container [index];
		}

		T & at (size_t index) {
			return container.at (index);
		}
		const T & at (size_t index) const {
			return container.at (index);
		}
	};

} // namespace util

#endif // LOOP_H
