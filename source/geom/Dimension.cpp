#include "../headers/geom.h"
#include "stdio.h"

namespace geos {

/**
 *  Converts the dimension value to a dimension symbol, for example, <code>TRUE => 'T'</code>.
 *
 *@param  dimensionValue  a number that can be stored in the <code>IntersectionMatrix</code>.
 *			Possible values are <code>{TRUE, FALSE, DONTCARE, 0, 1, 2}</code>.
 *@return   a character for use in the string representation of
 *      an <code>IntersectionMatrix</code>. Possible values are <code>{T, F, * , 0, 1, 2}</code>.
 */
char Dimension::toDimensionSymbol(int dimensionValue) {
	switch (dimensionValue) {
		case False:
			return 'F';
		case True:
			return 'T';
		case DONTCARE:
			return '*';
		case P:
			return '0';
		case L:
			return '1';
		case A:
			return '2';
		default:
			char buffer[255];
			sprintf(buffer,"Unknown dimension value: %i\n",dimensionValue);
			throw new IllegalArgumentException(buffer);
	}
}

/**
 *  Converts the dimension symbol to a dimension value, for example, <code>'*' => DONTCARE</code>.
 *
 *@param  dimensionSymbol  a character for use in the string representation of
 *      an <code>IntersectionMatrix</code>. Possible values are <code>{T, F, * , 0, 1, 2}</code>.
 *@return       a number that can be stored in the <code>IntersectionMatrix</code>.
 *				Possible values are <code>{TRUE, FALSE, DONTCARE, 0, 1, 2}</code>.
 */
int Dimension::toDimensionValue(char dimensionSymbol) {
	switch (dimensionSymbol) {
		case 'F':
		case 'f':
			return False;
		case 'T':
		case 't':
			return True;
		case '*':
			return DONTCARE;
		case '0':
			return P;
		case '1':
			return L;
		case '2':
			return A;
		default:
			char buffer[255];
			sprintf(buffer,"Unknown dimension symbol: %c\n",dimensionSymbol);
			throw new IllegalArgumentException(buffer);
	}
}
}