/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_UTIL_GEOSEXCEPTION_H
#define GEOS_UTIL_GEOSEXCEPTION_H

#include <exception>
#include <string>

namespace geos {
namespace util { // geos.util

/**
 * \class GEOSException util.h geos.h
 *
 * \brief Base class for all GEOS exceptions.
 *
 * Exceptions are thrown as pointers to this type.
 * Use toString() to get a readable message.
 */
class GEOSException: public std::exception {

protected:
	std::string txt;
	std::string name;

public:
	virtual void setName(const std::string& nname) { name=nname; }
	virtual void setMessage(const std::string& msg) { txt=msg; }

	GEOSException()
	{
		setName("GEOSException");
		setMessage("unknown error");
	}

	GEOSException(const std::string& msg)
	{
		setName("GEOSException");
		setMessage(msg);
	}

	/// Create an exception of given type containing given message 
	GEOSException(const std::string& nname, const std::string& msg)
	{
		setName(nname);
		setMessage(msg);
	}

	virtual ~GEOSException() throw() {}

	/// Returns exception message
	virtual std::string toString() const;

	/// Implement std::exception.what()
	virtual const char* what() const throw() {
		return toString().c_str();
	}

};

} // namespace geos.util
} // namespace geos

#endif // GEOS_UTIL_GEOSEXCEPTION_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
