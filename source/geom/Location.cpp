#include "../headers/geom.h"
#include "stdio.h"

/**
 *  Converts the location value to a location symbol, for example, <code>EXTERIOR => 'e'</code>.
 *
 *@param  locationValue  either EXTERIOR, BOUNDARY, INTERIOR or NULL
 *@return                either 'e', 'b', 'i' or '-'
 */
char Location::toLocationSymbol(int locationValue) {
	switch (locationValue) {
		case EXTERIOR:
			return 'e';
		case BOUNDARY:
			return 'b';
		case INTERIOR:
			return 'i';
		case UNDEF: //NULL
			return '-';
		default:
			char buffer[255];
			sprintf(buffer,"Unknown location value: %i\n",locationValue);
			throw buffer;
	}
}
