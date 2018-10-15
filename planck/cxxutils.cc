/*
 *  This file contains the implementation of various convenience functions
 *  used by the Planck LevelS package.
 *
 *  Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007 Max-Planck-Society
 *  Authors: Martin Reinecke, Reinhard Hell
 */

/*
 * TODO: (EGP) clean up unused things
 */

// if we are using g++, check for version 3.0 or higher
#ifdef __GNUC__
#if (__GNUC__<3)
#error your C++ compiler is too old. g++ version 3.0 or higher is required.
#endif
#endif

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "cxxutils.h"
#include "datatypes.h"
#include "openmp_support.h"

using namespace std;

bool file_present (const string &filename)
  {
  ifstream dummy(filename.c_str());
  return dummy.good();
  }

void assert_present (const string &filename)
  {
  if (file_present(filename)) return;
  throw Message_error ("Error: file " + filename + " does not exist!");
  }

void assert_not_present (const string &filename)
  {
  if (!file_present(filename)) return;
  throw Message_error ("Error: file " + filename + " already exists!");
  }

void remove_file (const string &filename)
  {
  remove (filename.c_str());
  }

string trim (const string &orig)
  {
  string::size_type p1=orig.find_first_not_of(" \t");
  if (p1==string::npos) return "";
  string::size_type p2=orig.find_last_not_of(" \t");
  return orig.substr(p1,p2-p1+1);
  }

template<typename T> string dataToString (const T &x)
  {
  ostringstream strstrm;
  strstrm << x;
  return trim(strstrm.str());
  }

template<> string dataToString (const bool &x)
  { return x ? "T" : "F"; }
template<> string dataToString (const string &x)
  { return trim(x); }
template<> string dataToString (const float &x)
  {
  ostringstream strstrm;
  strstrm << setprecision(8) << x;
  return trim(strstrm.str());
  }
template<> string dataToString (const double &x)
  {
  ostringstream strstrm;
  strstrm << setprecision(16) << x;
  return trim(strstrm.str());
  }

template string dataToString (const int &x);
template string dataToString (const unsigned int &x);
template string dataToString (const long &x);
template string dataToString (const unsigned long long &x);
template string dataToString (const long long &x);
template string dataToString (const unsigned long &x);

string intToString(int x, int width)
  {
  ostringstream strstrm;
  strstrm << setw(width) << setfill('0') << x;
  return trim(strstrm.str());
  }

template<typename T> void stringToData (const string &x, T &value)
  {
  string error = string("conversion error in stringToData<")
               + type2typename<T>()
               +">(\""+x+"\")";
  istringstream strstrm(x);
  strstrm >> value;
  if (!strstrm)
    throw Message_error(error);

  string rest;
  strstrm >> rest;
//  rest=trim(rest);
  if (rest.length()>0) throw Message_error(error);
  }

template<> void stringToData (const string &x, string &value)
  { value = trim(x); }

template<> void stringToData (const string &x, bool &value)
  {
  if ( x=="F" || x=="f" || x=="n" || x=="N" || x=="false" || x==".false."
       || x=="FALSE" || x==".FALSE.")
    value=false;
  else if (x=="T" || x=="t" || x=="y" || x=="Y" || x=="true" || x==".true."
       || x=="TRUE" || x==".TRUE.")
    value=true;
  else
    {
    string error = string("conversion error in stringToData<bool>(\"")+x+"\")";
    throw Message_error (error);
    }
  }

template void stringToData (const string &x, int &value);
template void stringToData (const string &x, long &value);
template void stringToData (const string &x, float &value);
template void stringToData (const string &x, double &value);
template void stringToData (const string &x, unsigned long long &value);
template void stringToData (const string &x, long long &value);
template void stringToData (const string &x, unsigned long &value);
template void stringToData (const string &x, unsigned int &value);

bool equal_nocase (const string &a, const string &b)
  {
  if (a.size()!=b.size()) return false;
  for (unsigned int m=0; m<a.size(); ++m)
    if (tolower(a[m])!=tolower(b[m])) return false;
  return true;
  }

string tolower(const string &input)
  {
  string result=input;
  for (unsigned int m=0; m<result.size(); ++m)
    result[m]=char(tolower(result[m]));
  return result;
  }

#ifdef PLANCK_SILENT

void announce_progress (int, int) {}
void announce_progress (double, double, double) {}
void end_announce_progress () {}

#else

void announce_progress (int now, int total)
  {
  if ((now%(max(total/100,1)))==0)
    cout << "\r " << setw(3) << planck_nint ((now*100.)/total)
         << "% done\r" << flush;
  }

void announce_progress (double now, double last, double total)
  {
  int lastpercent = int((last/total)*100),
      nowpercent  = int(( now/total)*100);
  if (nowpercent>lastpercent)
    cout << "\r " << setw(3) << nowpercent << "% done\r" << flush;
  }

void end_announce_progress ()
  { cout << endl; }

#endif

static void openmp_status()
  {
  if (openmp_enabled())
    {
    cout << "Application was compiled with OpenMP support," << endl;
    if (openmp_max_threads() == 1)
      cout << "but running with one process only." << endl;
    else
      cout << "running with up to " << openmp_max_threads()
           << " processes." << endl;
    }
  else
    cout << "Application was compiled without OpenMP support;" << endl
         << "running in scalar mode." << endl;
  }

void announce (const string &name)
  {
  cout << endl << "+-";
  for (unsigned int m=0; m<name.length(); ++m) cout << "-";
  cout << "-+" << endl;
  cout << "| " << name << " |" << endl;
  cout << "+-";
  for (unsigned int m=0; m<name.length(); ++m) cout << "-";
  cout << "-+" << endl << endl;
  openmp_status();
  cout << endl;
  }

void module_startup (const string &name, int argc, const char **,
  int argc_expected, const string &argv_expected)
  {
  announce (name);
  if (argc==argc_expected) return;
  cerr << "Usage: " << name << " " << argv_expected << endl;
  throw Message_error();
  }

void parse_file (const string &filename, map<string,string> &dict)
  {
  int lineno=0;
  dict.clear();
  ifstream inp(filename.c_str());
  planck_assert(inp.good(),"Could not open parameter file "+filename);
  while (inp)
    {
    string line;
    getline(inp, line);
    ++lineno;
    line=line.substr(0,line.find_first_of("#"));
    line=trim(line);
    if (line.size()>0)
      {
      string::size_type eqpos=line.find("=");
      if (eqpos!=string::npos)
        {
        string key=trim(line.substr(0,eqpos)),
               value=trim(line.substr(eqpos+1,string::npos));
        if (key=="")
          cerr << "Warning: empty key in " << filename << ", line "
               << lineno << endl;
        else
          {
          if (dict.find(key)!=dict.end())
            cerr << "Warning: key " << key << " multiply defined in "
                 << filename << ", line " << lineno << endl;
          dict[key]=value;
          }
        }
      else
        cerr << "Warning: unrecognized format in " << filename << ", line "
             << lineno << ":\n" << line << endl;
      }
    }
  }