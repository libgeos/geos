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


#include "../headers/io.h"

namespace geos {

StringTokenizer::StringTokenizer(){
	str="";
	stok="";
	ntok=0.0;
}
StringTokenizer::StringTokenizer(string txt) {
	str=txt;
	stok="";
	ntok=0.0;
}
StringTokenizer::~StringTokenizer(){}

int StringTokenizer::nextToken(){
	string tok="";
	if (str.size()==0)
		return StringTokenizer::TT_EOF;
	switch(str[0]) {
		case '(':
			str=str.substr(1);
			return '(';
		case ')':
			str=str.substr(1);
			return ')';
		case ',':
			str=str.substr(1);
			return ',';
		case '\n':
		case '\r':
		case '\t':
		case ' ':
			string::size_type pos=str.find_first_not_of(" \n\r\t");
			if (pos==string::npos) {
				return StringTokenizer::TT_EOF;
			} else {
				str=str.substr(pos);
				return nextToken();
			}
		}
	string::size_type pos=str.find_first_of("\n\r\t() ,");
	if (pos==string::npos) {
		if (str.size()>0) {
			tok=str.substr(0);
			str="";
		} else {
			return StringTokenizer::TT_EOF;
		}
	} else {
		tok=str.substr(0,pos);
		str=str.substr(pos);
	}
	char *stopstring;
	double dbl=strtod(tok.c_str(),&stopstring);
	if (strcmp(stopstring,"")==0) {
		ntok=dbl;
		stok="";
		return StringTokenizer::TT_NUMBER;
	} else {
		ntok=0.0;
		stok=tok;
		return StringTokenizer::TT_WORD;
	}
}

int StringTokenizer::peekNextToken(){
	string::size_type pos;
	string tok="";
	if (str.size()==0)
		return StringTokenizer::TT_EOF;

	pos=str.find_first_not_of(" \r\n\t");
	if (pos==string::npos) return StringTokenizer::TT_EOF;
	switch(str[pos]) {
		case '(':
			return '(';
		case ')':
			return ')';
		case ',':
			return ',';
	}
	pos=str.find_first_of("\n\r\t() ,");
	if (pos==string::npos) {
		if (str.size()>0) {
			tok=str.substr(0);
		} else {
			return StringTokenizer::TT_EOF;
		}
	} else {
		tok=str.substr(0,pos);
	}
	char *stopstring;
	double dbl=strtod(tok.c_str(),&stopstring);
	if (strcmp(stopstring,"")==0) {
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

