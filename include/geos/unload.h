/**********************************************************************
 * $Id: unload.h 2556 2009-06-06 22:22:28Z strk $
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
 * Revision 1.3  2006/03/06 19:43:20  strk
 * Fixed Unload class definition namespace (geos::io)
 *
 * Revision 1.2  2006/03/01 10:39:58  strk
 * ctor and dtor made private and inlined
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.2  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

#include <geos/export.h>

#ifndef GEOS_UNLOAD_H
#define GEOS_UNLOAD_H
//xie add for realse static memory 2003,10,06
namespace geos {
namespace io {

class GEOS_DLL Unload
{
private:
	Unload(void) {}
	~Unload(void) {}
public:
	static void Release();
};

}
}
#endif
