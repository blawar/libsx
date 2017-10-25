/*
www.sourceforge.net/projects/sxml
Original code (2.0 and earlier )copyright (c) 2000-2006 Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/


#ifndef SXML_INCLUDED
#define SXML_INCLUDED

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4530 )
#pragma warning( disable : 4786 )
#endif

#define xml SxmlDocument

#include "../sx"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Help out windows:
#if defined( _DEBUG ) && !defined( DEBUG )
#define DEBUG
#endif

#define SXML_STRING	string
#define ATTRIBUTES std::vector< sxmlstyle* >

// Deprecated library function hell. Compilers want to use the
// new safe versions. This probably doesn't fully address the problem,
// but it gets closer. There are too many compilers for me to fully
// test. If you get compilation troubles, undefine SXML_SAFE
#define SXML_SAFE

#ifdef SXML_SAFE
	#if defined(_MSC_VER) && (_MSC_VER >= 1400 )
		// Microsoft visual studio, version 2005 and higher.
		#define SXML_SNPRINTF _snprintf_s
		#define SXML_SSCANF   sscanf_s
	#elif defined(_MSC_VER) && (_MSC_VER >= 1200 )
		// Microsoft visual studio, version 6 and higher.
		//#pragma message( "Using _sn* functions." )
		#define SXML_SNPRINTF _snprintf
		#define SXML_SSCANF   sscanf
	#elif defined(__GNUC__) && (__GNUC__ >= 3 )
		// GCC version 3 and higher.s
		//#warning( "Using sn* functions." )
		#define SXML_SNPRINTF snprintf
		#define SXML_SSCANF   sscanf
	#else
		#define SXML_SNPRINTF snprintf
		#define SXML_SSCANF   sscanf
	#endif
#endif	

class SxmlDocument;
class SxmlElement;
class SxmlComment;
class SxmlUnknown;
class SxmlAttribute;
class SxmlText;
class SxmlDeclaration;
class SxmlParsingData;
class cssNode;

const int SXML_MAJOR_VERSION = 2;
const int SXML_MINOR_VERSION = 6;
const int SXML_PATCH_VERSION = 1;

/*	Internal structure for tracking location of items 
	in the XML file.
*/
struct SxmlCursor
{
	SxmlCursor()		{ Clear(); }
	void Clear()		{ row = col = -1; }

	int row;	// 0 based.
	int col;	// 0 based.
};

typedef array<string, cssNode> CSSMAP;


class ELEMENTS : public std::vector<SxmlElement*>
{
public:
        ELEMENTS(){}
        ELEMENTS(SxmlElement* e){if(e) push_back(e);}
        bool operator!(){ return !empty(); }
        void copy(ELEMENTS* dest)
        {
                dest->clear();
                for(long i=0; i < size(); i++)
                {
                        dest->push_back(at(i));
                }
        }
private:
};

class sxmlstyle : public array<string, string>
{
public:
private:
};

class cssNode
{
public:
        cssNode();
        ~cssNode();
        cssNode* operator[](const char* key);
	cssNode* operator[](string key);
        cssNode* get() { return this; }
        void operator=(string v) { value = v; }
        string setValue(string v) { value = v; }
        string getValue() { return value; }
        void dump(string &buffer, long depth=0);
        CSSMAP* map() { return &cssMap; }
private:
        string value;
        CSSMAP cssMap;
};



/**
	Implements the interface to the "Visitor pattern" (see the Accept() method.)
	If you call the Accept() method, it requires being passed a SxmlVisitor
	class to handle callbacks. For nodes that contain other nodes (Document, Element)
	you will get called with a VisitEnter/VisitExit pair. Nodes that are always leaves
	are simply called with Visit().

	If you return 'true' from a Visit method, recursive parsing will continue. If you return
	false, <b>no children of this node or its sibilings</b> will be Visited.

	All flavors of Visit methods have a default implementation that returns 'true' (continue 
	visiting). You need to only override methods that are interesting to you.

	Generally Accept() is called on the SxmlDocument, although all nodes suppert Visiting.

	You should never change the document from a callback.

	@sa SxmlNode::Accept()
*/
class SxmlVisitor
{
public:
	virtual ~SxmlVisitor() {}

	/// Visit a document.
	virtual bool VisitEnter( const SxmlDocument& /*doc*/ )			{ return true; }
	/// Visit a document.
	virtual bool VisitExit( const SxmlDocument& /*doc*/ )			{ return true; }

	/// Visit an element.
	virtual bool VisitEnter( const SxmlElement& /*element*/, const SxmlAttribute* /*firstAttribute*/ )	{ return true; }
	/// Visit an element.
	virtual bool VisitExit( const SxmlElement& /*element*/ )		{ return true; }

	/// Visit a declaration
	virtual bool Visit( const SxmlDeclaration& /*declaration*/ )	{ return true; }
	/// Visit a text node
	virtual bool Visit( const SxmlText& /*text*/ )					{ return true; }
	/// Visit a comment node
	virtual bool Visit( const SxmlComment& /*comment*/ )			{ return true; }
	/// Visit an unknow node
	virtual bool Visit( const SxmlUnknown& /*unknown*/ )			{ return true; }
};

// Only used by Attribute::Query functions
enum 
{ 
	SXML_SUCCESS,
	SXML_NO_ATTRIBUTE,
	SXML_WRONG_TYPE
};


// Used by the parsing routines.
enum SxmlEncoding
{
	SXML_ENCODING_UNKNOWN,
	SXML_ENCODING_UTF8,
	SXML_ENCODING_LEGACY
};

const SxmlEncoding SXML_DEFAULT_ENCODING = SXML_ENCODING_UNKNOWN;

/** SxmlBase is a base class for every class in Sxml.
	It does little except to establish that Sxml classes
	can be printed and provide some utility functions.

	In XML, the document and elements can contain
	other elements and other types of nodes.

	@verbatim
	A Document can contain:	Element	(container or leaf)
							Comment (leaf)
							Unknown (leaf)
							Declaration( leaf )

	An Element can contain:	Element (container or leaf)
							Text	(leaf)
							Attributes (not on tree)
							Comment (leaf)
							Unknown (leaf)

	A Decleration contains: Attributes (not on tree)
	@endverbatim
*/
class SxmlBase
{
	friend class SxmlNode;
	friend class SxmlElement;
	friend class SxmlDocument;

public:
	SxmlBase()	:	userData(0)		{}
	virtual ~SxmlBase()			{}

	/**	All Sxml classes can print themselves to a filestream
		or the string class (SxmlString in non-STL mode, string
		in STL mode.) Either or both cfile and str can be null.
		
		This is a formatted print, and will insert 
		tabs and newlines.
		
		(For an unformatted stream, use the << operator.)
	*/
	virtual void Print( FILE* cfile, int depth ) const = 0;

	/**	The world does not agree on whether white space should be kept or
		not. In order to make everyone happy, these global, static functions
		are provided to set whether or not Sxml will condense all white space
		into a single space or not. The default is to condense. Note changing this
		value is not thread safe.
	*/
	static void SetCondenseWhiteSpace( bool condense )		{ condenseWhiteSpace = condense; }

	/// Return the current white space setting.
	static bool IsWhiteSpaceCondensed()						{ return condenseWhiteSpace; }

	/** Return the position, in the original source file, of this node or attribute.
		The row and column are 1-based. (That is the first row and first column is
		1,1). If the returns values are 0 or less, then the parser does not have
		a row and column value.

		Generally, the row and column value will be set when the SxmlDocument::Load(),
		SxmlDocument::LoadFile(), or any SxmlNode::Parse() is called. It will NOT be set
		when the DOM was created from operator>>.

		The values reflect the initial load. Once the DOM is modified programmatically
		(by adding or changing nodes and attributes) the new values will NOT update to
		reflect changes in the document.

		There is a minor performance cost to computing the row and column. Computation
		can be disabled if SxmlDocument::SetTabSize() is called with 0 as the value.

		@sa SxmlDocument::SetTabSize()
	*/
	int Row() const			{ return location.row + 1; }
	int Column() const		{ return location.col + 1; }	///< See Row()

	void  SetUserData( void* user )			{ userData = user; }	///< Set a pointer to arbitrary user data.
	void* GetUserData()						{ return userData; }	///< Get a pointer to arbitrary user data.
	const void* GetUserData() const 		{ return userData; }	///< Get a pointer to arbitrary user data.

	// Table that returs, for a given lead byte, the total number of bytes
	// in the UTF-8 sequence.
	static const int utf8ByteTable[256];

	virtual const char* Parse(	const char* p, 
								SxmlParsingData* data, 
								SxmlEncoding encoding /*= SXML_ENCODING_UNKNOWN */ ) = 0;

	/** Expands entities in a string. Note this should not contian the tag's '<', '>', etc, 
		or they will be transformed into entities!
	*/
	static void EncodeString( const SXML_STRING& str, SXML_STRING* out );

	enum
	{
		SXML_NO_ERROR = 0,
		SXML_ERROR,
		SXML_ERROR_OPENING_FILE,
		SXML_ERROR_PARSING_ELEMENT,
		SXML_ERROR_FAILED_TO_READ_ELEMENT_NAME,
		SXML_ERROR_READING_ELEMENT_VALUE,
		SXML_ERROR_READING_ATTRIBUTES,
		SXML_ERROR_PARSING_EMPTY,
		SXML_ERROR_READING_END_TAG,
		SXML_ERROR_PARSING_UNKNOWN,
		SXML_ERROR_PARSING_COMMENT,
		SXML_ERROR_PARSING_DECLARATION,
		SXML_ERROR_DOCUMENT_EMPTY,
		SXML_ERROR_EMBEDDED_NULL,
		SXML_ERROR_PARSING_CDATA,
		SXML_ERROR_DOCUMENT_TOP_ONLY,

		SXML_ERROR_STRING_COUNT
	};

protected:

	static const char* SkipWhiteSpace( const char*, SxmlEncoding encoding );

	inline static bool IsWhiteSpace( char c )		
	{ 
		return ( isspace( (unsigned char) c ) || c == '\n' || c == '\r' ); 
	}
	inline static bool IsWhiteSpace( int c )
	{
		if ( c < 256 )
			return IsWhiteSpace( (char) c );
		return false;	// Again, only truly correct for English/Latin...but usually works.
	}

	#ifdef SXML_USE_STL
	static bool	StreamWhiteSpace( std::istream * in, SXML_STRING * tag );
	static bool StreamTo( std::istream * in, int character, SXML_STRING * tag );
	#endif

	/*	Reads an XML name into the string provided. Returns
		a pointer just past the last character of the name,
		or 0 if the function has an error.
	*/
	static const char* ReadName( const char* p, SXML_STRING* name, SxmlEncoding encoding );

	/*	Reads text. Returns a pointer past the given end tag.
		Wickedly complex options, but it keeps the (sensitive) code in one place.
	*/
	static const char* ReadText(	const char* in,				// where to start
									SXML_STRING* text,			// the string read
									bool ignoreWhiteSpace,		// whether to keep the white space
									const char* endTag,			// what ends this text
									bool ignoreCase,			// whether to ignore case in the end tag
									SxmlEncoding encoding );	// the current encoding

	// If an entity has been found, transform it into a character.
	static const char* GetEntity( const char* in, char* value, int* length, SxmlEncoding encoding );

	// Get a character, while interpreting entities.
	// The length can be from 0 to 4 bytes.
	inline static const char* GetChar( const char* p, char* _value, int* length, SxmlEncoding encoding )
	{
		assert( p );
		if ( encoding == SXML_ENCODING_UTF8 )
		{
			*length = utf8ByteTable[ *((const unsigned char*)p) ];
			assert( *length >= 0 && *length < 5 );
		}
		else
		{
			*length = 1;
		}

		if ( *length == 1 )
		{
			if ( *p == '&' )
				return GetEntity( p, _value, length, encoding );
			*_value = *p;
			return p+1;
		}
		else if ( *length )
		{
			//strncpy( _value, p, *length );	// lots of compilers don't like this function (unsafe),
												// and the null terminator isn't needed
			for( int i=0; p[i] && i<*length; ++i ) {
				_value[i] = p[i];
			}
			return p + (*length);
		}
		else
		{
			// Not valid text.
			return 0;
		}
	}

	// Return true if the next characters in the stream are any of the endTag sequences.
	// Ignore case only works for english, and should only be relied on when comparing
	// to English words: StringEqual( p, "version", true ) is fine.
	static bool StringEqual(	const char* p,
								const char* endTag,
								bool ignoreCase,
								SxmlEncoding encoding );

	static const char* errorString[ SXML_ERROR_STRING_COUNT ];

	SxmlCursor location;

    /// Field containing a generic user pointer
	void*			userData;
	
	// None of these methods are reliable for any language except English.
	// Good for approximation, not great for accuracy.
	static int IsAlpha( unsigned char anyByte, SxmlEncoding encoding );
	static int IsAlphaNum( unsigned char anyByte, SxmlEncoding encoding );
	inline static int ToLower( int v, SxmlEncoding encoding )
	{
		if ( encoding == SXML_ENCODING_UTF8 )
		{
			if ( v < 128 ) return tolower( v );
			return v;
		}
		else
		{
			return tolower( v );
		}
	}
	static void ConvertUTF32ToUTF8( unsigned long input, char* output, int* length );

private:
	SxmlBase( const SxmlBase& );				// not implemented.
	void operator=( const SxmlBase& base );	// not allowed.

	struct Entity
	{
		const char*     str;
		unsigned int	strLength;
		char		    chr;
	};
	enum
	{
		NUM_ENTITY = 5,
		MAX_ENTITY_LENGTH = 6

	};
	static Entity entity[ NUM_ENTITY ];
	static bool condenseWhiteSpace;
};


/** The parent class for everything in the Document Object Model.
	(Except for attributes).
	Nodes have siblings, a parent, and children. A node can be
	in a document, or stand on its own. The type of a SxmlNode
	can be queried, and it can be cast to its more defined type.
*/
class SxmlNode : public SxmlBase
{
	friend class SxmlDocument;
	friend class SxmlElement;

public:
	#ifdef SXML_USE_STL	

	    /** An input stream operator, for every class. Tolerant of newlines and
		    formatting, but doesn't expect them.
	    */
	    friend std::istream& operator >> (std::istream& in, SxmlNode& base);

	    /** An output stream operator, for every class. Note that this outputs
		    without any newlines or formatting, as opposed to Print(), which
		    includes tabs and new lines.

		    The operator<< and operator>> are not completely symmetric. Writing
		    a node to a stream is very well defined. You'll get a nice stream
		    of output, without any extra whitespace or newlines.
		    
		    But reading is not as well defined. (As it always is.) If you create
		    a SxmlElement (for example) and read that from an input stream,
		    the text needs to define an element or junk will result. This is
		    true of all input streams, but it's worth keeping in mind.

		    A SxmlDocument will read nodes until it reads a root element, and
			all the children of that root element.
	    */	
	    friend std::ostream& operator<< (std::ostream& out, const SxmlNode& base);

		/// Appends the XML node or attribute to a string.
		friend string& operator<< (string& out, const SxmlNode& base );

	#endif

	/** The types of XML nodes supported by Sxml. (All the
			unsupported types are picked up by UNKNOWN.)
	*/
	enum NodeType
	{
		SXML_DOCUMENT,
		SXML_ELEMENT,
		SXML_COMMENT,
		SXML_UNKNOWN,
		SXML_TEXT,
		SXML_DECLARATION,
		SXML_TYPECOUNT
	};

	virtual ~SxmlNode();

	/** The meaning of 'value' changes for the specific type of
		SxmlNode.
		@verbatim
		Document:	filename of the xml file
		Element:	name of the element
		Comment:	the comment text
		Unknown:	the tag contents
		Text:		the text string
		@endverbatim

		The subclasses will wrap this function.
	*/
	const char *Value() const { return value.c_str (); }

	const string& ValueStr() const { return value; }

	const SXML_STRING& ValueTStr() const { return value; }

	/** Changes the value of the node. Defined as:
		@verbatim
		Document:	filename of the xml file
		Element:	name of the element
		Comment:	the comment text
		Unknown:	the tag contents
		Text:		the text string
		@endverbatim
	*/
	void SetValue(const char * _value) { value = _value;}

    #ifdef SXML_USE_STL
	/// STL string form.
	void SetValue( const string& _value )	{ value = _value; }
	#endif

	/// Delete all the children of this node. Does not affect 'this'.
	void Clear();

	/// One step up the DOM.
	SxmlNode* Parent()							{ return parent; }
	const SxmlNode* Parent() const				{ return parent; }

	const SxmlNode* FirstChild()	const		{ return firstChild; }	///< The first child of this node. Will be null if there are no children.
	SxmlNode* FirstChild()						{ return firstChild; }
	const SxmlNode* FirstChild( const char * value ) const;			///< The first child of this node with the matching 'value'. Will be null if none found.
	/// The first child of this node with the matching 'value'. Will be null if none found.
	SxmlNode* FirstChild( const char * _value ) {
		// Call through to the const version - safe since nothing is changed. Exiting syntax: cast this to a const (always safe)
		// call the method, cast the return back to non-const.
		return const_cast< SxmlNode* > ((const_cast< const SxmlNode* >(this))->FirstChild( _value ));
	}
	const SxmlNode* LastChild() const	{ return lastChild; }		/// The last child of this node. Will be null if there are no children.
	SxmlNode* LastChild()	{ return lastChild; }
	
	const SxmlNode* LastChild( const char * value ) const;			/// The last child of this node matching 'value'. Will be null if there are no children.
	SxmlNode* LastChild( const char * _value ) {
		return const_cast< SxmlNode* > ((const_cast< const SxmlNode* >(this))->LastChild( _value ));
	}

    #ifdef SXML_USE_STL
	const SxmlNode* FirstChild( const string& _value ) const	{	return FirstChild (_value.c_str ());	}	///< STL string form.
	SxmlNode* FirstChild( const string& _value )				{	return FirstChild (_value.c_str ());	}	///< STL string form.
	const SxmlNode* LastChild( const string& _value ) const	{	return LastChild (_value.c_str ());	}	///< STL string form.
	SxmlNode* LastChild( const string& _value )				{	return LastChild (_value.c_str ());	}	///< STL string form.
	#endif

	/** An alternate way to walk the children of a node.
		One way to iterate over nodes is:
		@verbatim
			for( child = parent->FirstChild(); child; child = child->NextSibling() )
		@endverbatim

		IterateChildren does the same thing with the syntax:
		@verbatim
			child = 0;
			while( child = parent->IterateChildren( child ) )
		@endverbatim

		IterateChildren takes the previous child as input and finds
		the next one. If the previous child is null, it returns the
		first. IterateChildren will return null when done.
	*/
	const SxmlNode* IterateChildren( const SxmlNode* previous ) const;
	SxmlNode* IterateChildren( const SxmlNode* previous ) {
		return const_cast< SxmlNode* >( (const_cast< const SxmlNode* >(this))->IterateChildren( previous ) );
	}

	/// This flavor of IterateChildren searches for children with a particular 'value'
	const SxmlNode* IterateChildren( const char * value, const SxmlNode* previous ) const;
	SxmlNode* IterateChildren( const char * _value, const SxmlNode* previous ) {
		return const_cast< SxmlNode* >( (const_cast< const SxmlNode* >(this))->IterateChildren( _value, previous ) );
	}

    #ifdef SXML_USE_STL
	const SxmlNode* IterateChildren( const string& _value, const SxmlNode* previous ) const	{	return IterateChildren (_value.c_str (), previous);	}	///< STL string form.
	SxmlNode* IterateChildren( const string& _value, const SxmlNode* previous ) {	return IterateChildren (_value.c_str (), previous);	}	///< STL string form.
	#endif

	/** Add a new node related to this. Adds a child past the LastChild.
		Returns a pointer to the new object or NULL if an error occured.
	*/
	SxmlNode* InsertEndChild( const SxmlNode& addThis );


	/** Add a new node related to this. Adds a child past the LastChild.

		NOTE: the node to be added is passed by pointer, and will be
		henceforth owned (and deleted) by Sxml. This method is efficient
		and avoids an extra copy, but should be used with care as it
		uses a different memory model than the other insert functions.

		@sa InsertEndChild
	*/
	SxmlNode* LinkEndChild( SxmlNode* addThis );

	/** Add a new node related to this. Adds a child before the specified child.
		Returns a pointer to the new object or NULL if an error occured.
	*/
	SxmlNode* InsertBeforeChild( SxmlNode* beforeThis, const SxmlNode& addThis );

	/** Add a new node related to this. Adds a child after the specified child.
		Returns a pointer to the new object or NULL if an error occured.
	*/
	SxmlNode* InsertAfterChild(  SxmlNode* afterThis, const SxmlNode& addThis );

	/** Replace a child of this node.
		Returns a pointer to the new object or NULL if an error occured.
	*/
	SxmlNode* ReplaceChild( SxmlNode* replaceThis, const SxmlNode& withThis );

	/// Delete a child of this node.
	bool RemoveChild( SxmlNode* removeThis );

	/// Navigate to a sibling node.
	const SxmlNode* PreviousSibling() const			{ return prev; }
	SxmlNode* PreviousSibling()						{ return prev; }

	/// Navigate to a sibling node.
	const SxmlNode* PreviousSibling( const char * ) const;
	SxmlNode* PreviousSibling( const char *_prev ) {
		return const_cast< SxmlNode* >( (const_cast< const SxmlNode* >(this))->PreviousSibling( _prev ) );
	}

    #ifdef SXML_USE_STL
	const SxmlNode* PreviousSibling( const string& _value ) const	{	return PreviousSibling (_value.c_str ());	}	///< STL string form.
	SxmlNode* PreviousSibling( const string& _value ) 			{	return PreviousSibling (_value.c_str ());	}	///< STL string form.
	const SxmlNode* NextSibling( const string& _value) const		{	return NextSibling (_value.c_str ());	}	///< STL string form.
	SxmlNode* NextSibling( const string& _value) 					{	return NextSibling (_value.c_str ());	}	///< STL string form.
	#endif

	/// Navigate to a sibling node.
	const SxmlNode* NextSibling() const				{ return next; }
	SxmlNode* NextSibling()							{ return next; }

	/// Navigate to a sibling node with the given 'value'.
	const SxmlNode* NextSibling( const char * ) const;
	SxmlNode* NextSibling( const char* _next ) {
		return const_cast< SxmlNode* >( (const_cast< const SxmlNode* >(this))->NextSibling( _next ) );
	}

	/** Convenience function to get through elements.
		Calls NextSibling and ToElement. Will skip all non-Element
		nodes. Returns 0 if there is not another element.
	*/
	const SxmlElement* NextSiblingElement() const;
	SxmlElement* NextSiblingElement() {
		return const_cast< SxmlElement* >( (const_cast< const SxmlNode* >(this))->NextSiblingElement() );
	}

	/** Convenience function to get through elements.
		Calls NextSibling and ToElement. Will skip all non-Element
		nodes. Returns 0 if there is not another element.
	*/
	const SxmlElement* NextSiblingElement( const char * ) const;
	SxmlElement* NextSiblingElement( const char *_next ) {
		return const_cast< SxmlElement* >( (const_cast< const SxmlNode* >(this))->NextSiblingElement( _next ) );
	}

    #ifdef SXML_USE_STL
	const SxmlElement* NextSiblingElement( const string& _value) const	{	return NextSiblingElement (_value.c_str ());	}	///< STL string form.
	SxmlElement* NextSiblingElement( const string& _value)				{	return NextSiblingElement (_value.c_str ());	}	///< STL string form.
	#endif

	/// Convenience function to get through elements.
	const SxmlElement* FirstChildElement()	const;
	SxmlElement* FirstChildElement() {
		return const_cast< SxmlElement* >( (const_cast< const SxmlNode* >(this))->FirstChildElement() );
	}

	/// Convenience function to get through elements.
	const SxmlElement* FirstChildElement( const char * _value ) const;
	SxmlElement* FirstChildElement( const char * _value ) {
		return const_cast< SxmlElement* >( (const_cast< const SxmlNode* >(this))->FirstChildElement( _value ) );
	}

    #ifdef SXML_USE_STL
	const SxmlElement* FirstChildElement( const string& _value ) const	{	return FirstChildElement (_value.c_str ());	}	///< STL string form.
	SxmlElement* FirstChildElement( const string& _value )				{	return FirstChildElement (_value.c_str ());	}	///< STL string form.
	#endif

	/** Query the type (as an enumerated value, above) of this node.
		The possible types are: DOCUMENT, ELEMENT, COMMENT,
								UNKNOWN, TEXT, and DECLARATION.
	*/
	int Type() const	{ return type; }

	/** Return a pointer to the Document this node lives in.
		Returns null if not in a document.
	*/
	const SxmlDocument* GetDocument() const;
	SxmlDocument* GetDocument() {
		return const_cast< SxmlDocument* >( (const_cast< const SxmlNode* >(this))->GetDocument() );
	}

	/// Returns true if this node has no children.
	bool NoChildren() const						{ return !firstChild; }

	virtual const SxmlDocument*    ToDocument()    const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual const SxmlElement*     ToElement()     const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual const SxmlComment*     ToComment()     const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual const SxmlUnknown*     ToUnknown()     const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual const SxmlText*        ToText()        const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual const SxmlDeclaration* ToDeclaration() const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.

	virtual SxmlDocument*          ToDocument()    { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual SxmlElement*           ToElement()	    { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual SxmlComment*           ToComment()     { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual SxmlUnknown*           ToUnknown()	    { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual SxmlText*	            ToText()        { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
	virtual SxmlDeclaration*       ToDeclaration() { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.

	/** Create an exact duplicate of this node and return it. The memory must be deleted
		by the caller. 
	*/
	virtual SxmlNode* Clone() const = 0;

	/** Accept a hierchical visit the nodes in the Sxml DOM. Every node in the 
		XML tree will be conditionally visited and the host will be called back
		via the SxmlVisitor interface.

		This is essentially a SAX interface for Sxml. (Note however it doesn't re-parse
		the XML for the callbacks, so the performance of Sxml is unchanged by using this
		interface versus any other.)

		The interface has been based on ideas from:

		- http://www.saxproject.org/
		- http://c2.com/cgi/wiki?HierarchicalVisitorPattern 

		Which are both good references for "visiting".

		An example of using Accept():
		@verbatim
		SxmlPrinter printer;
		sxmlDoc.Accept( &printer );
		const char* xmlcstr = printer.CStr();
		@endverbatim
	*/
	virtual bool Accept( SxmlVisitor* visitor ) const = 0;

protected:
	SxmlNode( NodeType _type );

	// Copy to the allocated object. Shared functionality between Clone, Copy constructor,
	// and the assignment operator.
	void CopyTo( SxmlNode* target ) const;

	#ifdef SXML_USE_STL
	    // The real work of the input operator.
	virtual void StreamIn( std::istream* in, SXML_STRING* tag ) = 0;
	#endif

	// Figure out what is at *p, and parse it. Returns null if it is not an xml node.
	SxmlNode* Identify( const char* start, SxmlEncoding encoding );

	SxmlNode*		parent;
	NodeType		type;

	SxmlNode*		firstChild;
	SxmlNode*		lastChild;

	SXML_STRING	value;

	SxmlNode*		prev;
	SxmlNode*		next;

private:
	SxmlNode( const SxmlNode& );				// not implemented.
	void operator=( const SxmlNode& base );	// not allowed.
};


/** An attribute is a name-value pair. Elements have an arbitrary
	number of attributes, each with a unique name.

	@note The attributes are not SxmlNodes, since they are not
		  part of the Sxml document object model. There are other
		  suggested ways to look at this problem.
*/
class SxmlAttribute : public SxmlBase
{
	friend class SxmlAttributeSet;

public:
	/// Construct an empty attribute.
	SxmlAttribute() : SxmlBase()
	{
		document = 0;
		prev = next = 0;
	}

	#ifdef SXML_USE_STL
	/// string constructor.
	SxmlAttribute( const string& _name, const string& _value )
	{
		name = _name;
		value = _value;
		document = 0;
		prev = next = 0;
	}
	#endif

	/// Construct an attribute with a name and value.
	SxmlAttribute( const char * _name, const char * _value )
	{
		name = _name;
		value = _value;
		document = 0;
		prev = next = 0;
	}

	const char*		Name()  const		{ return name.c_str(); }		///< Return the name of this attribute.
	const char*		Value() const		{ return value.c_str(); }		///< Return the value of this attribute.
	const string& ValueStr() const	{ return value; }				///< Return the value of this attribute.
	int				IntValue() const;									///< Return the value of this attribute, converted to an integer.
	double			DoubleValue() const;								///< Return the value of this attribute, converted to a double.

	// Get the sxml string representation
	const SXML_STRING& NameTStr() const { return name; }

	/** QueryIntValue examines the value string. It is an alternative to the
		IntValue() method with richer error checking.
		If the value is an integer, it is stored in 'value' and 
		the call returns SXML_SUCCESS. If it is not
		an integer, it returns SXML_WRONG_TYPE.

		A specialized but useful call. Note that for success it returns 0,
		which is the opposite of almost all other Sxml calls.
	*/
	int QueryIntValue( int* _value ) const;
	/// QueryDoubleValue examines the value string. See QueryIntValue().
	int QueryDoubleValue( double* _value ) const;

	void SetName( const char* _name )	{ name = _name; }				///< Set the name of this attribute.
	void SetValue( const char* _value )	{ value = _value; }				///< Set the value.

	void SetIntValue( int _value );										///< Set the value from an integer.
	void SetDoubleValue( double _value );								///< Set the value from a double.

    #ifdef SXML_USE_STL
	/// STL string form.
	void SetName( const string& _name )	{ name = _name; }	
	/// STL string form.	
	void SetValue( const string& _value )	{ value = _value; }
	#endif

	/// Get the next sibling attribute in the DOM. Returns null at end.
	const SxmlAttribute* Next() const;
	SxmlAttribute* Next() {
		return const_cast< SxmlAttribute* >( (const_cast< const SxmlAttribute* >(this))->Next() ); 
	}

	/// Get the previous sibling attribute in the DOM. Returns null at beginning.
	const SxmlAttribute* Previous() const;
	SxmlAttribute* Previous() {
		return const_cast< SxmlAttribute* >( (const_cast< const SxmlAttribute* >(this))->Previous() ); 
	}

	bool operator==( const SxmlAttribute& rhs ) const { return rhs.name == name; }
	bool operator<( const SxmlAttribute& rhs )	 const { return name < rhs.name; }
	bool operator>( const SxmlAttribute& rhs )  const { return name > rhs.name; }

	/*	Attribute parsing starts: first letter of the name
						 returns: the next char after the value end quote
	*/
	virtual const char* Parse( const char* p, SxmlParsingData* data, SxmlEncoding encoding );

	// Prints this Attribute to a FILE stream.
	virtual void Print( FILE* cfile, int depth ) const {
		Print( cfile, depth, 0 );
	}
	void Print( FILE* cfile, int depth, SXML_STRING* str ) const;

	// [internal use]
	// Set the document pointer so the attribute can report errors.
	void SetDocument( SxmlDocument* doc )	{ document = doc; }

private:
	SxmlAttribute( const SxmlAttribute& );				// not implemented.
	void operator=( const SxmlAttribute& base );	// not allowed.

	SxmlDocument*	document;	// A pointer back to a document, for error reporting.
	SXML_STRING name;
	SXML_STRING value;
	SxmlAttribute*	prev;
	SxmlAttribute*	next;
};


/*	A class used to manage a group of attributes.
	It is only used internally, both by the ELEMENT and the DECLARATION.
	
	The set can be changed transparent to the Element and Declaration
	classes that use it, but NOT transparent to the Attribute
	which has to implement a next() and previous() method. Which makes
	it a bit problematic and prevents the use of STL.

	This version is implemented with circular lists because:
		- I like circular lists
		- it demonstrates some independence from the (typical) doubly linked list.
*/
class SxmlAttributeSet
{
public:
	SxmlAttributeSet();
	~SxmlAttributeSet();

	void Add( SxmlAttribute* attribute );
	void Remove( SxmlAttribute* attribute );

	const SxmlAttribute* First()	const	{ return ( sentinel.next == &sentinel ) ? 0 : sentinel.next; }
	SxmlAttribute* First()					{ return ( sentinel.next == &sentinel ) ? 0 : sentinel.next; }
	const SxmlAttribute* Last() const		{ return ( sentinel.prev == &sentinel ) ? 0 : sentinel.prev; }
	SxmlAttribute* Last()					{ return ( sentinel.prev == &sentinel ) ? 0 : sentinel.prev; }

	SxmlAttribute*	Find( const char* _name ) const;
	SxmlAttribute* FindOrCreate( const char* _name );

#	ifdef SXML_USE_STL
	SxmlAttribute*	Find( const string& _name ) const;
	SxmlAttribute* FindOrCreate( const string& _name );
#	endif


private:
	//*ME:	Because of hidden/disabled copy-construktor in SxmlAttribute (sentinel-element),
	//*ME:	this class must be also use a hidden/disabled copy-constructor !!!
	SxmlAttributeSet( const SxmlAttributeSet& );	// not allowed
	void operator=( const SxmlAttributeSet& );	// not allowed (as SxmlAttribute)

	SxmlAttribute sentinel;
};


/** The element is a container class. It has a value, the element name,
	and can contain other elements, text, comments, and unknowns.
	Elements also contain an arbitrary number of attributes.
*/
class SxmlElement : public SxmlNode
{
public:
	/// Construct an element.
	SxmlElement (const char * in_value);

	#ifdef SXML_USE_STL
	/// string constructor.
	SxmlElement( const string& _value );
	#endif

	SxmlElement( const SxmlElement& );

	void operator=( const SxmlElement& base );

	virtual ~SxmlElement();

	/** Given an attribute name, Attribute() returns the value
		for the attribute of that name, or null if none exists.
	*/
	const char* Attribute( const char* name ) const;

	/** Given an attribute name, Attribute() returns the value
		for the attribute of that name, or null if none exists.
		If the attribute exists and can be converted to an integer,
		the integer value will be put in the return 'i', if 'i'
		is non-null.
	*/
	const char* Attribute( const char* name, int* i ) const;

	/** Given an attribute name, Attribute() returns the value
		for the attribute of that name, or null if none exists.
		If the attribute exists and can be converted to an double,
		the double value will be put in the return 'd', if 'd'
		is non-null.
	*/
	const char* Attribute( const char* name, double* d ) const;

	/** QueryIntAttribute examines the attribute - it is an alternative to the
		Attribute() method with richer error checking.
		If the attribute is an integer, it is stored in 'value' and 
		the call returns SXML_SUCCESS. If it is not
		an integer, it returns SXML_WRONG_TYPE. If the attribute
		does not exist, then SXML_NO_ATTRIBUTE is returned.
	*/	
	int QueryIntAttribute( const char* name, int* _value ) const;
	/// QueryDoubleAttribute examines the attribute - see QueryIntAttribute().
	int QueryDoubleAttribute( const char* name, double* _value ) const;
	/// QueryFloatAttribute examines the attribute - see QueryIntAttribute().
	int QueryFloatAttribute( const char* name, float* _value ) const {
		double d;
		int result = QueryDoubleAttribute( name, &d );
		if ( result == SXML_SUCCESS ) {
			*_value = (float)d;
		}
		return result;
	}

	/** Sets an attribute of name to a given value. The attribute
		will be created if it does not exist, or changed if it does.
	*/
	void SetAttribute( const char* name, const char * _value );

	/** Sets an attribute of name to a given value. The attribute
		will be created if it does not exist, or changed if it does.
	*/
	void SetAttribute( const char * name, int value );

	/** Sets an attribute of name to a given value. The attribute
		will be created if it does not exist, or changed if it does.
	*/
	void SetDoubleAttribute( const char * name, double value );

	/** Deletes an attribute with the given name.
	*/
	void RemoveAttribute( const char * name );

	const SxmlAttribute* FirstAttribute() const	{ return attributeSet.First(); }		///< Access the first attribute in this element.
	SxmlAttribute* FirstAttribute() 				{ return attributeSet.First(); }
	const SxmlAttribute* LastAttribute()	const 	{ return attributeSet.Last(); }		///< Access the last attribute in this element.
	SxmlAttribute* LastAttribute()					{ return attributeSet.Last(); }

	/** Convenience function for easy access to the text inside an element. Although easy
		and concise, GetText() is limited compared to getting the SxmlText child
		and accessing it directly.
	
		If the first child of 'this' is a SxmlText, the GetText()
		returns the character string of the Text node, else null is returned.

		This is a convenient method for getting the text of simple contained text:
		@verbatim
		<foo>This is text</foo>
		const char* str = fooElement->GetText();
		@endverbatim

		'str' will be a pointer to "This is text". 
		
		Note that this function can be misleading. If the element foo was created from
		this XML:
		@verbatim
		<foo><b>This is text</b></foo> 
		@endverbatim

		then the value of str would be null. The first child node isn't a text node, it is
		another element. From this XML:
		@verbatim
		<foo>This is <b>text</b></foo> 
		@endverbatim
		GetText() will return "This is ".

		WARNING: GetText() accesses a child node - don't become confused with the 
				 similarly named SxmlHandle::Text() and SxmlNode::ToText() which are 
				 safe type casts on the referenced node.
	*/
	const char* GetText() const;

	/// Creates a new Element and returns it - the returned element is a copy.
	virtual SxmlNode* Clone() const;
	// Print the Element to a FILE stream.
	virtual void Print( FILE* cfile, int depth ) const;

	/*	Attribtue parsing starts: next char past '<'
						 returns: next char past '>'
	*/
	virtual const char* Parse( const char* p, SxmlParsingData* data, SxmlEncoding encoding );

	virtual const SxmlElement*     ToElement()     const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
	virtual SxmlElement*           ToElement()	          { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

	/** Walk the XML tree visiting this node and all of its children. 
	*/
	virtual bool Accept( SxmlVisitor* visitor ) const;

	void render(string &buffer);
        sxmlstyle style;

protected:

	void CopyTo( SxmlElement* target ) const;
	void ClearThis();	// like clear, but initializes 'this' object as well

	// Used to be public [internal use]
	#ifdef SXML_USE_STL
	virtual void StreamIn( std::istream * in, SXML_STRING * tag );
	#endif
	/*	[internal use]
		Reads the "value" of the element -- another element, or text.
		This should terminate with the current end tag.
	*/
	const char* ReadValue( const char* in, SxmlParsingData* prevData, SxmlEncoding encoding );

private:
	SxmlAttributeSet attributeSet;
};


/**	An XML comment.
*/
class SxmlComment : public SxmlNode
{
public:
	/// Constructs an empty comment.
	SxmlComment() : SxmlNode( SxmlNode::SXML_COMMENT ) {}
	/// Construct a comment from text.
	SxmlComment( const char* _value ) : SxmlNode( SxmlNode::SXML_COMMENT ) {
		SetValue( _value );
	}
	SxmlComment( const SxmlComment& );
	void operator=( const SxmlComment& base );

	virtual ~SxmlComment()	{}

	/// Returns a copy of this Comment.
	virtual SxmlNode* Clone() const;
	// Write this Comment to a FILE stream.
	virtual void Print( FILE* cfile, int depth ) const;

	/*	Attribtue parsing starts: at the ! of the !--
						 returns: next char past '>'
	*/
	virtual const char* Parse( const char* p, SxmlParsingData* data, SxmlEncoding encoding );

	virtual const SxmlComment*  ToComment() const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
	virtual SxmlComment*  ToComment() { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

	/** Walk the XML tree visiting this node and all of its children. 
	*/
	virtual bool Accept( SxmlVisitor* visitor ) const;

protected:
	void CopyTo( SxmlComment* target ) const;

	// used to be public
	#ifdef SXML_USE_STL
	virtual void StreamIn( std::istream * in, SXML_STRING * tag );
	#endif
//	virtual void StreamOut( SXML_OSTREAM * out ) const;

private:

};


/** XML text. A text node can have 2 ways to output the next. "normal" output 
	and CDATA. It will default to the mode it was parsed from the XML file and
	you generally want to leave it alone, but you can change the output mode with 
	SetCDATA() and query it with CDATA().
*/
class SxmlText : public SxmlNode
{
	friend class SxmlElement;
public:
	/** Constructor for text element. By default, it is treated as 
		normal, encoded text. If you want it be output as a CDATA text
		element, set the parameter _cdata to 'true'
	*/
	SxmlText (const char * initValue ) : SxmlNode (SxmlNode::SXML_TEXT)
	{
		SetValue( initValue );
		cdata = false;
	}
	virtual ~SxmlText() {}

	#ifdef SXML_USE_STL
	/// Constructor.
	SxmlText( const string& initValue ) : SxmlNode (SxmlNode::SXML_TEXT)
	{
		SetValue( initValue );
		cdata = false;
	}
	#endif

	SxmlText( const SxmlText& copy ) : SxmlNode( SxmlNode::SXML_TEXT )	{ copy.CopyTo( this ); }
	void operator=( const SxmlText& base )							 	{ base.CopyTo( this ); }

	// Write this text object to a FILE stream.
	virtual void Print( FILE* cfile, int depth ) const;

	/// Queries whether this represents text using a CDATA section.
	bool CDATA() const				{ return cdata; }
	/// Turns on or off a CDATA representation of text.
	void SetCDATA( bool _cdata )	{ cdata = _cdata; }

	virtual const char* Parse( const char* p, SxmlParsingData* data, SxmlEncoding encoding );

	virtual const SxmlText* ToText() const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
	virtual SxmlText*       ToText()       { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

	/** Walk the XML tree visiting this node and all of its children. 
	*/
	virtual bool Accept( SxmlVisitor* content ) const;

protected :
	///  [internal use] Creates a new Element and returns it.
	virtual SxmlNode* Clone() const;
	void CopyTo( SxmlText* target ) const;

	bool Blank() const;	// returns true if all white space and new lines
	// [internal use]
	#ifdef SXML_USE_STL
	virtual void StreamIn( std::istream * in, SXML_STRING * tag );
	#endif

private:
	bool cdata;			// true if this should be input and output as a CDATA style text element
};


/** In correct XML the declaration is the first entry in the file.
	@verbatim
		<?xml version="1.0" standalone="yes"?>
	@endverbatim

	Sxml will happily read or write files without a declaration,
	however. There are 3 possible attributes to the declaration:
	version, encoding, and standalone.

	Note: In this version of the code, the attributes are
	handled as special cases, not generic attributes, simply
	because there can only be at most 3 and they are always the same.
*/
class SxmlDeclaration : public SxmlNode
{
public:
	/// Construct an empty declaration.
	SxmlDeclaration()   : SxmlNode( SxmlNode::SXML_DECLARATION ) {}

#ifdef SXML_USE_STL
	/// Constructor.
	SxmlDeclaration(	const string& _version,
						const string& _encoding,
						const string& _standalone );
#endif

	/// Construct.
	SxmlDeclaration(	const char* _version,
						const char* _encoding,
						const char* _standalone );

	SxmlDeclaration( const SxmlDeclaration& copy );
	void operator=( const SxmlDeclaration& copy );

	virtual ~SxmlDeclaration()	{}

	/// Version. Will return an empty string if none was found.
	const char *Version() const			{ return version.c_str (); }
	/// Encoding. Will return an empty string if none was found.
	const char *Encoding() const		{ return encoding.c_str (); }
	/// Is this a standalone document?
	const char *Standalone() const		{ return standalone.c_str (); }

	/// Creates a copy of this Declaration and returns it.
	virtual SxmlNode* Clone() const;
	// Print this declaration to a FILE stream.
	virtual void Print( FILE* cfile, int depth, SXML_STRING* str ) const;
	virtual void Print( FILE* cfile, int depth ) const {
		Print( cfile, depth, 0 );
	}

	virtual const char* Parse( const char* p, SxmlParsingData* data, SxmlEncoding encoding );

	virtual const SxmlDeclaration* ToDeclaration() const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
	virtual SxmlDeclaration*       ToDeclaration()       { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

	/** Walk the XML tree visiting this node and all of its children. 
	*/
	virtual bool Accept( SxmlVisitor* visitor ) const;

protected:
	void CopyTo( SxmlDeclaration* target ) const;
	// used to be public
	#ifdef SXML_USE_STL
	virtual void StreamIn( std::istream * in, SXML_STRING * tag );
	#endif

private:

	SXML_STRING version;
	SXML_STRING encoding;
	SXML_STRING standalone;
};


/** Any tag that Sxml doesn't recognize is saved as an
	unknown. It is a tag of text, but should not be modified.
	It will be written back to the XML, unchanged, when the file
	is saved.

	DTD tags get thrown into SxmlUnknowns.
*/
class SxmlUnknown : public SxmlNode
{
public:
	SxmlUnknown() : SxmlNode( SxmlNode::SXML_UNKNOWN )	{}
	virtual ~SxmlUnknown() {}

	SxmlUnknown( const SxmlUnknown& copy ) : SxmlNode( SxmlNode::SXML_UNKNOWN )		{ copy.CopyTo( this ); }
	void operator=( const SxmlUnknown& copy )										{ copy.CopyTo( this ); }

	/// Creates a copy of this Unknown and returns it.
	virtual SxmlNode* Clone() const;
	// Print this Unknown to a FILE stream.
	virtual void Print( FILE* cfile, int depth ) const;

	virtual const char* Parse( const char* p, SxmlParsingData* data, SxmlEncoding encoding );

	virtual const SxmlUnknown*     ToUnknown()     const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
	virtual SxmlUnknown*           ToUnknown()	    { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

	/** Walk the XML tree visiting this node and all of its children. 
	*/
	virtual bool Accept( SxmlVisitor* content ) const;

protected:
	void CopyTo( SxmlUnknown* target ) const;

	#ifdef SXML_USE_STL
	virtual void StreamIn( std::istream * in, SXML_STRING * tag );
	#endif

private:

};


/** Always the top level node. A document binds together all the
	XML pieces. It can be saved, loaded, and printed to the screen.
	The 'value' of a document node is the xml file name.
*/
class SxmlDocument : public SxmlNode
{
public:
	/// Create an empty document, that has no name.
	SxmlDocument();
	/// Create a document with a name. The name of the document is also the filename of the xml.
	SxmlDocument( const char * documentName );

	#ifdef SXML_USE_STL
	/// Constructor.
	SxmlDocument( const string& documentName );
	#endif

	SxmlDocument( const SxmlDocument& copy );
	void operator=( const SxmlDocument& copy );

	virtual ~SxmlDocument() {}

	/** Load a file using the current document value.
		Returns true if successful. Will delete any existing
		document data before loading.
	*/
	bool LoadFile( SxmlEncoding encoding = SXML_DEFAULT_ENCODING );
	/// Save a file using the current document value. Returns true if successful.
	bool SaveFile() const;
	/// Load a file using the given filename. Returns true if successful.
	bool LoadFile( const char * filename, SxmlEncoding encoding = SXML_DEFAULT_ENCODING );
	/// Save a file using the given filename. Returns true if successful.
	bool SaveFile( const char * filename ) const;
	/** Load a file using the given FILE*. Returns true if successful. Note that this method
		doesn't stream - the entire object pointed at by the FILE*
		will be interpreted as an XML file. Sxml doesn't stream in XML from the current
		file location. Streaming may be added in the future.
	*/
	bool LoadFile( FILE*, SxmlEncoding encoding = SXML_DEFAULT_ENCODING );
	/// Save a file using the given FILE*. Returns true if successful.
	bool SaveFile( FILE* ) const;

	#ifdef SXML_USE_STL
	bool LoadFile( const string& filename, SxmlEncoding encoding = SXML_DEFAULT_ENCODING )			///< STL string version.
	{
		return LoadFile( filename.c_str(), encoding );
	}
	bool SaveFile( const string& filename ) const		///< STL string version.
	{
		return SaveFile( filename.c_str() );
	}
	#endif

	operator SxmlElement*();

	/** Parse the given null terminated block of xml data. Passing in an encoding to this
		method (either SXML_ENCODING_LEGACY or SXML_ENCODING_UTF8 will force Sxml
		to use that encoding, regardless of what Sxml might otherwise try to detect.
	*/
	virtual const char* Parse( const char* p, SxmlParsingData* data = 0, SxmlEncoding encoding = SXML_DEFAULT_ENCODING );

	/** Get the root element -- the only top level element -- of the document.
		In well formed XML, there should only be one. Sxml is tolerant of
		multiple elements at the document level.
	*/
	const SxmlElement* RootElement() const		{ return FirstChildElement(); }
	SxmlElement* RootElement()					{ return FirstChildElement(); }

	/** If an error occurs, Error will be set to true. Also,
		- The ErrorId() will contain the integer identifier of the error (not generally useful)
		- The ErrorDesc() method will return the name of the error. (very useful)
		- The ErrorRow() and ErrorCol() will return the location of the error (if known)
	*/	
	bool Error() const						{ return error; }

	/// Contains a textual (english) description of the error if one occurs.
	const char * ErrorDesc() const	{ return errorDesc.c_str (); }

	/** Generally, you probably want the error string ( ErrorDesc() ). But if you
		prefer the ErrorId, this function will fetch it.
	*/
	int ErrorId()	const				{ return errorId; }

	/** Returns the location (if known) of the error. The first column is column 1, 
		and the first row is row 1. A value of 0 means the row and column wasn't applicable
		(memory errors, for example, have no row/column) or the parser lost the error. (An
		error in the error reporting, in that case.)

		@sa SetTabSize, Row, Column
	*/
	int ErrorRow() const	{ return errorLocation.row+1; }
	int ErrorCol() const	{ return errorLocation.col+1; }	///< The column where the error occured. See ErrorRow()

	/** SetTabSize() allows the error reporting functions (ErrorRow() and ErrorCol())
		to report the correct values for row and column. It does not change the output
		or input in any way.
		
		By calling this method, with a tab size
		greater than 0, the row and column of each node and attribute is stored
		when the file is loaded. Very useful for tracking the DOM back in to
		the source file.

		The tab size is required for calculating the location of nodes. If not
		set, the default of 4 is used. The tabsize is set per document. Setting
		the tabsize to 0 disables row/column tracking.

		Note that row and column tracking is not supported when using operator>>.

		The tab size needs to be enabled before the parse or load. Correct usage:
		@verbatim
		SxmlDocument doc;
		doc.SetTabSize( 8 );
		doc.Load( "myfile.xml" );
		@endverbatim

		@sa Row, Column
	*/
	void SetTabSize( int _tabsize )		{ tabsize = _tabsize; }

	int TabSize() const	{ return tabsize; }

	/** If you have handled the error, it can be reset with this call. The error
		state is automatically cleared if you Parse a new XML block.
	*/
	void ClearError()						{	error = false; 
												errorId = 0; 
												errorDesc = ""; 
												errorLocation.row = errorLocation.col = 0; 
												//errorLocation.last = 0; 
											}

	/** Write the document to standard out using formatted printing ("pretty print"). */
	void Print() const						{ Print( stdout, 0 ); }

	/* Write the document to a string using formatted printing ("pretty print"). This
		will allocate a character array (new char[]) and return it as a pointer. The
		calling code pust call delete[] on the return char* to avoid a memory leak.
	*/
	//char* PrintToMemory() const; 

	/// Print this Document to a FILE stream.
	virtual void Print( FILE* cfile, int depth = 0 ) const;
	// [internal use]
	void SetError( int err, const char* errorLocation, SxmlParsingData* prevData, SxmlEncoding encoding );

	virtual const SxmlDocument*    ToDocument()    const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
	virtual SxmlDocument*          ToDocument()          { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

	/** Walk the XML tree visiting this node and all of its children. 
	*/
	virtual bool Accept( SxmlVisitor* content ) const;

	void addStyleSheet(string &buffer);
        void render(string &buffer);
        void applyClasses();
        void applySelector(SxmlElement* element, cssNode *cssRoot=NULL);
        void applySelector(ELEMENTS elements, cssNode *cssRoot=NULL);
        ELEMENTS * selector(const char* pszSearch, ELEMENTS parents, ELEMENTS *result);
        ELEMENTS selector(const char* pszSearch, ELEMENTS parents);
        SxmlElement* selector(const char* pszSearch, SxmlElement* parent);
        ELEMENTS* selectorDescendant(STRINGS &bits, long pos, ELEMENTS parents, ELEMENTS *result);
        ELEMENTS* getElementsByAttr(const char* attr, const char* value, ELEMENTS parents, ELEMENTS *result);
        ELEMENTS* getElementsByTag(const char* tag, ELEMENTS parents, ELEMENTS *result);
        void applyStyles();
	int32 generateClass(SxmlElement* element);
	int32 generateClasses(SxmlElement* element, SxmlElement* parent = NULL);

protected :
	// [internal use]
	virtual SxmlNode* Clone() const;
	#ifdef SXML_USE_STL
	virtual void StreamIn( std::istream * in, SXML_STRING * tag );
	#endif

private:
	ATTRIBUTES styleList;
	cssNode cssRoot;
	void CopyTo( SxmlDocument* target ) const;

	bool error;
	int  errorId;
	SXML_STRING errorDesc;
	int tabsize;
	SxmlCursor errorLocation;
	bool useMicrosoftBOM;		// the UTF-8 BOM were found when read. Note this, and try to write.
};


/**
	A SxmlHandle is a class that wraps a node pointer with null checks; this is
	an incredibly useful thing. Note that SxmlHandle is not part of the Sxml
	DOM structure. It is a separate utility class.

	Take an example:
	@verbatim
	<Document>
		<Element attributeA = "valueA">
			<Child attributeB = "value1" />
			<Child attributeB = "value2" />
		</Element>
	<Document>
	@endverbatim

	Assuming you want the value of "attributeB" in the 2nd "Child" element, it's very 
	easy to write a *lot* of code that looks like:

	@verbatim
	SxmlElement* root = document.FirstChildElement( "Document" );
	if ( root )
	{
		SxmlElement* element = root->FirstChildElement( "Element" );
		if ( element )
		{
			SxmlElement* child = element->FirstChildElement( "Child" );
			if ( child )
			{
				SxmlElement* child2 = child->NextSiblingElement( "Child" );
				if ( child2 )
				{
					// Finally do something useful.
	@endverbatim

	And that doesn't even cover "else" cases. SxmlHandle addresses the verbosity
	of such code. A SxmlHandle checks for null	pointers so it is perfectly safe 
	and correct to use:

	@verbatim
	SxmlHandle docHandle( &document );
	SxmlElement* child2 = docHandle.FirstChild( "Document" ).FirstChild( "Element" ).Child( "Child", 1 ).ToElement();
	if ( child2 )
	{
		// do something useful
	@endverbatim

	Which is MUCH more concise and useful.

	It is also safe to copy handles - internally they are nothing more than node pointers.
	@verbatim
	SxmlHandle handleCopy = handle;
	@endverbatim

	What they should not be used for is iteration:

	@verbatim
	int i=0; 
	while ( true )
	{
		SxmlElement* child = docHandle.FirstChild( "Document" ).FirstChild( "Element" ).Child( "Child", i ).ToElement();
		if ( !child )
			break;
		// do something
		++i;
	}
	@endverbatim

	It seems reasonable, but it is in fact two embedded while loops. The Child method is 
	a linear walk to find the element, so this code would iterate much more than it needs 
	to. Instead, prefer:

	@verbatim
	SxmlElement* child = docHandle.FirstChild( "Document" ).FirstChild( "Element" ).FirstChild( "Child" ).ToElement();

	for( child; child; child=child->NextSiblingElement() )
	{
		// do something
	}
	@endverbatim
*/
class SxmlHandle
{
public:
	/// Create a handle from any node (at any depth of the tree.) This can be a null pointer.
	SxmlHandle( SxmlNode* _node )					{ this->node = _node; }
	/// Copy constructor
	SxmlHandle( const SxmlHandle& ref )			{ this->node = ref.node; }
	SxmlHandle operator=( const SxmlHandle& ref ) { this->node = ref.node; return *this; }

	/// Return a handle to the first child node.
	SxmlHandle FirstChild() const;
	/// Return a handle to the first child node with the given name.
	SxmlHandle FirstChild( const char * value ) const;
	/// Return a handle to the first child element.
	SxmlHandle FirstChildElement() const;
	/// Return a handle to the first child element with the given name.
	SxmlHandle FirstChildElement( const char * value ) const;

	/** Return a handle to the "index" child with the given name. 
		The first child is 0, the second 1, etc.
	*/
	SxmlHandle Child( const char* value, int index ) const;
	/** Return a handle to the "index" child. 
		The first child is 0, the second 1, etc.
	*/
	SxmlHandle Child( int index ) const;
	/** Return a handle to the "index" child element with the given name. 
		The first child element is 0, the second 1, etc. Note that only SxmlElements
		are indexed: other types are not counted.
	*/
	SxmlHandle ChildElement( const char* value, int index ) const;
	/** Return a handle to the "index" child element. 
		The first child element is 0, the second 1, etc. Note that only SxmlElements
		are indexed: other types are not counted.
	*/
	SxmlHandle ChildElement( int index ) const;

	#ifdef SXML_USE_STL
	SxmlHandle FirstChild( const string& _value ) const				{ return FirstChild( _value.c_str() ); }
	SxmlHandle FirstChildElement( const string& _value ) const		{ return FirstChildElement( _value.c_str() ); }

	SxmlHandle Child( const string& _value, int index ) const			{ return Child( _value.c_str(), index ); }
	SxmlHandle ChildElement( const string& _value, int index ) const	{ return ChildElement( _value.c_str(), index ); }
	#endif

	/** Return the handle as a SxmlNode. This may return null.
	*/
	SxmlNode* ToNode() const			{ return node; } 
	/** Return the handle as a SxmlElement. This may return null.
	*/
	SxmlElement* ToElement() const		{ return ( ( node && node->ToElement() ) ? node->ToElement() : 0 ); }
	/**	Return the handle as a SxmlText. This may return null.
	*/
	SxmlText* ToText() const			{ return ( ( node && node->ToText() ) ? node->ToText() : 0 ); }
	/** Return the handle as a SxmlUnknown. This may return null.
	*/
	SxmlUnknown* ToUnknown() const		{ return ( ( node && node->ToUnknown() ) ? node->ToUnknown() : 0 ); }

	/** @deprecated use ToNode. 
		Return the handle as a SxmlNode. This may return null.
	*/
	SxmlNode* Node() const			{ return ToNode(); } 
	/** @deprecated use ToElement. 
		Return the handle as a SxmlElement. This may return null.
	*/
	SxmlElement* Element() const	{ return ToElement(); }
	/**	@deprecated use ToText()
		Return the handle as a SxmlText. This may return null.
	*/
	SxmlText* Text() const			{ return ToText(); }
	/** @deprecated use ToUnknown()
		Return the handle as a SxmlUnknown. This may return null.
	*/
	SxmlUnknown* Unknown() const	{ return ToUnknown(); }

private:
	SxmlNode* node;
};


/** Print to memory functionality. The SxmlPrinter is useful when you need to:

	-# Print to memory (especially in non-STL mode)
	-# Control formatting (line endings, etc.)

	When constructed, the SxmlPrinter is in its default "pretty printing" mode.
	Before calling Accept() you can call methods to control the printing
	of the XML document. After SxmlNode::Accept() is called, the printed document can
	be accessed via the CStr(), Str(), and Size() methods.

	SxmlPrinter uses the Visitor API.
	@verbatim
	SxmlPrinter printer;
	printer.SetIndent( "\t" );

	doc.Accept( &printer );
	fprintf( stdout, "%s", printer.CStr() );
	@endverbatim
*/
class SxmlPrinter : public SxmlVisitor
{
public:
	SxmlPrinter() : depth( 0 ), simpleTextPrint( false ),
					 buffer(), indent( "    " ), lineBreak( "\n" ) {}

	virtual bool VisitEnter( const SxmlDocument& doc );
	virtual bool VisitExit( const SxmlDocument& doc );

	virtual bool VisitEnter( const SxmlElement& element, const SxmlAttribute* firstAttribute );
	virtual bool VisitExit( const SxmlElement& element );

	virtual bool Visit( const SxmlDeclaration& declaration );
	virtual bool Visit( const SxmlText& text );
	virtual bool Visit( const SxmlComment& comment );
	virtual bool Visit( const SxmlUnknown& unknown );

	/** Set the indent characters for printing. By default 4 spaces
		but tab (\t) is also useful, or null/empty string for no indentation.
	*/
	void SetIndent( const char* _indent )			{ indent = _indent ? _indent : "" ; }
	/// Query the indention string.
	const char* Indent()							{ return indent.c_str(); }
	/** Set the line breaking string. By default set to newline (\n). 
		Some operating systems prefer other characters, or can be
		set to the null/empty string for no indenation.
	*/
	void SetLineBreak( const char* _lineBreak )		{ lineBreak = _lineBreak ? _lineBreak : ""; }
	/// Query the current line breaking string.
	const char* LineBreak()							{ return lineBreak.c_str(); }

	/** Switch over to "stream printing" which is the most dense formatting without 
		linebreaks. Common when the XML is needed for network transmission.
	*/
	void SetStreamPrinting()						{ indent = "";
													  lineBreak = "";
													}	
	/// Return the result.
	const char* CStr()								{ return buffer.c_str(); }
	/// Return the length of the result string.
	size_t Size()									{ return buffer.size(); }

	#ifdef SXML_USE_STL
	/// Return the result.
	const string& Str()						{ return buffer; }
	#endif

private:
	void DoIndent()	{
		for( int i=0; i<depth; ++i )
			buffer += indent;
	}
	void DoLineBreak() {
		buffer += lineBreak;
	}

	int depth;
	bool simpleTextPrint;
	SXML_STRING buffer;
	SXML_STRING indent;
	SXML_STRING lineBreak;
};


#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif

