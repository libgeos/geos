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
		case '\n':
		case '\r':
			str=str.substr(1);
			return nextToken();
		case '(':
			str=str.substr(1);
			return '(';
		case ')':
			str=str.substr(1);
			return ')';
		case ',':
			str=str.substr(1);
			return ',';
		case ' ':
			string::size_type pos=str.find_first_not_of(" ");
			if (pos==string::npos) {
				return StringTokenizer::TT_EOF;
			} else {
				str=str.substr(pos);
				return nextToken();
			}
		}
	string::size_type pos=str.find_first_of("\n() ,");
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

double StringTokenizer::getNVal(){
	return ntok;
}

string StringTokenizer::getSVal(){
	return stok;
}
}

