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
 * Revision 1.10  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.9  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.8  2003/11/07 01:23:43  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/util.h>
#include <geos/geom.h>

namespace geos {

void Assert::isTrue(bool assertion) {
	isTrue(assertion, string());
}

void Assert::isTrue(bool assertion, string message) {
	if (!assertion) {
		if (message.empty()) {
			throw  AssertionFailedException();
		} else {
			throw  AssertionFailedException(message);
		}
	}
}

void Assert::equals(const Coordinate& expectedValue, const Coordinate& actualValue){
	equals(expectedValue, actualValue, string());
}

void Assert::equals(const Coordinate& expectedValue, const Coordinate& actualValue, string message){
	if (!(actualValue==expectedValue)) {
		throw  AssertionFailedException("Expected " + expectedValue.toString() + " but encountered "
			+ actualValue.toString() + (!message.empty() ? ": " + message : ""));
	}
}


void Assert::shouldNeverReachHere() {
	shouldNeverReachHere(string());
}

void Assert::shouldNeverReachHere(string message) {
	throw  AssertionFailedException("Should never reach here"
		+ (!message.empty() ? ": " + message : ""));
}

}

