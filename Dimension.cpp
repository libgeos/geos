#include "geom.h"

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
		case FALSE:
			return 'F';
		case TRUE:
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
			//temporary replacement for the exception 
			printf("Unknown dimension value: %i\n",dimensionValue);
			exit(1);
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
			return FALSE;
		case 'f':
			return FALSE;
		case 'T':
			return TRUE;
		case 't':
			return TRUE;
		case '*':
			return DONTCARE;
		case '0':
			return P;
		case '1':
			return L;
		case '2':
			return A;
		default:
			//temporary replacement for the exception 
			printf("Unknown dimension symbol: %c\n",dimensionSymbol);
			exit(1);
	}
}