#include "geom.h"

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
			//Temporary replacement for exception
			printf("Unknown location value: %i\n",locationValue);
			exit(1);
	}
}