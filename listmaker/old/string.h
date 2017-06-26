#ifndef __STRING_
#define __STRING_

#include <iostream.h>

// *******************************************************************
//  APCS string class
//
//  string class consistent with a subset of the standard C++ string class
//  as defined in the draft ANSI standard
// *******************************************************************

extern const int npos;  // used to indicate not a position in the string

class string
{
  public:

  // constructors/destructor

    string( );                     // construct empty string ""
    string( char c );              // construct from character literal
    string( const char * s );      // construct from null-terminated char array
    string( const string & str );  // copy constructor
    ~string( );                    // destructor

  // assignment

    const string & operator = ( const string & str ); // assign str
    const string & operator = ( const char * s );       // assign s
    const string & operator = ( char ch );              // assign ch

  // accessors

    int    length( )                  const;  // number of chars
    int    find( const string & str ) const;  // index of first occurrence of str
    int    find( char ch )            const;  // index of first occurrence of ch
    string substr( int pos, int len ) const;  // substring of len chars, starting at pos
    const char * c_str( )             const;  // explicit conversion to char *

  // indexing

    char   operator[ ] ( int k )      const;  // range-checked indexing
	                                          //   (return value is read-only)
    char & operator[ ] ( int k );             // range-checked indexing
	                                          //   (return value can be l-value)

  // modifiers

    const string & operator += ( const string & str );  // append another string
    const string & operator += ( char ch );             // append a char


  private:
      int myLength;                     // length of string (# of characters)
      int myCapacity;                   // capacity of string
      char * myCstring;                 // storage for characters
};

// The following free (non-member) functions operate on strings
//
// I/O functions

ostream & operator << ( ostream & os, const string & str );
istream & operator >> ( istream & is, string & str );
istream & getline( istream & is, string & str );

// comparison operators:

bool operator == ( const string & lhs, const string & rhs );
bool operator != ( const string & lhs, const string & rhs );
bool operator <  ( const string & lhs, const string & rhs );
bool operator <= ( const string & lhs, const string & rhs );
bool operator >  ( const string & lhs, const string & rhs );
bool operator >= ( const string & lhs, const string & rhs );

// concatenation operator +

string operator + ( const string & lhs, const string & rhs );
string operator + ( char ch, const string & str );
string operator + ( const string & str, char ch );

// *******************************************************************
// Specifications for string functions
//
// Any violation of a function's precondition should result in an error
// message followed by a call to exit().
//
// The string class assumes that '\0' is not a valid
// character in an string. Any attempts to place '\0'
// in an string will result in undefined behavior. Generally
// this means that characters that follow the '\0' will not
// be considered part of the string for purposes of
// comparison, output, and subsequent copying.
//
// constructors / destructor
//
// string( )
//    postcondition: string is empty
//
// string( char c )
//    description:   constructs a string object from a literal char such as 'a'
//    precondition:  c is a character other than '\0'
//    postcondition: a string of length 1 containing the character c has
//                   been constructed
//
// string( const char * s )
//    description:   constructs a string object from a literal C-string
//                   such as "abcd"
//    precondition:  s is '\0'-terminated string as used in C
//    postcondition: copy of s has been constructed
//
// string( const string & str )
//    description:   copy constructor
//    postcondition: deep copy of str has been constructed
//
// ~string( );
//    description:   destructor
//    postcondition: string is destroyed
//
// assignment
//
// string & operator = ( const string & str )
//    postcondition: normal assignment via copying has been performed
//
// string & operator = ( const char * s )
//    description:   assignment from literal string such as "abcd"
//    precondition:  s is '\0'-terminated string as used in C
//    postcondition: assignment via copying of s has been performed
//
// string & operator = ( char ch )
//    description:   assignment from character as though single char string
//    postcondition: assignment of one-character string has been performed
//
// accessors
//
// int length( ) const;
//    postcondition: returns # of chars in string
//
// int find( const string & str)  const;
//    description:   find the first occurrence of the string str within this
//                   string and return the index of the first character.  If
//                   str does not occur in this string, then return npos.
//    precondition:  this string represents c0, c1, ..., c(n-1)
//                   str represents s0, s1, ...,s(m-1)
//    postcondition: if s0 == ck0, s1 == ck1, ..., s(m-1) == ck(m-1) and
//                   there is no j < k0 such that s0 = cj, ...., sm == c(j+m-1),
//                   then returns k0;
//                   otherwise returns npos
//
// int find( char ch ) const;
//    description:   finds the first occurrence of the character ch within this
//                   string and returns the index.  If ch does not occur in this
//                   string, then returns npos.
//    precondition:  this string represents c0, c1, ..., c(n-1)
//    postcondition: if ch == ck, and there is no j < k such that ch == cj
//                   then returns k;
//                   otherwise returns npos
//
// string substr( int pos, int len ) const;
//    description:   extract and return the substring of length len starting
//                   at index pos
//    precondition:  this string represents c0, c1, ..., c(n-1)
//                         0 <= pos <= pos + len - 1 < n.
//    postcondition: returns the string that represents
//                   c(pos), c(pos+1), ..., c(pos+len-1)
//
// const char * c_str( ) const;
//    description:   convert string into a '\0'-terminated string as
//                   used in C for use with functions
//                   that have '\0'-terminated C-string parameters.
//    postcondition: returns the equivalent '\0'-terminated string
//
// indexing
//
// char   operator [ ] ( int k ) const
//    precondition:  0 <= k < length()
//    postcondition: returns kth character
//    note:          because the return value is a char and not a reference
//                   to a char, this function is read-only
//
// char & operator [ ] ( int k )
//    precondition:  0 <= k < length()
//    postcondition: returns reference to the kth character
//    note:          because the return value is a reference to a char and
//                   not merely a char, this function can be used to change
//                   values within the string (i.e. it can be used as an
//                   l-value).  If this reference is used to write a '\0'
//                   subsequent results are undefined since other functions
//                   expect the null-terminator to occur only after myLength
//                   characters.
//
// modifiers
//
// const string & operator += ( const string & str )
//    postcondition: concatenates a copy of str onto this string
//
// const string & operator += ( char ch )
//    postcondition: concatenates a copy of ch onto this string
//
//
// non-member functions
//
// ostream & operator << ( ostream & os, const string & str )
//    postcondition: str is written to output stream os
//
// istream & operator >> ( istream & is, string & str )
//    precondition:  input stream is open for reading
//    postcondition: the next string from input stream is has been read
//                   and stored in str.  Reading terminates when a whitespace
//                   character is encountered ( '\n', '\t', ' ' ).  The first
//                   whitespace character has been consumed but not stored.
//
// istream & getline( istream & is, string & str )
//    description:   reads a line from input stream is into the string str
//    precondition:  input stream is open for reading
//    postcondition: chars from input stream is up to '\n' have been read and
//                   stored in str; the '\n' has been consumed but not stored
//
// string operator + ( const string & lhs, const string & rhs )
//    postcondition: returns concatenation of lhs with rhs
//
// string operator + ( char ch, const string & str )
//    postcondition: returns concatenation of ch with str
//
// string operator + ( const string & str, char ch )
//    postcondition: returns concatenation of str with ch
//
//***************************************************************

#endif
