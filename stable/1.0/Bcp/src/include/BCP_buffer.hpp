// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_BUFFER_H
#define _BCP_BUFFER_H

#include <memory>
#include <vector>

// This file is fully docified.

#include "BCP_error.hpp"
#include "BCP_string.hpp"
#include "BCP_message_tag.hpp"
#include "BCP_message.hpp"
#include "BCP_vector.hpp"

/**        
   This class describes the message buffer used for all processes of BCP.
   This buffer is a character array; the components of a message are simply
   copied into this array one after the other. Note that each process has
   only one buffer, which serves both for outgoing and incoming messages.
   This can be done since when a message arrives it is completely unpacked
   before anything else is done and conversely, once a message is started to
   be packed together it will be sent out before another message is received.
       
   NOTE: Only the following type of objects can be packed with the various
   <code>pack()</code> member methods:
   <ul>
     <li> Objects that can be assigned with <code>memcpy()</code>, i.e.,
          built-in non-pointer types, and structs recursively built from such
	  types;
     <li> <code>BCP_vec</code>s of the above and
     <li> <code>BCP_string</code>s.
   </ul>
   Everything else that needs to be packed at any time must have a
   <code>pack()</code> member method.
*/

class BCP_buffer{
public:
   /* The data members are public for efficiency reasons. The message
       environment's receiving function should be able to directly manipulate
       these fields. However, making the virtual base class of all message
       passing environment to be a friend doesn't help... Anyway, access these
       fields sparingly.<br>
       THINK: maybe it's not that inefficient to access the fields thru
       functions... */
   /**@name Data members 
       The data members are public for efficiency reasons. Access these
       fields sparingly.<br>
       THINK: maybe it's not that inefficient to access the fields thru
       functions...

       Note that:
       <ol>
         <li>The size of the buffer never decreases. The buffer size increases
	     when the current size is not sufficient for the message.
	 <li>With normal usage for incoming messages <code>_size</code> stays
	     constant while reading out the message and <code>_pos</code> moves
	     forward in the buffer.
	 <li>With normal usage for outgoing messages <code>_size</code> and
	     <code>_pos</code> moves forward together as the buffer is filled
	     with the message.
       </ol>
   */ 
   /*@{*/
   /** The message tag of the last <em>received</em> message. This member has
       no meaning if the buffer holds an outgoing message. */
   BCP_message_tag _msgtag;
   /** The process id of the sender of the last <em>received</em> message.
       This member has no meaning if the buffer holds an outgoing message. */ 
   BCP_proc_id*    _sender;
   /** The next read/write position in the buffer. */
   size_t _pos;
   /** The amount of memory allocated for the buffer. */
   size_t _max_size;
   /** The current size of the message (the first <code>_size</code> bytes of
       the buffer). */
   size_t _size;
   /** Pointer to the buffer itself. */
   char*  _data;
   /*@}*/

public:
   //=========================================================================
   /**@name Query methods */
   /*@{*/
   /** Return the message tag of the message in the buffer. */
   inline BCP_message_tag msgtag() const { return _msgtag; }
   /** Return a const pointer to the process id of the sender of the message
       in the buffer. */
   inline const BCP_proc_id* sender() const { return _sender; }
   /** Return the size of the current message in the buffer. */
   inline int                size() const { return _size; }
   /** Return a const pointer to the data stored in the buffer. */
   inline const char*        data() const { return _data; }
   /*@}*/
   //=========================================================================

   /**@name Modifying methods */
   /*@{*/
   /** Position the read/write head in the buffer. Must be between 0 and
       size(). */
   inline void set_position(const int pos) throw(BCP_fatal_error) {
     if (pos < 0 || pos >= size())
       throw BCP_fatal_error("Incorrest buffer position setting.\n");
     _pos = pos;
   }
   /** Set the message tag on the buffer */
   inline void set_msgtag(const BCP_message_tag tag) { _msgtag = tag; }

  /** Set the buffer to be a copy of the given data. Use this with care! */
  void set_content(const char* data, const size_t size,
		   BCP_proc_id* sender, BCP_message_tag msgtag) {
    _sender = sender;
    _msgtag = msgtag;
    if (_max_size < size) {
      delete[] _data;
      _data = new char[size];
      _max_size = size;
    }
    _pos = 0;
    _size = size;
    if (_size)
      memcpy(_data, data, size * sizeof(char));
   }
     
    
   /** Make an exact replica of the other buffer. */
   BCP_buffer& operator=(const BCP_buffer& buf) {
      _msgtag = buf._msgtag;
      _sender = buf._sender ? buf._sender->clone() : 0;
      _pos = buf._pos;
      if (_max_size < buf._max_size) {
	 delete[] _data;
	 _data = new char[buf._max_size];
	 _max_size = buf._max_size;
      }
      _size = buf._size;
      if (_size)
	 memcpy(_data, buf._data, _size * sizeof(char));
      return *this;
   }
   /** Reallocate the buffer if necessary so that at least
       <code>add_size</code> number of additional bytes will fit into the
       buffer. */
   inline void make_fit(const int add_size){
      if (_max_size < _size + add_size){
	 _max_size = 2 * (_size + add_size + 0x1000/*4K*/);
	 char *new_data = new char[_max_size];
	 if (_size)
	    memcpy(new_data, _data, _size);
	 delete[] _data;
	 _data = new_data;
      }
   }
   /** Completely clear the buffer. Delete and zero out <code>_msgtag, _size,
       _pos</code> and <code>_sender</code>. */
   inline void clear(){
      _msgtag = BCP_Msg_NoMessage;
      _size = 0;
      _pos = 0;
      delete _sender; _sender = 0;
   }

   /** Pack a single object of type <code>T</code>. Copies
       <code>sizeof(T)</code> bytes from the address of the object. */
   template <class T> BCP_buffer& pack(const T& value) {
     make_fit( sizeof(T) );
     memcpy(_data + _size, &value, sizeof(T));
     _size += sizeof(T);
     return *this;
   }

   /** Unpack a single object of type <code>T</code>. Copies
       <code>sizeof(T)</code> bytes to the address of the object. */
   template <class T> BCP_buffer& unpack(T& value){
#ifdef PARANOID
     if (_pos + sizeof(T) > _size)
       throw BCP_fatal_error("Reading over the end of buffer.\n");
#endif
     memcpy(&value, _data + _pos, sizeof(T));
     _pos += sizeof(T);
     return *this;
   }

   /** Pack a C style array of objects of type <code>T</code>. */
   template <class T> BCP_buffer& pack(const T* const values,
				       const int length){
     make_fit( sizeof(int) + sizeof(T) * length );
     memcpy(_data + _size, &length, sizeof(int));
     _size += sizeof(int);
     if (length > 0){
       memcpy(_data + _size, values, sizeof(T) * length);
       _size += sizeof(T) * length;
     }
     return *this;
   }

   /** Unpack an array of objects of type <code>T</code>, where T
       <strong>must</strong> be a built-in type (ar at least something that
       can be copied with memcpy).

       If the third argument is true then memory is allocated for the array
       and the array pointer and the length of the array are returned in the
       arguments.

       If the third argument is false then the arriving array's length is
       compared to <code>length</code> and an exception is thrown if they are
       not the same. Also, the array passed as the first argument will be
       filled with the arriving array.
   */
   template <class T> BCP_buffer& unpack(T*& values, int& length,
					 bool allocate = true)
     throw(BCP_fatal_error) {
     if (allocate) {
#ifdef PARANOID
       if (_pos + sizeof(int) > _size)
	 throw BCP_fatal_error("Reading over the end of buffer.\n");
#endif
       memcpy(&length, _data + _pos, sizeof(int));
       _pos += sizeof(int);
       if (length > 0){
#ifdef PARANOID
	 if (_pos + sizeof(T)*length > _size)
	   throw BCP_fatal_error("Reading over the end of buffer.\n");
#endif
	 values = new T[length];
	 memcpy(values, _data + _pos, sizeof(T)*length);
	 _pos += sizeof(T) * length;
       }
       
     } else { /* ! allocate */

       int l;
#ifdef PARANOID
       if (_pos + sizeof(int) > _size)
	 throw BCP_fatal_error("Reading over the end of buffer.\n");
#endif
       memcpy(&l, _data + _pos, sizeof(int));
       _pos += sizeof(int);
       if (l != length)
	 throw BCP_fatal_error("BCP_buffer::unpack() : bad array lentgh.\n");
       if (length > 0){
#ifdef PARANOID
	 if (_pos + sizeof(T)*length > _size)
	   throw BCP_fatal_error("Reading over the end of buffer.\n");
#endif
	 memcpy(values, _data + _pos, sizeof(T)*length);
	 _pos += sizeof(T) * length;
       }
       
     }
     
     return *this;
   }

   /** Pack a <code>BCP_string</code> into the buffer. */
   BCP_buffer& pack(const BCP_string& value){
      // must define here, 'cos in BCP_message.C we have only templated members
      int len = value.length();
      make_fit( sizeof(int) + len );
      memcpy(_data + _size, &len, sizeof(int));
      _size += sizeof(int);
      if (len > 0){
	 memcpy(_data + _size, value.c_str(), len);
	 _size += len;
      }
      return *this;
   }
   /** Pack a <code>BCP_string</code> into the buffer. */
   BCP_buffer& pack(BCP_string& value){
      // must define here, 'cos in BCP_message.C we have only templated members
      int len = value.length();
      make_fit( sizeof(int) + len );
      memcpy(_data + _size, &len, sizeof(int));
      _size += sizeof(int);
      if (len > 0){
	 memcpy(_data + _size, value.c_str(), len);
	 _size += len;
      }
      return *this;
   }
   /** Unpack a <code>BCP_string</code> from the buffer. */
   BCP_buffer& unpack(BCP_string& value){
      int len;
      unpack(len);
      value.assign(_data + _pos, len);
      _pos += len;
      return *this;
   }

   // packing/unpacking for BCP_vec
   /** Pack a <code>BCP_vec</code> into the buffer. */
   template <class T> BCP_buffer& pack(const BCP_vec<T>& vec) {
     int objnum = vec.size();
     int new_bytes = objnum * sizeof(T);
     make_fit( sizeof(int) + new_bytes );
     memcpy(_data + _size, &objnum, sizeof(int));
     _size += sizeof(int);
     if (objnum > 0){
       memcpy(_data + _size, vec.begin(), new_bytes);
       _size += new_bytes;
     }
     return *this;
   }

   /** Pack a <code>std::vector</code> into the buffer. */
   template <class T> BCP_buffer& pack(const std::vector<T>& vec) {
     int objnum = vec.size();
     int new_bytes = objnum * sizeof(T);
     make_fit( sizeof(int) + new_bytes );
     memcpy(_data + _size, &objnum, sizeof(int));
     _size += sizeof(int);
     if (objnum > 0){
       memcpy(_data + _size, &vec[0], new_bytes);
       _size += new_bytes;
     }
     return *this;
   }

   /** Unpack a <code>BCP_vec</code> from the buffer. */
   template <class T> BCP_buffer& unpack(BCP_vec<T>& vec) {
     int objnum;
#ifdef PARANOID
     if (_pos + sizeof(int) > _size)
       throw BCP_fatal_error("Reading over the end of buffer.\n");
#endif
     memcpy(&objnum, _data + _pos, sizeof(int));
     _pos += sizeof(int);
     vec.clear();
     if (objnum > 0){
#ifdef PARANOID
       if (_pos + sizeof(T)*objnum > _size)
	 throw BCP_fatal_error("Reading over the end of buffer.\n");
#endif
       vec.reserve(objnum);
       vec.insert(vec.end(), _data + _pos, objnum);
       _pos += objnum * sizeof(T);
     }
     return *this;
   }

   /** Unpack a <code>std::vector</code> from the buffer. */
   template <class T> BCP_buffer& unpack(std::vector<T>& vec) {
     int objnum;
#ifdef PARANOID
     if (_pos + sizeof(int) > _size)
       throw BCP_fatal_error("Reading over the end of buffer.\n");
#endif
     memcpy(&objnum, _data + _pos, sizeof(int));
     _pos += sizeof(int);
     vec.clear();
     if (objnum > 0){
#ifdef PARANOID
       if (_pos + sizeof(T)*objnum > _size)
	 throw BCP_fatal_error("Reading over the end of buffer.\n");
#endif
       vec.insert(vec.end(), objnum, T());
       memcpy(&vec[0], _data + _pos, objnum * sizeof(T));
       _pos += objnum * sizeof(T);
     }
     return *this;
   }

   /*@}*/

   /**@name Constructors and destructor */
   /*@{*/
   /** The default constructor creates a buffer of size 16 Kbytes with no
       message in it. */
   BCP_buffer() : _msgtag(BCP_Msg_NoMessage), _sender(0), _pos(0),
      _max_size(0x4000/*16K*/), _size(0), _data(new char[_max_size]) {}
   /** The copy constructor makes an exact replica of the other buffer. */
   BCP_buffer(const BCP_buffer& buf) :
      _msgtag(BCP_Msg_NoMessage), _sender(0), _pos(0),
      _max_size(0), _size(0), _data(0){
	 operator=(buf);
   }
   /** The desctructor deletes all data members (including freeing the
       buffer). */
   ~BCP_buffer() {
      delete _sender;   _sender = 0;
      delete[] _data;
   }
   /*@}*/
};

#endif
