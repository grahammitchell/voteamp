#ifndef __VECTOR_H
#define __VECTOR_H

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

#endif
