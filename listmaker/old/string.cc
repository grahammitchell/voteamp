#include <iostream.h>
#include <stdlib.h>		// for exit()
#include "string.h"

// *******************************************************************
//  APCS string class
//
//  string class consistent with a subset of the standard C++ string class
//  as defined in the draft ANSI standard
// *******************************************************************

const int npos = -1;  // used to indicate not a position in the string

// *******************************************************************
// *******************                        ************************
// *******************    MEMBER FUNCTIONS    ************************
// *******************                        ************************
// *******************************************************************

// *******************************************************************
//  constructors/destructor
// *******************************************************************

// string( )
//    description:   default constructor, constructs empty string from nothing
//    postcondition: string is empty
string::string( )
{
#ifdef DEBUG
	cerr << "string::string( )" << endl;
#endif
	myLength = 0;
	myCapacity = 10;
	myCstring = new char[myCapacity];
}

// string( char c )
//    description:   constructs a string object from a literal char such as 'a'
//    precondition:  c is a character other than '\0'
//    postcondition: a string of length 1 containing the character c has
//                   been constructed
string::string( char c )
{
#ifdef DEBUG
	cerr << "string::string( char c )" << endl;
#endif
	myLength = 1;
	myCapacity = 10;
	myCstring = new char[myCapacity];
	myCstring[0] = c;
	myCstring[1] = '\0';
}

// string( const char * s )
//    description:   constructs a string object from a literal C-string
//                   such as "abcd"
//    precondition:  s is '\0'-terminated string as used in C
//    postcondition: copy of s has been constructed
string::string( const char * s )
{
#ifdef DEBUG
	cerr << "string::string( const char * s )" << endl;
#endif
	// get length of s
	myLength = 0;
	while ( s[myLength] )
		++myLength;

	myCapacity = myLength+5;
	myCstring = new char[myCapacity];
	for ( int i=0; i<=myLength; ++i )     // <= so we get the null-terminator
		myCstring[i] = s[i];
}

// string( const string & str )
//    description:   copy constructor
//    postcondition: deep copy of str has been constructed
string::string( const string & str )
{
#ifdef DEBUG
	cerr << "string::string( const string & str )" << endl;
#endif
	myLength = str.myLength;
	myCapacity = str.myCapacity;
	myCstring = new char[myCapacity];
	for ( int i=0; i<=myLength; ++i )     // <= so we get the null-terminator
		myCstring[i] = str.myCstring[i];
}

// ~string( );
//    description:   destructor
//    postcondition: string is destroyed
string::~string( )
{
#ifdef DEBUG
	cerr << "string::~string( )" << endl;
#endif
	delete [] myCstring;
}


// *******************************************************************
//  assignment
// *******************************************************************

// string & operator = ( const string & str )
//    postcondition: normal assignment via copying has been performed
const string & string::operator = ( const string & str )
{
#ifdef DEBUG
	cerr << "const string & string::operator = ( const string & str )" << endl;
#endif
	if ( &str != this )
	{
		// destroy old string
		delete [] myCstring;
		// copy existing string
		myLength = str.myLength;
		myCapacity = str.myCapacity;
		myCstring = new char[myCapacity];
		for ( int i=0; i<=myLength; ++i )
			myCstring[i] = str.myCstring[i];
	}
	return *this;
}

// string & operator = ( const char * s )
//    description:   assignment from literal string such as "abcd"
//    precondition:  s is '\0'-terminated string as used in C
//    postcondition: assignment via copying of s has been performed
const string & string::operator = ( const char * s )
{
#ifdef DEBUG
	cerr << "const string & string::operator = ( const char * s )" << endl;
#endif
	if ( s != myCstring )
	{
		// destroy old string
		delete [] myCstring;
		// get length of s
		myLength = 0;
		while ( s[myLength] )
			++myLength;

		myCapacity = myLength+5;
		myCstring = new char[myCapacity];
		for ( int i=0; i<=myLength; ++i )
			myCstring[i] = s[i];
	}
	return *this;
}

// string & operator = ( char ch )
//    description:   assignment from character as though single char string
//    postcondition: assignment of one-character string has been performed
const string & string::operator = ( char ch )
{
#ifdef DEBUG
	cerr << "const string & string::operator = ( char ch )" << endl;
#endif
	myLength = 1;
	if ( myCapacity < 2 )
	{
		delete [] myCstring;
		myCapacity = 10;
		myCstring = new char[myCapacity];
	}
	myCstring[0] = ch;
	myCstring[1] = '\0';
	return *this;
}


// *******************************************************************
//  accessors
// *******************************************************************

// int length( ) const;
//    postcondition: returns number of chars in string
int string::length( ) const
{
#ifdef DEBUG
	cerr << "int string::length( ) const" << endl;
#endif
	return myLength;
}

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
int string::find( const string & str ) const
{
#ifdef DEBUG
	cerr << "int string::find( const string & str ) const with str=\"" << str.myCstring << '"' << endl;
#endif
	// This would be more efficient if I rewrote this from ground, but my
	// brain hurts so I'm just going to call a lot of member functions
	for ( int i=0; i<myLength-str.myLength; ++i )
	{
		if ( myCstring[i] == str.myCstring[0] && substr(i,str.myLength) == str )
			return i;
	}
	return npos;
}

// int find( char ch ) const;
//    description:   finds the first occurrence of the character ch within this
//                   string and returns the index.  If ch does not occur in this
//                   string, then returns npos.
//    precondition:  this string represents c0, c1, ..., c(n-1)
//    postcondition: if ch == ck, and there is no j < k such that ch == cj
//                   then returns k;
//                   otherwise returns npos
int string::find( char ch ) const
{
#ifdef DEBUG
	cerr << "int string::find( char ch ) const" << endl;
#endif
	int i;
	for ( i=0; i<myLength; ++i )
		if ( myCstring[i] == ch )
			return i;
	return npos;
}

// string substr( int pos, int len ) const;
//    description:   extract and return the substring of length len starting
//                   at index pos
//    precondition:  this string represents c0, c1, ..., c(n-1)
//                         0 <= pos <= pos + len - 1 < n.
//    postcondition: returns the string that represents
//                   c(pos), c(pos+1), ..., c(pos+len-1)
string string::substr( int pos, int len ) const
{
#ifdef DEBUG
	cerr << "string string::substr( int pos, int len ) const" << endl;
#endif
	char* r = new char[len+1];    // start with a simple array of chars
	for ( int i=pos; i<pos+len; ++i )       // copy chars
		r[i-pos] = myCstring[i];
	r[len] = '\0';                // null-terminate
	string r_string(r);           // use constructor to make a string out of it
	return r_string;
}

// const char * c_str( ) const
//    description:   convert string into a '\0'-terminated string as
//                   used in C for use with functions
//                   that have '\0'-terminated C-string parameters.
//    postcondition: returns the equivalent '\0'-terminated string
const char * string::c_str( ) const
{
#ifdef DEBUG
	cerr << "const char * string::c_str( ) const" << endl;
#endif
	return myCstring;
}


// *******************************************************************
//  indexing
// *******************************************************************

// char   operator [ ] ( int k ) const
//    precondition:  0 <= k < length()
//    postcondition: returns kth character
//    note:          because the return value is a char and not a reference
//                   to a char, this function is read-only
char string::operator[ ] ( int k ) const
{
#ifdef DEBUG
	cerr << "char string::operator[ ] ( int k ) const" << endl;
#endif
	if ( k < 0 || k > myLength )
	{
		cerr << "index " << k << " out of range in string::operator[ ] ";
		cerr << "(length is " << myLength << ")\n";
		exit(1);
	}
	return myCstring[k];
}

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
char & string::operator[ ] ( int k )
{
#ifdef DEBUG
	cerr << "char & string::operator[ ] ( int k )" << endl;
#endif
	if ( k < 0 || k > myLength )
	{
		cerr << "index " << k << " out of range in string::operator[ ] ";
		cerr << "(length is " << myLength << ")\n";
		exit(1);
	}
	return myCstring[k];
}


// *******************************************************************
//  modifiers
// *******************************************************************

// const string & operator += ( const string & str )
//    postcondition: concatenates a copy of str onto this string
const string & string::operator += ( const string & str )
{
#ifdef DEBUG
	cerr << "const string & string::operator += ( const string & str )" << endl;
#endif
	// don't bother growing if you don't have to
	if ( myCapacity <= myLength+str.myLength )
	{
		// grow
		while ( myCapacity <= myLength+str.myLength )
			myCapacity += 5;
		char* temp = new char[myCapacity];   // get more space
		for ( int i=0; i<myLength; ++i )     // copy from old to new
			temp[i] = myCstring[i];
		delete [] myCstring;                 // free old memory
		myCstring = temp;                    // point at the new chunk
	}
	// now append the new string after the end
	for ( int i=0; i<=str.myLength; ++i )   // <= gets the '\0' too
		myCstring[myLength+i] = str.myCstring[i];
	myLength += str.myLength;               // fix my length

	// ... and I'm out.
	return *this;
}

// const string & operator += ( char ch )
//    postcondition: concatenates a copy of ch onto this string
const string & string::operator += ( char ch )
{
#ifdef DEBUG
	cerr << "const string & string::operator += ( char ch )" << endl;
#endif
	// don't bother growing if you don't have to
	if ( myCapacity <= myLength+1 )
	{
		// grow
		myCapacity += 5;
		char* temp = new char[myCapacity];   // get more space
		for ( int i=0; i<myLength; ++i )     // copy from old to new
			temp[i] = myCstring[i];
		delete [] myCstring;                 // free old memory
		myCstring = temp;                    // point at the new chunk
	}
	// just stick in the new char one past the end
	myCstring[myLength] = ch;
	++myLength;
	myCstring[myLength] = '\0';              // and null-terminate

	return *this;
}



// *******************************************************************
// **************                                   ******************
// **************    FREE (NON-MEMBER) FUNCTIONS    ******************
// **************                                   ******************
// *******************************************************************

// *******************************************************************
//  I/O functions
// *******************************************************************

// ostream & operator << ( ostream & os, const string & str )
//    postcondition: str is written to output stream os
ostream & operator << ( ostream & os, const string & str )
{
#ifdef DEBUG
	cerr << "ostream & operator << ( ostream & os, const string & str )" << endl;
#endif
	os << str.c_str();
	return os;
}

// istream & operator >> ( istream & is, string & str )
//    precondition:  input stream is open for reading
//    postcondition: the next string from input stream is has been read
//                   and stored in str.  Reading terminates when a whitespace
//                   character is encountered ( '\n', '\t', ' ' ).  The first
//                   whitespace character has not been consumed or stored.
istream & operator >> ( istream & is, string & str )
{
#ifdef DEBUG
	cerr << "istream & operator >> ( istream & is, string & str )" << endl;
#endif
	int i=0, n=8;
	char c;
	char* buf = new char[n];

	// munch leading whitespace
	is >> ws;

	c = is.peek();  // look ahead (so we don't accidentally consume whitespace)
	while ( c != ' ' && c != '\t' && c != '\n' )
	{
		is.get();  // we know it's not whitespace, so it's safe to consume it
		buf[i] = c; // put it in the array
		++i;        // and increment the index

		// now we may need to grow
		if ( i == n )
		{
			// grow
			char* temp = new char[n+8];  // make new, bigger buffer
			for ( int j=0; j<n; ++j )
				temp[j] = buf[j];        // copy existing values
			delete [] buf;               // recycle old, smaller buffer
			buf = temp;                  // now buf is the new buffer
			n += 8;                      // increment our maximum size
		}

		c = is.peek();  // set up for the next iteration
	}

	buf[i] = '\0';       // null-terminate
	str = buf;           // and then let operator = finish the job

	return is;
}

// istream & getline( istream & is, string & str )
//    description:   reads a line from input stream is into the string str
//    precondition:  input stream is open for reading
//    postcondition: chars from input stream is up to '\n' have been read and
//                   stored in str; the '\n' has been consumed but not stored
istream & getline( istream & is, string & str )
{
#ifdef DEBUG
	cerr << "istream & getline( istream & is, string & str )" << endl;
#endif
	int i=0, n=8;
	char c;
	char* buf = new char[n];

	// NOTE: getline doesn't munch leading whitespace (nor is it supposed to)

	do {
		is.get(c);           // get a character (including whitespace)
		buf[i] = c;          // put it in the array
		++i;                 // and increment the index

		// we may need to grow
		if ( i == n )
		{
			// grow
			char* temp = new char[n+8];  // make new, bigger buffer
			for ( int j=0; j<n; ++j )
				temp[j] = buf[j];        // copy existing values
			delete [] buf;               // recycle old, smaller buffer
			buf = temp;                  // now buf is the new buffer
			n += 8;                      // increment our maximum size
		}

	} while ( c != '\n' );    // stop when you get a newline

	// now, the last char in the buffer *is* a newline at this point
	// so we'll overwrite it with the null-terminator
	buf[i-1] = '\0';
	str = buf;               // and then let operator = finish the job

	return is;
}


// *******************************************************************
//  relational operators
// *******************************************************************

// bool operator == ( const string & lhs, const string & rhs )
//    postcondition: returns true if lhs is lexigraphically equal to rhs,
//                   false otherwise
bool operator == ( const string & lhs, const string & rhs )
{
#ifdef DEBUG
	cerr << "bool operator == ( const string & lhs, const string & rhs )" << endl;
#endif
	if ( lhs.length() != rhs.length() )
		return false;

	for ( int i=0; i<lhs.length() ; ++i )
		if ( lhs[i] != rhs[i] )
			return false;

	return true;
}

// bool operator != ( const string & lhs, const string & rhs )
//    postcondition: returns true if lhs is not lexigraphically equal to rhs,
//                   false otherwise
bool operator != ( const string & lhs, const string & rhs )
{
#ifdef DEBUG
	cerr << "bool operator != ( const string & lhs, const string & rhs )" << endl;
#endif
	if ( lhs.length() != rhs.length() )
		return true;

	for ( int i=0; i<lhs.length() ; ++i )
		if ( lhs[i] != rhs[i] )
			return true;

	return false;
}

// bool operator <  ( const string & lhs, const string & rhs )
//    postcondition: returns true if lhs lexigraphically less than rhs
//                   false otherwise
bool operator <  ( const string & lhs, const string & rhs )
{
#ifdef DEBUG
	cerr << "bool operator <  ( const string & lhs, const string & rhs )" << endl;
#endif
	int min = lhs.length() < rhs.length() ? lhs.length() : rhs.length() ;
	for ( int i=0; i<min ; ++i )
		if ( lhs[i] != rhs[i] )
			return ( lhs[i] < rhs[i] );

	return ( lhs.length() < rhs.length() );
}

// bool operator <= ( const string & lhs, const string & rhs )
//    postcondition: returns true if lhs lexigraphically less than or
//                   equal to rhs, false otherwise
bool operator <= ( const string & lhs, const string & rhs )
{
#ifdef DEBUG
	cerr << "bool operator <= ( const string & lhs, const string & rhs )" << endl;
#endif
	int min = lhs.length() < rhs.length() ? lhs.length() : rhs.length() ;
	for ( int i=0; i<min ; ++i )
		if ( lhs[i] != rhs[i] )
			return ( lhs[i] <= rhs[i] );

	return ( lhs.length() <= rhs.length() );
}

// bool operator >  ( const string & lhs, const string & rhs )
//    postcondition: returns true if lhs lexigraphically greater than rhs
//                   false otherwise
bool operator >  ( const string & lhs, const string & rhs )
{
#ifdef DEBUG
	cerr << "bool operator >  ( const string & lhs, const string & rhs )" << endl;
#endif
	int min = lhs.length() < rhs.length() ? lhs.length() : rhs.length() ;
	for ( int i=0; i<min ; ++i )
		if ( lhs[i] != rhs[i] )
			return ( lhs[i] > rhs[i] );

	return ( lhs.length() > rhs.length() );
}

// bool operator >= ( const string & lhs, const string & rhs )
//    postcondition: returns true if lhs lexigraphically greater than or
//                   equal to rhs, false otherwise
bool operator >= ( const string & lhs, const string & rhs )
{
#ifdef DEBUG
	cerr << "bool operator >= ( const string & lhs, const string & rhs )" << endl;
#endif
	int min = lhs.length() < rhs.length() ? lhs.length() : rhs.length() ;
	for ( int i=0; i<min ; ++i )
		if ( lhs[i] != rhs[i] )
			return ( lhs[i] >= rhs[i] );

	return ( lhs.length() >= rhs.length() );
}


// *******************************************************************
//  concatenation
// *******************************************************************

// string operator + ( const string & lhs, const string & rhs )
//    postcondition: returns concatenation of lhs with rhs
string operator + ( const string & lhs, const string & rhs )
{
#ifdef DEBUG
	cerr << "string operator + ( const string & lhs, const string & rhs )" << endl;
#endif
	// no use repeating myself;
	string result = lhs;           // copy constructor
	result += rhs;                 // concatenate
	return result;                 // go home
}

// string operator + ( char ch, const string & str )
//    postcondition: returns concatenation of ch with str
string operator + ( char ch, const string & str )
{
#ifdef DEBUG
	cerr << "string operator + ( char ch, const string & str )" << endl;
#endif
	string result = ch;
	result += str;
	return result;
}

// string operator + ( const string & str, char ch )
//    postcondition: returns concatenation of str with ch
string operator + ( const string & str, char ch )
{
#ifdef DEBUG
	cerr << "string operator + ( const string & str, char ch )" << endl;
#endif
	string result = str;
	result += ch;
	return result;
}

