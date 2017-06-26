#include <stdlib.h>			// for exit(1)

template <class itemType>
class Vector
{
  public:

  // constructors/destructor
    Vector( );                        // default constructor (size==0)
    explicit Vector( int size );      // initial size of Vector is size
    Vector( int size, const itemType & fillValue ); // all entries == fillValue
    Vector( const Vector & vec );   // copy constructor
    ~Vector( );                       // destructor

  // assignment
    const Vector & operator = ( const Vector & vec );

  // accessors
    int  length( ) const;                   // capacity of Vector
	void fill( const itemType & fillValue ); // all entries == fillValue
	bool allowed_index ( int index ) const; // tests if index is allowed

  // indexing
    itemType &       operator [ ] ( int index );       // indexing with range checking
    const itemType & operator [ ] ( int index ) const; // indexing with range checking

  // modifiers
    void resize( int newSize );             // change size dynamically;
                                            // can result in losing values
  private:

    int  myCapacity;                        // space for elements
    itemType* myElements;                   // array used for storage
};



// these are examples of the syntax for member functions

template <class itemType>
Vector<itemType>::Vector()
{
	myCapacity = 0;
	myElements = NULL;
}


template <class itemType>
Vector<itemType>::Vector( int size )
{
	myCapacity = size;
	myElements = new itemType[size];
}

template <class itemType>
int Vector<itemType>::length() const
{
	return myCapacity;
}


template <class itemType>
itemType & Vector<itemType>::operator [ ] ( int index )
{
	if ( index < 0 || index >= myCapacity ) {
		cerr << "bad index in Vector::operator[] (" << index << ")\n";
		exit(1);
	}
	return myElements[index];
}


template <class itemType>
bool Vector<itemType>::allowed_index ( int index ) const
{
	return ( index >= 0 && index < myCapacity );
}


template <class itemType>
Vector<itemType>::Vector( int size, const itemType & fillValue )
{
	myCapacity = size;
	myElements = new itemType[size];
	for ( int i=0; i<size; ++i )
		myElements[i] = fillValue;
}


template <class itemType>
void Vector<itemType>::fill( const itemType & fillValue )
{
	for ( int i=0; i<myCapacity; ++i )
		myElements[i] = fillValue;
}

template <class itemType>
Vector<itemType>::Vector( const Vector<itemType> & vec )
{
	myCapacity = vec.myCapacity;
	myElements = new itemType[myCapacity];
	for ( int i=0; i<myCapacity; ++i )
		myElements[i] = vec.myElements[i];
}

template <class itemType>
Vector<itemType>::~Vector( )
{
	delete [] myElements;
}


template <class itemType>
const Vector<itemType> & Vector<itemType>::operator = ( const Vector<itemType> & vec )
{
	if ( this != &vec )
	{
		if ( myCapacity < vec.myCapacity )
		{
			delete [] myElements;
			myElements = new itemType[vec.myCapacity];
		}
		myCapacity = vec.myCapacity;
		for ( int i=0; i<myCapacity; ++i )
			myElements[i] = vec.myElements[i];
	}
	return *this;
}


template <class itemType>
const itemType & Vector<itemType>::operator [ ] ( int index ) const
{
	if ( index < 0 || index >= myCapacity ) {
		cerr << "bad index in Vector::operator[] (" << index << ")\n";
		exit(1);
	}
	return myElements[index];
}


template <class itemType>
void Vector<itemType>::resize( int newSize )
{
	if ( newSize == myCapacity )
		return;

	int min = ( newSize < myCapacity ) ? newSize : myCapacity;
	itemType* temp = new itemType[newSize];

	for ( int i=0; i<min; ++i )
		temp[i] = myElements[i];
	delete [] myElements;
	myElements = temp;
	myCapacity = newSize;
}

