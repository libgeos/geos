/**********************************************************************
 * $Id: Writer.h 3255 2011-03-01 17:56:10Z mloskot $
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
 * Last port: ORIGINAL WORK to be used like java.io.Writer
 *
 **********************************************************************/

#ifndef GEOS_IO_WRITER_H
#define GEOS_IO_WRITER_H

#include <geos/export.h>

#include <string>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

namespace geos {
namespace io {

class GEOS_DLL Writer {
public:
	Writer();
	~Writer();
	void write(std::string txt);
	std::string toString();
private:
	std::string str;
};

} // namespace geos::io
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // #ifndef GEOS_IO_WRITER_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/20 18:18:15  strk
 * io.h header split
 *
 **********************************************************************/
