// Copyright (C) 2000, International Business Machines
// Corporation and others.  All Rights Reserved.
#ifndef _BCP_PARAMETERS_H
#define _BCP_PARAMETERS_H

// This file is fully docified.


#include <utility> // for 'pair'
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

#if defined(__GNUC__) && (__GNUC__ >= 3)
#    include <sstream>
#    include <locale>
#    define BCP_STRINGSTREAM std::istringstream
#else
#    include <strstream>
#    include <cctype>
#    define BCP_STRINGSTREAM std::istrstream
#endif

#include "BCP_error.hpp"
#include "BCP_string.hpp"
#include "BCP_vector.hpp"
#include "BCP_buffer.hpp"

/** This enumerative constant describes the possible parameter types. */
enum BCP_parameter_t{
  /** The type is not yet specified. Used only in the default constructor of
      a BCP parameter. */
  BCP_NoPar,
  /** Character parameter. */
  BCP_CharPar,
  /** Integer parameter. */
  BCP_IntPar,
  /** Double parameter. */
  BCP_DoublePar,
  /** String parameter. */
  BCP_StringPar,
  /** The parameter is an array of strings. (E.g., the names of machines in
      the parallel configuration.) */
  BCP_StringArrayPar
};

//-----------------------------------------------------------------------------

/** This parameter indeintifies a single parameter entry. */

class BCP_parameter {

private:
  /**@name Data members */
  /*@{*/
    /** The type of the parameter (e.g., BCP_IntPar). */
    BCP_parameter_t _type;
    /** The index of this parameter within all parameters of the same type. */
    int _index;
  /*@}*/

public:
  // default copy constructor and assignment operator are fine 
  /**@name Constructors / Destructor */
  /*@{*/
    /** The default constructor creates a phony parameter. */
    BCP_parameter() : _type(BCP_NoPar), _index(0) {}
    /** Constructor where members are specified. */
    BCP_parameter(const BCP_parameter_t t, const int i) :
      _type(t), _index(i) {}
    /** The destructor. */
    ~BCP_parameter() {}
  /*@}*/

  /**@name Query methods */
  /*@{*/
    /** Return the type of the parameter. */
    BCP_parameter_t type() const { return _type; }
    /** Return the index of the parameter within all parameters of the same
	type. */
    int index() const            { return _index; }
  /*@}*/
};

//-----------------------------------------------------------------------------

/** This is the class serves as a holder for a set of parameters. 

    For example, BCP stores has a parameter set for each process. Of course,
    the user can use this class for her own parameters. To use this class the
    user must 
    <ol>
      <li> first create a \c struct with the names of the parameters (see,
           e.g., BCP_tm_par. Assume that the structure is named \c USER_par.
      <li> define the member functions
           <code>BCP_parameter_set<USER_par>::create_keyword_list()</code> and
           <code>BCP_parameter_set<USER_par>::set_default_entries()</code>.
	   For an example look at the file
	   <code>BCP-common/TM/BCP_tm_param.cpp</code>. Essentially, the first
	   method defines what keywords should be looked for in the parameter
	   file, and if one is found which parameter should take the
	   corresponding value; the other method specifies the default values
	   for each parameter.
      <li> in her source code declare a variable 
           "<code>BCP_parameter_set<USER_par> param;</code>"
    </ol>

    After this the user can read in the parameters from a file, she can
    set/access the parameters in the parameter, etc.
*/

template <class Par> class BCP_parameter_set : public Par {
private:
  /**@name Private methods that must be defined for each parameter set.
     If the user creates a new parameter set, she must define these two
     methods for the class. */
  /*@{*/
    /** Method for creating the list of keyword looked for in the parameter
	file. */
    void create_keyword_list();
    /** Method for setting the default values for the parameters. */
    void set_default_entries();
  /*@}*/
  //---------------------------------------------------------------------------

public:
  /* Type definitions. Just to make it easier to refer to types. */
  typedef typename Par::chr_params chr_params;
  typedef typename Par::int_params int_params;
  typedef typename Par::dbl_params dbl_params;
  typedef typename Par::str_params str_params;
  typedef typename Par::str_array_params str_array_params;

private:
  /**@name Data members. All of them are private. */
  /*@{*/
    /** The keyword, parameter pairs. Used when the parameter file is read in.
     */
    BCP_vec< std::pair<BCP_string, BCP_parameter> > keys;
    /** list of obsolete keywords. If any of these is encountered a warning is
	printed. */
    BCP_vec<BCP_string> obsolete_keys;
    /** The character parameters. */
    char*                cpar;
    /** The integer parameters. */
    int*                 ipar;
    /** The double parameters. */
    double*              dpar;
    /** The string (actually, BCP_string) parameters. */
    BCP_string*          spar;
    /** The string array parameters */
    BCP_vec<BCP_string>* sapar;
  /*@}*/
  //---------------------------------------------------------------------------

public:
  /**@name Query methods 
      
     The members of the parameter set can be queried for using the overloaded
     entry() method. Using the example in the class
     documentation the user can get a parameter with the
     "<code>param.entry(USER_par::parameter_name)</code>" expression.
    */
  /*@{*/
    ///
    inline char
    entry(const chr_params key) const { return cpar[key]; }
    ///
    inline int
    entry(const int_params key) const { return ipar[key]; }
    ///
    inline double
    entry(const dbl_params key) const { return dpar[key]; }
    ///
    inline const BCP_string&
    entry(const str_params key) const { return spar[key]; }
    ///
    inline const BCP_vec<BCP_string>&
    entry(const str_array_params key) const { return sapar[key]; }
  /*@}*/
  //---------------------------------------------------------------------------

  /**@name Set methods 

     First, there is the assignment operator that sets the whole parameter
     set at once.

     Individual members of the parameter set can be set for using the
     overloaded set_entry() method. Using the example in the
     class documentation the user can set a parameter with the
     "<code>param.set_entry(USER_par::parameter_name, param_value)</code>"
     expression. */
  /*@{*/
    ///
    BCP_parameter_set<Par>& operator=(const BCP_parameter_set<Par>& x) {
      // no need to delete anything, since the size of (almost) everything is
      // the same, just copy over
      // -- The static_cast is needed to satisfy the more picky IBM Visual Age
      //    C++ compiler
      std::copy(x.cpar, x.cpar + static_cast<int>(Par::end_of_chr_params),
		cpar);
      std::copy(x.ipar, x.ipar + static_cast<int>(Par::end_of_int_params),
		ipar);
      std::copy(x.dpar, x.dpar + static_cast<int>(Par::end_of_dbl_params),
		dpar);
      std::copy(x.spar, x.spar + static_cast<int>(Par::end_of_str_params),
		spar);
      std::copy(x.sapar,
		x.sapar + static_cast<int>(Par::end_of_str_array_params),
		sapar);
      return *this;
    }
    /// 
    void set_entry(const chr_params key, const char val) {
      cpar[key] = val; }
    ///
    void set_entry(const chr_params key, const bool val) {
      cpar[key] = val; }
    ///
    void set_entry(const int_params key, const int val) {
      ipar[key] = val; }
    ///
    void set_entry(const dbl_params key, const double val) {
      dpar[key] = val; }
    ///
    void set_entry(const str_params key, const char * val) {
      spar[key] = val; }
    ///
    void set_entry(const str_array_params key, const char *val) {
      sapar[key].push_back(val); }
    ///
    void set_entry(const BCP_parameter key, const char * val) {
      switch (key.type()){
      case BCP_NoPar: break;
      case BCP_CharPar:        cpar [key.index()] = atoi(val);    break;
      case BCP_IntPar:         ipar [key.index()] = atoi(val);    break;
      case BCP_DoublePar:      dpar [key.index()] = atof(val);    break;
      case BCP_StringPar:      spar [key.index()] = val;          break;
      case BCP_StringArrayPar: sapar[key.index()].push_back(val); break;
      }
    }
  /*@}*/
  //---------------------------------------------------------------------------

  /**@name Read parameters from a stream. */
  /*@{*/
  /** Read the parameters from the stream specified in the argument.

      The stream is interpreted as a lines separated by newline characters.
      The first word on each line is tested for match with the keywords
      specified in the create_keyword_list() method. If there is
      a match then the second word will be interpreted as the value for the
      corresponding parameter. Any further words on the line are discarded.
      Every non-matching line is discarded. 

      If the keyword corresponds to a non-array parameter then the new value
      simply overwrites the old one. Otherwise, i.e., if it is a
      StringArrayPar, the value is appended to the list of strings in that
      array. 
  */
    void read_from_stream(std::istream& parstream) {
      // Get the lines of the parameter file one-by-one and if a line contains
      // a (keyword, value) pair set the appropriate parameter
      const int MAX_PARAM_LINE_LENGTH = 1024;
      char line[MAX_PARAM_LINE_LENGTH], *end_of_line, *keyword, *ctmp;
      char value[MAX_PARAM_LINE_LENGTH];
      bool quiet = true;

      BCP_vec< std::pair<BCP_string, BCP_parameter> >::const_iterator ind;
      BCP_vec<BCP_string>::const_iterator obs_ind;
      printf("\
BCP_parameters::read_from_stream   Scanning parameter stream.\n");
      while (!parstream.eof()) {
	 parstream.getline(line, MAX_PARAM_LINE_LENGTH);
	 const int len = strlen(line);
	 if (len == MAX_PARAM_LINE_LENGTH - 1) {
	    sprintf(line, "\
There's a too long (>= %i characters) line in the parameter file.\n\
This is absurd.\n", MAX_PARAM_LINE_LENGTH);
	    throw BCP_fatal_error(line);
	 }

	 end_of_line = line + len;

	 //------------------------ First separate the keyword and value ------
	 // keyword = std::find_if(line, end_of_line, isgraph);
	 for (keyword = line; keyword < end_of_line; ++keyword) {
	    if (isgraph(*keyword))
	       break;
	 }
	 if (keyword == end_of_line) // empty line
	    continue;
	 // ctmp = std::find_if(keyword, end_of_line, isspace);
	 for (ctmp = keyword; ctmp < end_of_line; ++ctmp) {
	    if (isspace(*ctmp))
	       break;
	 }
	 *ctmp = 0; // terminate the keyword with a 0 character
	 // ctmp = std::find_if(ctmp, end_of_line, isgraph);
	 for ( ; ctmp < end_of_line; ++ctmp) {
	    if (isgraph(*ctmp))
	       break;
	 }
	 if (ctmp == end_of_line) // line is just one word. must be a comment
	    continue;

	 int i;
	 if (*ctmp == '"') {
	    ++ctmp;
	    for (i = 0; ctmp < end_of_line; ++ctmp) {
	       if (*ctmp == '\\') {
		  if (++ctmp == end_of_line)
		     break;
		  value[i++] = *ctmp;
		  continue;
	       }
	       if (*ctmp != '"') {
		  value[i++] = *ctmp;
	       } else {
		  ++ctmp;
		  break;
	       }
	    }
	 } else {
	    for (i = 0; ctmp < end_of_line; ++ctmp) {
	       if (!isspace(*ctmp)) {
		  value[i++] = *ctmp;
	       } else {
		  break;
	       }
	    }
	 }
	 value[i] = 0;

	 //--------------- Check if the keyword is a param file ---------------
	 if (strcmp(keyword, "ParamFile") == 0) {
	    read_from_file(value);
	 }

	 //--------------- Check if we need to be quiet -----------------------
	 if (strcmp(keyword, "Quiet") == 0) {
	    int val = atoi(value);
	    quiet = (val != 0);
	 }
	 
	 //--------------- Expand the value (look for keywords) ---------------
	 std::string value_expanded = expand(value);

	 //--------------- Find the parameter corresponding to  the keyword ---
	 for (ind = keys.begin(); ind != keys.end(); ++ind) {
	    if (ind->first == keyword) {
	       // The keyword does exists
	       // set_param(ind->second, value);    should work
	       if (!quiet) {
		  printf("%s %s\n", keyword, value_expanded.c_str());
	       }
	       set_entry((*ind).second, value_expanded.c_str());
	       break;
	    }
	 }

	 for (obs_ind = obsolete_keys.begin();
	      obs_ind != obsolete_keys.end();
	      ++obs_ind) {
	    if (*obs_ind == keyword) {
	       // The keyword does exists but is obsolete
	       printf("***WARNING*** : Obsolete keyword `%s' is found.\n",
		      keyword);
	       break;
	    }
	 }
      }
      if (!quiet) {
	 printf("\
BCP_parameters::read_from_stream   Finished scanning parameter stream.\n\n");
      }
    }


  /*@}*/
  //---------------------------------------------------------------------------
  /**@name Expand parameter value (look for environment vars). */
  /*@{*/
  std::string expand(const char* value){

    // std::cout << "expand( " << value << " ) : ";
    
    const int MAX_PARAM_LINE_LENGTH = 1024;
    char valueBuf[MAX_PARAM_LINE_LENGTH];
    bool bDollar = false;
    int j = 0;
    for(int i = 0; value[i] != '\0'; i++){
      char cval = value[i];
      if(!bDollar){
	if(cval == '$'){
	  bDollar = true;
	  continue;
	}
	valueBuf[j++] = cval;
	continue;
      }
      else{
	if(cval == '('){
	  char envBuf[MAX_PARAM_LINE_LENGTH];
	  //const char *ptr = &value[i+1];
	  int k=0;
	  char c;
	  i++;
	  while((c = value[i++]) != ')' && c != '\0')
	    envBuf[k++] = c;
	  envBuf[k] = '\0';
	  char* eVal = getenv(envBuf);
	  if(eVal != NULL){
	    while(*eVal != '\0')
	      valueBuf[j++] = *eVal++;
	  }
	  bDollar = false;
	  i--;   
	} else {
	  valueBuf[j++] = '$';
	  valueBuf[j++] = cval;
	  bDollar = false;
	}
      }      
    }   
    valueBuf[j] = '\0';
    std::string sExpand(valueBuf);
    
    // std::cout << sExpand << std::endl;

    return sExpand;
  }

  /*@}*/
  //---------------------------------------------------------------------------

  /**@name Read parameters from a file. */
  /*@{*/
    /** Simply invoke reading from a stream. */
    void read_from_file(const char * paramfile) {
      // Open the parameter file
      std::ifstream parstream(paramfile);
      if (!parstream)
	 throw BCP_fatal_error("Cannot open parameter file");
      read_from_stream(parstream);
    }
  /*@}*/
  //---------------------------------------------------------------------------

  /**@name Read parameters from the command line */
  /*@{*/
    /** Simply invoke reading from a stream. */
    void read_from_arglist(const int argnum, const char * const * arglist) {
       // create a stream
       std::string argstring;
       for (int i = 1; i < argnum; i += 2) {
	  argstring += arglist[i];
	  argstring += " ";
	  if (i+1 < argnum) {
	     argstring += arglist[i+1];
	  }
	  argstring += "\n";
       }
       BCP_STRINGSTREAM parstream(argstring.c_str());
       read_from_stream(parstream);
    }
  /*@}*/
  //---------------------------------------------------------------------------
  /**@name Write parameters to a stream. */
  /*@{*/
  /** Write keyword-value pairs to the stream specified in the argument.

      Each keyword-value pair is separated by a newline character. 
  */
   void write_to_stream(std::ostream& outstream) const {
      
      const int size = keys.size();
      for (int i = 0; i < size; ++i) {
	 const BCP_string& key = keys[i].first;
	 const BCP_parameter& par = keys[i].second;
	 switch (par.type()) {
	  case BCP_CharPar:
	    outstream << key.c_str() << "   "
		      << static_cast<int>(cpar[par.index()]) << "\n";
	    break;
	  case BCP_IntPar:
	    outstream << key.c_str() << "   "
		      << ipar[par.index()] << "\n";
	    break;
	  case BCP_DoublePar:
	    outstream << key.c_str() << "   "
		      << dpar[par.index()] << "\n";
	    break;
	  case BCP_StringPar:
	    outstream << key.c_str() << "   "
		      << spar[par.index()].c_str() << "\n";
	    break;
	  case BCP_StringArrayPar:
	    for (size_t j = 0; j < sapar[par.index()].size(); ++j) {
	       outstream << key.c_str() << "   "
			 << sapar[par.index()][j].c_str() << "\n";
	    }
	    break;
	  case BCP_NoPar:
	  default:
	    // error
	    throw BCP_fatal_error("\
BCP_parameters::write_to_stream   ERROR: Unrecognized parameter type!\n");
	    break;
	 }
	 
      }
       
   }

  //---------------------------------------------------------------------------

  /**@name Packing/unpacking methods */
  /*@{*/
    /** Pack the parameter set into the buffer. */
    void pack(BCP_buffer& buf) {
      buf.pack(cpar, Par::end_of_chr_params)
	 .pack(ipar, Par::end_of_int_params)
	 .pack(dpar, Par::end_of_dbl_params);
      for (int i = 0; i < Par::end_of_str_params; ++i)
	 buf.pack(spar[i]);
      for (int i = 0; i < Par::end_of_str_array_params; ++i) {
	 buf.pack(sapar[i].size());
	 for (size_t j = 0; j < sapar[i].size(); ++j)
	    buf.pack(sapar[i][j]);
      }
    }
    /** Unpack the parameter set from the buffer. */
    void unpack(BCP_buffer& buf) {
      int dummy;
      // No need to allocate the arrays, they are of fixed length
      dummy = static_cast<int>(Par::end_of_chr_params);
      buf.unpack(cpar, dummy, false);
      dummy = static_cast<int>(Par::end_of_int_params);
      buf.unpack(ipar, dummy, false);
      dummy = static_cast<int>(Par::end_of_dbl_params);
      buf.unpack(dpar, dummy, false);
      for (int i = 0; i < Par::end_of_str_params; ++i)
	 buf.unpack(spar[i]);
      for (int i = 0; i < Par::end_of_str_array_params; ++i) {
	 size_t str_size;
	 buf.unpack(str_size);
	 sapar[i].reserve(str_size);
	 for (size_t j = 0; j < str_size; ++j){
	    sapar[i].unchecked_push_back(BCP_string());
	    buf.unpack(sapar[i].back());
	 }
      }
    }
  /*@}*/
  //---------------------------------------------------------------------------

  /**@name Constructors and destructor. */
  /*@{*/
    /** The default constructor creates a parameter set with from the template
	argument structure. The keyword list is created and the defaults are
	set. */
    BCP_parameter_set() :
      keys(),
      cpar(new char[static_cast<int>(Par::end_of_chr_params)]),
      ipar(new int[static_cast<int>(Par::end_of_int_params)]),
      dpar(new double[static_cast<int>(Par::end_of_dbl_params)]),
      spar(new BCP_string[static_cast<int>(Par::end_of_str_params)]),
      sapar(new BCP_vec<BCP_string>[static_cast<int>(Par::end_of_str_array_params)])
    {
      create_keyword_list();
      set_default_entries();
    }
    /** The destructor deletes all data members. */
    ~BCP_parameter_set() {
      delete[] cpar;
      delete[] ipar;
      delete[] dpar;
      delete[] spar;
      delete[] sapar;
    }
  /*@}*/
};

#endif
