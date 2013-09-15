/*
	(c) Copyright 2002, 2003 Rogier van Dalen
	(R.C.van.Dalen@umail.leidenuniv.nl for any comments, questions or bugs)

	This file is part of my OpenType/TrueType Font Tools.

	The OpenType/TrueType Font Tools is free software; you can redistribute
	it and/or modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation; either version 2 of the
	License, or (at your option) any later version.

	The OpenType/TrueType Font Tools is distributed in the hope that it will
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
	Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Foobar; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/**
	\file smart_ptr.h
	Smart pointer contains a smart pointer that will delete itself as soon
	as the reference count becomes 0.

	NEVER use this pointer to point to existing objects; use only new objects
	or other smart pointers.
****************************************************************************/

#ifndef SMART_PTR_H
#define SMART_PTR_H

#include <cassert>
#include <ostream>

namespace util {

/**
	\brief A generic reference-counted Smart Pointer class

	smart_ptr may be used to keep track of all pointers to an object. The object
	will be automatically deleted when all references have been deleted.
	Do not keep any non-smart_ptr pointers to the object, because they are
	not seen by smart_ptr so that the objects they are referring to may
	be deleted quite suddenly.

	If you get any warnings about deleting incomplete classes, make sure
	you include the class declaration in every file where the Smart Pointer
	could be deleted. E.g.,

  (Foo.h)
\code
	#include "Bar.h"
	class Foo {
		smart_ptr <Bar> bar;
	public:
		~Foo() {}
	}
\endcode
	will need to have Bar declared in that very file.
	To get a slightly nicer dependency tree, you'd better make sure you
	provide all destructor implementations in your .cpp files and not in
	header files. E.g.,

  (Foo.h)
\code
	class Foo {
		smart_ptr <Bar> bar;
	public:
		~Foo();
	}
\endcode

  (Foo.cpp)
\code
	#include "Bar.h"

	Foo::~Foo() {}
\endcode
*/

	template <typename Type>
	class smart_ptr {
	public:

		class smart_ptr_reference {
			Type *object;
			int ref_count;
		public:
			smart_ptr_reference (Type *_object) : object (_object), ref_count (1) {
				assert (_object != NULL);
			}
			
			~smart_ptr_reference() {
				assert (ref_count == 0);
				// If you get an error here about "deletion of undeclared pointer type",
				// see above.
				delete object;
			}
			
			Type * get() const { return object; }
			
			void increase_ref_count()  {
				ref_count ++;
			}
			void release()  {
				ref_count --;
				if (ref_count == 0)
					delete this;
			}
		};

		smart_ptr_reference *ref;

	public:
		/// Initialise a smart_ptr with a NULL reference.
		smart_ptr () : ref (NULL) {}

		/// \brief Constructor that takes an object of type Type.
		/// \param newObject The object to be referenced. Do not ever leave a
		/// normal pointer to this object around.
		/// This constructor should normally only be called with <tt>new Type (...)</tt>.
		smart_ptr (Type *new_object) {
			if (new_object)
				ref = new smart_ptr_reference (new_object);
			else
				ref = NULL;
		}

		/// \brief Generalised constructor: construct a smart_ptr from a derived type.
		/// \param p A smart_ptr to a derived type
		template <typename DerivedType>
			smart_ptr <Type> (const smart_ptr <DerivedType> &p) {
	#ifdef _DEBUG
			// You'll get an error here if you try to set this to an underived class.
			Type *test_whether_it_is_really_derived = (DerivedType *) 0;
	#endif
			ref = (smart_ptr_reference *) p.ref;
			if (ref)
				ref->increase_ref_count();
		}

		/// \brief Copy constructor
		///
		/// This will not copy the object, but only increase a reference counter.
		/// \param p A smart_ptr to the object.
		smart_ptr (const smart_ptr <Type> &p) {
			ref = p.ref;
			if (ref)
				ref->increase_ref_count();
		}

		~smart_ptr () {
			if (ref)
				ref->release();
		}

		smart_ptr <Type> & operator = (const smart_ptr <Type> &p) {
			if (p.ref)
				p.ref->increase_ref_count();
			if (ref)
				ref->release();
			ref = p.ref;
			return *this;
		}

		smart_ptr <Type> & operator = (Type *aNewObject) {
			if (ref)
				ref->release();
			if (aNewObject)
				ref = new smart_ptr_reference (aNewObject);
			else
				ref = NULL;
			return *this;
		}

		/// \brief Return true if this refers to an object; false if NULL.
		operator bool () const {
			return ref != NULL;
		}

		/// \brief Return true if this does not refer to an object; false if not NULL.
		bool operator ! () const {
			return ref == NULL;
		}

		/// \brief Returns true if this and p point to the same object
		bool operator == (const smart_ptr <Type> &p2) const {
			assert (ref == NULL || p2.ref == NULL || ref != p2.ref || ref->get() == ref->get());
			return ref == p2.ref;
		}

		bool operator == (const Type *p2) const {
			return (ref == NULL && p2 == NULL) || ref->get() == p2;
		}
		
		bool operator != (const smart_ptr <Type> &p2) const {
			assert (ref == NULL || p2.ref == NULL || ref != p2.ref || ref->get() == ref->get());
			return ref != p2.ref;
		}

		bool operator != (const Type *p2) const {
			return !((ref == NULL && p2 == NULL) || ref->get() == p2);
		}

		Type * operator -> () const {
			return ref->get();
		}

		Type & operator * () const {
			return *ref->get();
		}

	/*	const Type * operator -> () const {
			return ref->get();
		}

		const Type & operator * () const {
			return *ref->get();
		}*/

	#ifdef SMART_PTR_GET_METHOD
		Type * get() const {
			return ref->get();
		}
	#endif

	#ifdef CAST_SMART_PTR_TO_TYPE
		operator Type * () {
			return ref->get();
		}
	#endif
	};

	// Usage for this function:
	// smart_ptr_cast <SomeType> (thisObjectIsDerivedFromSomeType)
	template <typename DerivedType, typename BaseType>
	inline smart_ptr <DerivedType> smart_ptr_cast (const smart_ptr <BaseType> &p) {
		// Check whether p is really a DerivedType
		assert (!p || dynamic_cast <const DerivedType*> (&*p));
		return (smart_ptr <DerivedType> &) p;
	}

}	// namespace util

template <typename Type>
inline std::ostream &operator << (std::ostream & o, const util::smart_ptr <Type> & p) {
	return (o << *p);
}

template <typename Type>
inline std::ostream &operator << (std::ostream & o, util::smart_ptr <Type> & p) {
	return (o << *p);
}

#endif	// SMART_PTR_H
