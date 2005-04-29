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
 **********************************************************************/

#include <geos/geom.h>
//#include <stdio.h>

namespace geos {

//DefaultCoordinateSequence::DefaultCoordinateSequence(const CoordinateSequence *c){
//	vect=new vector<Coordinate>();
//	int size=c->getSize();
//	for(int i=0; i<size; i++) {
//		vect->push_back(c->getAt(i));
//	}
//}

DefaultCoordinateSequence::DefaultCoordinateSequence() {
	vect=new vector<Coordinate>();
}

DefaultCoordinateSequence::DefaultCoordinateSequence(int n) {
	vect=new vector<Coordinate>(n);
}

//DefaultCoordinateSequence::DefaultCoordinateSequence(const Coordinate& c) {
//	vect=new vector<Coordinate>(1,c);
//}

DefaultCoordinateSequence::DefaultCoordinateSequence(vector<Coordinate> *coords)
{
	if ( ! coords ) vect = new vector<Coordinate>();
	else vect=coords;
}

DefaultCoordinateSequence::DefaultCoordinateSequence(const DefaultCoordinateSequence &c) {
	vect=new vector<Coordinate>(*(c.vect));
}

CoordinateSequence *
DefaultCoordinateSequence::clone() const {
	return new DefaultCoordinateSequence(*this);
}

void DefaultCoordinateSequence::setPoints(const vector<Coordinate> &v) {
	//vect->swap(v);
	delete vect;
	vect=new vector<Coordinate>(v);
}

const vector<Coordinate>*
DefaultCoordinateSequence::toVector() const
{
	return vect; //new vector<Coordinate>(vect->begin(),vect->end());
}

bool DefaultCoordinateSequence::isEmpty() const {
	return vect->empty();
}

void DefaultCoordinateSequence::add(const Coordinate& c){
	vect->push_back(c);
}

int DefaultCoordinateSequence::getSize() const {
	return (int) vect->size();
}

const Coordinate& DefaultCoordinateSequence::getAt(int pos) const {
//	if (pos>=0 && pos<=vect->size()-1) 
		return (*vect)[pos];
//	else
//		cerr<<"DefaultCoordinateSequence exception: can't retrieve element\n";
}

void DefaultCoordinateSequence::setAt(const Coordinate& c, int pos){
//	if (pos>=0 && pos<=vect->size()-1) 
		(*vect)[pos]=c;
//	else
//		throw "DefaultCoordinateSequence exception: can't change element\n";
}
void DefaultCoordinateSequence::deleteAt(int pos){
//	if (pos>=0 && pos<=vect->size()-1) 
		vect->erase(vect->begin()+pos);
//	else
//		throw "DefaultCoordinateSequence exception: can't remove element\n";
}

string DefaultCoordinateSequence::toString() const {
	string result("");
	if (getSize()>0) {
		//char buffer[100];
		for (unsigned int i=0; i<vect->size(); i++) {
			Coordinate& c=(*vect)[i];
			//sprintf(buffer,"(%g,%g,%g) ",c.x,c.y,c.z);
			//result.append(buffer);
			result.append(c.toString());
		}
		result.append("");
	}
	return result;
}

DefaultCoordinateSequence::~DefaultCoordinateSequence() {
	delete vect;
}

void
DefaultCoordinateSequence::expandEnvelope(Envelope &env) const
{
	int size = (int)vect->size();
	for (int i=0; i<size; i++) env.expandToInclude((*vect)[i]);
}

double
DefaultCoordinateSequence::getOrdinate(int index, int ordinateIndex) const
{

#if PARANOIA_LEVEL > 0
	if ( index < 0 || index > vect->size() ) return DoubleNotANumber;
#endif

	switch (ordinateIndex)
	{
		case CoordinateSequence::X:
			return (*vect)[index].x;
		case CoordinateSequence::Y:
			return (*vect)[index].y;
		case CoordinateSequence::Z:
			return (*vect)[index].z;
		default:
			return DoubleNotANumber;
	}
}

void
DefaultCoordinateSequence::setOrdinate(int index, int ordinateIndex,
	double value)
{

#if PARANOIA_LEVEL > 0
	if ( index < 0 || index > vect->size() ) return;
#endif

	switch (ordinateIndex)
	{
		case CoordinateSequence::X:
			(*vect)[index].x = value;
		case CoordinateSequence::Y:
			(*vect)[index].y = value;
		case CoordinateSequence::Z:
			(*vect)[index].z = value;
		default:
			return;
	}
}


} //namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.5  2005/04/29 11:52:40  strk
 * Added new JTS interfaces for CoordinateSequence and factories,
 * removed example implementations to reduce maintainance costs.
 * Added first implementation of WKBWriter, made ByteOrderDataInStream
 * a template class.
 *
 * Revision 1.4  2005/01/28 07:58:04  strk
 * removed sprintf usage, ad ::toString call Coordinate::toString
 *
 * Revision 1.3  2004/12/03 22:52:56  strk
 * enforced const return of CoordinateSequence::toVector() method to derivate classes.
 *
 * Revision 1.2  2004/11/23 16:22:49  strk
 * Added ElevationMatrix class and components to do post-processing draping of overlayed geometries.
 *
 * Revision 1.1  2004/07/08 19:38:56  strk
 * renamed from *List* equivalents
 *
 **********************************************************************/

