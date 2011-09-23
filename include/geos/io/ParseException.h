/**********************************************************************
 * $Id: ParseException.h 2579 2009-06-15 14:03:52Z strk $
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
 * Last port: io/ParseException.java rev. 1.13 (JTS-1.10)
 *
 **********************************************************************/

#ifndef GEOS_IO_PARSEEXCEPTION_H
#define GEOS_IO_PARSEEXCEPTION_H

#include <geos/export.h>

#include <geos/util/GEOSException.h>

namespace geos {
namespace io {

/**
 * \class ParseException io.h geos.h
 * \brief Notifies a parsing error
 */
class GEOS_DLL ParseException : public util::GEOSException
{

public:

	ParseException();

	ParseException(const std::string& msg);

	ParseException(const std::string& msg, const std::string& var);

	ParseException(const std::string& msg, double num);

	~ParseException() throw() {};

private:
	static std::string stringify(double num);
};

} // namespace io
} // namespace geos

#endif // #ifndef GEOS_IO_PARSEEXCEPTION_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/04/04 08:16:46  strk
 * Changed GEOSException hierarchy to be derived from std::runtime_exception.
 * Removed the GEOSException::toString redundant method (use ::what() instead)
 *
 * Revision 1.1  2006/03/20 18:18:14  strk
 * io.h header split
 *
 **********************************************************************/
