/**********************************************************************
 * $Id: StringTokenizer.cpp 2579 2009-06-15 14:03:52Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: ORIGINAL WORK
 *
 **********************************************************************/

#include <geos/io/StringTokenizer.h>

#include <string>
#include <cstdlib>

using namespace std;

namespace geos {
namespace io { // geos.io

/*public*/
StringTokenizer::StringTokenizer(const string &txt)
	:
	str(txt)
{
	stok="";
	ntok=0.0;
	iter=str.begin();
}

/*public*/
int
StringTokenizer::nextToken()
{
	string tok="";
	if (iter==str.end())
		return StringTokenizer::TT_EOF;
	switch(*iter) {
		case '(':
		case ')':
		case ',':
			return *iter++;
		case '\n':
		case '\r':
		case '\t':
		case ' ':
			string::size_type pos=str.find_first_not_of(" \n\r\t",
				iter-str.begin());
			if (pos==string::npos) {
				return StringTokenizer::TT_EOF;
			} else {
				iter=str.begin()+pos;
				return nextToken();
			}
	}
	string::size_type pos=str.find_first_of("\n\r\t() ,",
		iter-str.begin());
	if (pos==string::npos) {
		if (iter!=str.end()) {
			tok.assign(iter,str.end());
			iter=str.end();
		} else {
			return StringTokenizer::TT_EOF;
		}
	} else {
		tok.assign(iter, str.begin()+pos);
		iter=str.begin()+pos;
	}
	char *stopstring;
	double dbl=strtod(tok.c_str(),&stopstring);
	if (*stopstring=='\0') {
		ntok=dbl;
		stok="";
		return StringTokenizer::TT_NUMBER;
	} else {
		ntok=0.0;
		stok=tok;
		return StringTokenizer::TT_WORD;
	}
}

/*public*/
int
StringTokenizer::peekNextToken()
{

	string::size_type pos;
	string tok="";
	if (iter==str.end())
		return StringTokenizer::TT_EOF;

	pos=str.find_first_not_of(" \r\n\t", iter-str.begin());

	if (pos==string::npos) return StringTokenizer::TT_EOF;
	switch(str[pos]) {
		case '(':
		case ')':
		case ',':
			return str[pos];
	}

	// It's either a Number or a Word, let's
	// see when it ends

	pos=str.find_first_of("\n\r\t() ,", iter-str.begin());

	if (pos==string::npos) {
		if (iter!=str.end()) {
			tok.assign(iter,str.end());
		} else {
			return StringTokenizer::TT_EOF;
		}
	} else {
		tok.assign(iter, str.begin()+pos); //str.end());
	}

	char *stopstring;
	double dbl=strtod(tok.c_str(),&stopstring);
	if (*stopstring=='\0') {
		ntok=dbl;
		stok="";
		return StringTokenizer::TT_NUMBER;
	} else {
		ntok=0.0;
		stok=tok;
		return StringTokenizer::TT_WORD;
	}
}

/*public*/
double
StringTokenizer::getNVal()
{
	return ntok;
}

/*public*/
string
StringTokenizer::getSVal()
{
	return stok;
}

} // namespace geos.io
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.16  2006/04/26 16:35:40  strk
 * Had WKTReader accept correct form for MultiPoint
 *
 **********************************************************************/
