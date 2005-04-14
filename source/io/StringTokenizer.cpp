/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.12  2005/04/14 11:49:02  strk
 * Applied slightly modified patch by Cheng Shan to speedup WKT parsing.
 *
 * Revision 1.11  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.10  2004/05/17 12:37:50  strk
 * Added carriage returns and tabs in set of blanks chars
 *
 * Revision 1.9  2004/05/07 14:15:08  strk
 * fixed peekNextToken to avoid incrementing string pointer
 *
 * Revision 1.8  2004/03/18 10:42:44  ybychkov
 * "IO" and "Util" upgraded to JTS 1.4
 * "Geometry" partially upgraded.
 *
 * Revision 1.7  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/io.h>

namespace geos {

//StringTokenizer::StringTokenizer(){
	//str="";
	//stok="";
	//ntok=0.0;
//}
StringTokenizer::StringTokenizer(const string &txt): str(txt) {
	//str=txt;
	stok="";
	ntok=0.0;
	iter=str.begin();
}

//StringTokenizer::~StringTokenizer(){}

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
	pos=str.find_first_of("\n\r\t() ,", iter-str.begin());
	if (pos==string::npos) {
		if (iter!=str.end()) {
			tok.assign(iter,str.end());
		} else {
			return StringTokenizer::TT_EOF;
		}
	} else {
		tok.assign(iter,str.end());
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

double StringTokenizer::getNVal(){
	return ntok;
}

string StringTokenizer::getSVal(){
	return stok;
}
}

