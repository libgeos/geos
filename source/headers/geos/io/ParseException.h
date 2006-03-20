/**********************************************************************
 * $Id$
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
 **********************************************************************/

#ifndef GEOS_IO_PARSEEXCEPTION_H
#define GEOS_IO_PARSEEXCEPTION_H

#include <geos/util/GEOSException.h>

namespace geos {
namespace io {

/**
 * \class ParseException io.h geos.h
 * \brief Notifies a parsing error
 */
class ParseException : public util::GEOSException
{
public:
	ParseException();
	ParseException(std::string msg);
	ParseException(std::string msg, std::string var);
	ParseException(std::string msg, double num);
	~ParseException() throw() {};
};

} // namespace io
} // namespace geos

#endif // #ifndef GEOS_IO_PARSEEXCEPTION_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/20 18:18:14  strk
 * io.h header split
 *
 **********************************************************************/
