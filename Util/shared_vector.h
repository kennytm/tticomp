
#ifndef SHARED_VECTOR_H
#define SHARED_VECTOR_H

#include <vector>

namespace util {

	template <typename T, class Alloc = std::allocator<T> >
		class shared_vector_object : public std::vector <T, Alloc>
	{
	public:
		typedef typename std::vector <T, Alloc>::size_type size_type;
	private:
		unsigned int ref_count;
	public:
		shared_vector_object () : ref_count (1) {}

		shared_vector_object (size_type n) : ref_count (1), std::vector <T, Alloc> (n) {}
		shared_vector_object (size_type n, const T & t)
			: ref_count (1), std::vector <T, Alloc> (n, t) {}

		shared_vector_object (const shared_vector_object & s)
			: ref_count (1), std::vector<T, Alloc> (s) {}

		shared_vector_object (const std::vector <T, Alloc> & v)
			: ref_count (1), std::vector<T, Alloc> (v) {}

		template <class InputIterator>
			shared_vector_object (InputIterator i1, InputIterator i2)
			: ref_count (1), std::vector <T, Alloc> (i1, i2) {}

		void increase_ref_count() { ref_count ++; }
		void decrease_ref_count() {
			ref_count --;
			if (!ref_count)
				delete this;
		}

		bool is_own_object() const {
			return (ref_count == 1);
		}

		shared_vector_object * get_own_object() {
			if (is_own_object())
				return this;
			else {
				ref_count --;
				return new shared_vector_object (*this);
			}
		}
	};

	template <typename T, class Alloc = std::allocator<T> >
		class shared_vector
	{
	public:
		typedef shared_vector_object <T, Alloc> object_type;
		typedef T value_type;
		typedef T * pointer;
		typedef T & reference;
		typedef const T & const_reference;
		typedef typename object_type::size_type size_type;
		typedef typename object_type::difference_type difference_type;
		typedef typename object_type::iterator iterator;
		typedef typename object_type::const_iterator const_iterator;
		typedef typename object_type::reverse_iterator reverse_iterator;
		typedef typename object_type::const_reverse_iterator const_reverse_iterator;

	private:
		object_type * v;

	public:
		// Construction
		shared_vector() {
			v = new object_type();
		}

		shared_vector (size_type n) {
			v = new object_type (n);
		}

		shared_vector (size_type n, const T& t) {
			v = new object_type (n, t);
		}

		shared_vector (const shared_vector & s) {
			v = s.v;
			v->increase_ref_count();
		}

		shared_vector (const std::vector <T, Alloc> & s) {
			v = new object_type (s);
		}

		template <class InputIterator>
			shared_vector (InputIterator i1, InputIterator i2)
		{
			v = new object_type (i1, i2);
		}

		~shared_vector() {
			v->decrease_ref_count();
		}

		shared_vector & operator = (const shared_vector & s) {
			s.v->increase_ref_count();
			v->decrease_ref_count();
			v = s.v;
			return *this;
		}

		shared_vector & operator = (const std::vector <T, Alloc> & s) {
			v->decrease_ref_count();
			v = new object_type (s);
			return *this;
		}

		// Access

		iterator begin() {
			v = v->get_own_object();
			return v->begin();
		}
		iterator end() {
			v = v->get_own_object();
			return v->end();
		}
		const_iterator begin() const {
			return ((object_type const *)v)->begin();
		}
		const_iterator end() const {
			return ((object_type const *)v)->end();
		}

		reverse_iterator rbegin() {
			v = v->get_own_object();
			return v->rbegin();
		}
		reverse_iterator rend() {
			v = v->get_own_object();
			return v->rend();
		}
		const_reverse_iterator rbegin() const {
			return ((object_type const *)v)->rbegin();
		}
		const_reverse_iterator rend() const {
			return ((object_type const *)v)->rend();
		}

		size_type size() const { return v->size(); }
		size_type max_size() const { return v->max_size(); }
		size_type capacity() const { return v->capacity(); }
		bool empty() const { return v->empty(); }

		reference operator[] (size_type n) {
			v = v->get_own_object();
			return v->operator[] (n);
		}
		const_reference operator[] (size_type n) const {
			v = v->get_own_object();
			return v->operator[] (n);
		}

		const std::vector <T, Alloc> & get_vector() const {
			return *v;
		}


		// Other
		void reserve (size_type n) {
			v = v->get_own_object();
			v->reserve (n);
		}

		reference front() {
			v = v->get_own_object();
			return v->front();
		}
		const_reference front() const {
			return ((object_type const *)v)->front();
		}
		reference back() {
			v = v->get_own_object();
			return v->back();
		}
		const_reference back() const {
			return ((object_type const *)v)->back();
		}

		void push_back (const T& t) {
			v = v->get_own_object();
			v->push_back (t);
		}
		void pop_back() {
			v = v->get_own_object();
			b->pop_back();
		}

		iterator insert (iterator pos, const T & x) {
			v = v->get_own_object();
			return v->insert (pos, x);
		}

		template <class InputIterator>
			void insert (iterator pos, InputIterator f, InputIterator l)
		{
			v = v->get_own_object();
			v->insert (pos, f, l);
		}

		void insert (iterator pos, size_type n, const T & x) {
			v = v->get_own_object();
			v->insert (pos, n, x);
		}

		iterator erase (iterator pos) {
			v = v->get_own_object();
			return v->erase (pos);
		}

		iterator erase (iterator f, iterator l) {
			v = v->get_own_object();
			return v->erase (f, l);
		}

		void clear() {
			v = v->get_own_object();
			v->clear();
		}

		bool operator == (const shared_vector & s) {
			if (v == s.v)
				return true;
			return *v == *s.v;
		}

		bool operator < (const shared_vector & s) {
			if (v == s.v)
				return false;
			return *v < *s.v;
		}
	};
}

#endif	// SHARED_VECTOR_H
