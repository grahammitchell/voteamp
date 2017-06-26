#ifndef __VECTOR_CPP
#define __VECTOR_CPP

template <class itemType>
class vector
{
  public:

  // constructors/destructor
    vector( );                        // default constructor (size==0)
    explicit vector( int size );      // initial size of vector is size
    vector( int size, const itemType & fillValue ); // all entries == fillValue
    vector( const vector & vec );   // copy constructor
    ~vector( );                       // destructor

  // assignment
    const vector & operator = ( const vector & vec );

  // accessors
    int  length( ) const;                   // capacity of vector

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
vector<itemType>::vector()
{
	myCapacity = 0;
	myElements = NULL;
}


template <class itemType>
vector<itemType>::vector( int size )
{
	myCapacity = size;
	myElements = new itemType[size];
}


template <class itemType>
vector<itemType>::~vector()
{
	delete [] myElements;
}


template <class itemType>
vector<itemType>::vector( int size, const itemType & fillValue )
{
	myCapacity = size;
	myElements = new itemType[size];
	for ( int i=0; i<size; ++i )
		myElements[i] = fillValue;
}


template <class itemType>
vector<itemType>::vector( const vector<itemType> & vec )
{
	myCapacity = vec.myCapacity;
	myElements = new itemType[myCapacity];
	for ( int i=0; i<myCapacity; ++i )
		myElements[i] = vec.myElements[i];
}


template <class itemType>
const vector<itemType> & vector<itemType>::operator = ( const vector<itemType> & rhs )
{
	if ( this != &rhs )
	{
		delete [] myElements;
		myCapacity = rhs.myCapacity;
		myElements = new itemType[myCapacity];
		for ( int i=0; i<myCapacity; ++i )
			myElements[i] = rhs.myElements[i];
	}
	return *this;
}


template <class itemType>
int vector<itemType>::length() const
{
	return myCapacity;
}


template <class itemType>
itemType & vector<itemType>::operator [ ] ( int index )
{
	if ( index < 0 || index >= myCapacity ) {
		cerr << "bad index in vector::operator[] (" << index << ")\n";
		exit(1);
	}
	return myElements[index];
}


template <class itemType>
const itemType & vector<itemType>::operator [ ] ( int index ) const
{
	if ( index < 0 || index >= myCapacity ) {
		cerr << "bad index in vector::operator[] (" << index << ")\n";
		exit(1);
	}
	return myElements[index];
}


template <class itemType>
void vector<itemType>::resize( int newSize )
{
	itemType* temp = new itemType[newSize];
	int max = ( newSize < myCapacity ) ? newSize : myCapacity;
	for ( int i=0; i<max; ++i )
		temp[i] = myElements[i];

	delete [] myElements;
	myCapacity = newSize;
	myElements = temp;
}

#endif
