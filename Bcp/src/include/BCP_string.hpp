// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_STRING_H
#define _BCP_STRING_H

// This file is fully docified.

#include <cstring>

/** This class is a very simple impelementation of a constant length string.
    Using it one can avoid some memory errors related to using functions
    operating on C style strings. */
class BCP_string {
public:
   /* Return the length of the string. */
   int length() const         { return _len; }
   /* Return a pointer to the data stored in the string. I.e., return a C
      style string. */
   const char * c_str() const { return _data; }
private:
   /* the length of the string */
   int    _len;
   /* the data in the string */
   char * _data;
public:
   /* The default constructor creates an empty sting. */
   BCP_string() : _len(0), _data(0) {};
   /* Create a <code>BCP_string</code> from a C style string. */
   BCP_string(const char * str) {
      _len = strlen(str);
      _data = new char[_len+1];
      memcpy(_data, str, _len);
      _data[_len] = 0;
   }
   /* Make a copy of the argument string. */
   BCP_string(const BCP_string& str) {
      _len = str.length();
      _data = new char[_len+1];
      memcpy(_data, str.c_str(), _len);
      _data[_len] = 0;
   }
   /* Delete the data members. */
   ~BCP_string() {
      delete[] _data;
   }
   /* This methods replaces the current <code>BCP_string</code> with one
      create from the first <code>len</code> bytes in <code>source</code>. */
   BCP_string& assign(const char * source, const int len) {
      delete[] _data;
      _len = len;
      _data = new char[_len+1];
      memcpy(_data, source, _len);
      _data[_len] = 0;
      return *this;
   }
   /* replace the current <code>BCP_string</code> with a copy of the argument
    */ 
   BCP_string& operator= (const BCP_string& str) {
      return assign(str.c_str(), str.length());
   }
   /* replace the current <code>BCP_string</code> with a copy of the argument
      C style string. */ 
   BCP_string& operator= (const char * str) {
      return assign(str, strlen(str));
   }

};

/** Equality tester for a <code>BCP_string</code> and a C style string. */
inline bool operator==(const BCP_string& s0, const char* s1) {
   if (s0.c_str() == 0)
      return s1 == 0;
   return s1 == 0 ? false : (strcmp(s0.c_str(), s1) == 0);
}

/** Equality tester for a C style string and a <code>BCP_string</code>. */
inline bool
operator==(const BCP_string& s0, const BCP_string& s1) {
   return s0 == s1.c_str();
}

/** Equality tester for two <code>BCP_string</code>s. */
inline bool
operator==(const char* s0, const BCP_string& s1) {
   return s1 == s0;
}

#endif
