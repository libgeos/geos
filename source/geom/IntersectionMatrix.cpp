#include "geom.h"
#include "stdio.h"

IntersectionMatrix::IntersectionMatrix(){
	//matrix = new int[3][3];
	setAll(Dimension::False);
}

IntersectionMatrix::IntersectionMatrix(string elements){
	//matrix = new int[3][3];
	setAll(Dimension::False);
    set(elements);
}

IntersectionMatrix::IntersectionMatrix(const IntersectionMatrix &im){
	//matrix = new int[3][3];
	setAll(Dimension::False);
	matrix[Location::INTERIOR][Location::INTERIOR] = im.matrix[Location::INTERIOR][Location::INTERIOR];
	matrix[Location::INTERIOR][Location::BOUNDARY] = im.matrix[Location::INTERIOR][Location::BOUNDARY];
	matrix[Location::INTERIOR][Location::EXTERIOR] = im.matrix[Location::INTERIOR][Location::EXTERIOR];
	matrix[Location::BOUNDARY][Location::INTERIOR] = im.matrix[Location::BOUNDARY][Location::INTERIOR];
	matrix[Location::BOUNDARY][Location::BOUNDARY] = im.matrix[Location::BOUNDARY][Location::BOUNDARY];
	matrix[Location::BOUNDARY][Location::EXTERIOR] = im.matrix[Location::BOUNDARY][Location::EXTERIOR];
	matrix[Location::EXTERIOR][Location::INTERIOR] = im.matrix[Location::EXTERIOR][Location::INTERIOR];
	matrix[Location::EXTERIOR][Location::BOUNDARY] = im.matrix[Location::EXTERIOR][Location::BOUNDARY];
	matrix[Location::EXTERIOR][Location::EXTERIOR] = im.matrix[Location::EXTERIOR][Location::EXTERIOR];
}

IntersectionMatrix::~IntersectionMatrix(){
	//delete[] matrix;
}

bool IntersectionMatrix::matches(int actualDimensionValue, char requiredDimensionSymbol) {
	if (requiredDimensionSymbol=='*') {
		return true;
	}
	if (requiredDimensionSymbol=='T' && (actualDimensionValue >= 0 ||
										 actualDimensionValue==Dimension::True)) {
		return true;
	}
	if (requiredDimensionSymbol=='F' && actualDimensionValue==Dimension::False) {
		return true;
	}
	if (requiredDimensionSymbol=='0' && actualDimensionValue==Dimension::P) {
		return true;
	}
	if (requiredDimensionSymbol=='1' && actualDimensionValue==Dimension::L) {
		return true;
	}
	if (requiredDimensionSymbol=='2' && actualDimensionValue==Dimension::A) {
		return true;
	}
	return false;
}

bool IntersectionMatrix::matches(string actualDimensionSymbols, string requiredDimensionSymbols) {
	IntersectionMatrix* m=new IntersectionMatrix(actualDimensionSymbols);
	bool result=m->matches(requiredDimensionSymbols);
	delete m;
	return result;
}

void IntersectionMatrix::set(int row, int column, int dimensionValue){
	matrix[row][column] = dimensionValue;
}

void IntersectionMatrix::set(string dimensionSymbols) {
	unsigned int limit;
	if ((unsigned int)dimensionSymbols.length()<=9) limit=(unsigned int)dimensionSymbols.length();
	else limit=9;

	for (unsigned int i = 0; i < limit; i++) {
		int row = i / 3;
		int col = i % 3;
		matrix[row][col] = Dimension::toDimensionValue(dimensionSymbols[i]);
	}
}
void IntersectionMatrix::setAtLeast(int row, int column, int minimumDimensionValue) {
	if (matrix[row][column] < minimumDimensionValue) {
		matrix[row][column] = minimumDimensionValue;
	}
}

void IntersectionMatrix::setAtLeastIfValid(int row, int column, int minimumDimensionValue) {
	if (row >= 0 && column >= 0) {
		setAtLeast(row, column, minimumDimensionValue);
	}
}

void IntersectionMatrix::setAtLeast(string minimumDimensionSymbols) {
	unsigned int limit;
	if ((unsigned int)minimumDimensionSymbols.length()<=9)
		limit=(unsigned int)minimumDimensionSymbols.length();
	else limit=9;

	for (unsigned int i = 0; i < limit; i++) {
		int row = i / 3;
		int col = i % 3;
		setAtLeast(row, col, Dimension::toDimensionValue(minimumDimensionSymbols[i]));
	}
}

void IntersectionMatrix::setAll(int dimensionValue) {
	for (int ai = 0; ai < 3; ai++) {
		for (int bi = 0; bi < 3; bi++) {
			matrix[ai][bi] = dimensionValue;
		}
	}
}

int IntersectionMatrix::get(int row, int column) {
	return matrix[row][column];
}

bool IntersectionMatrix::isDisjoint() {
	return matrix[Location::INTERIOR][Location::INTERIOR]==Dimension::False &&
		matrix[Location::INTERIOR][Location::BOUNDARY]==Dimension::False &&
		matrix[Location::BOUNDARY][Location::INTERIOR]==Dimension::False &&
		matrix[Location::BOUNDARY][Location::BOUNDARY]==Dimension::False;
}

bool IntersectionMatrix::isIntersects() {
	return !isDisjoint();
}

bool IntersectionMatrix::isTouches(int dimensionOfGeometryA, int dimensionOfGeometryB) {
	if (dimensionOfGeometryA > dimensionOfGeometryB) {
		//no need to get transpose because pattern matrix is symmetrical
		return isTouches(dimensionOfGeometryB, dimensionOfGeometryA);
	}
	if ((dimensionOfGeometryA==Dimension::A && dimensionOfGeometryB==Dimension::A) ||
		(dimensionOfGeometryA==Dimension::L && dimensionOfGeometryB==Dimension::L) ||
		(dimensionOfGeometryA==Dimension::L && dimensionOfGeometryB==Dimension::A) ||
		(dimensionOfGeometryA==Dimension::P && dimensionOfGeometryB==Dimension::A) ||
		(dimensionOfGeometryA==Dimension::P && dimensionOfGeometryB==Dimension::L)) {
		return matrix[Location::INTERIOR][Location::INTERIOR]==Dimension::False &&
			  (matches(matrix[Location::INTERIOR][Location::BOUNDARY], 'T') ||
			   matches(matrix[Location::BOUNDARY][Location::INTERIOR], 'T') ||
			   matches(matrix[Location::BOUNDARY][Location::BOUNDARY], 'T'));
	}
	return false;
}

bool IntersectionMatrix::isCrosses(int dimensionOfGeometryA, int dimensionOfGeometryB) {
	if ((dimensionOfGeometryA==Dimension::P && dimensionOfGeometryB==Dimension::L) ||
		(dimensionOfGeometryA==Dimension::P && dimensionOfGeometryB==Dimension::A) ||
		(dimensionOfGeometryA==Dimension::L && dimensionOfGeometryB==Dimension::A)) {
		return matches(matrix[Location::INTERIOR][Location::INTERIOR], 'T') &&
			   matches(matrix[Location::INTERIOR][Location::EXTERIOR], 'T');
	}
	if ((dimensionOfGeometryA==Dimension::L && dimensionOfGeometryB==Dimension::P) ||
		(dimensionOfGeometryA==Dimension::A && dimensionOfGeometryB==Dimension::P) ||
		(dimensionOfGeometryA==Dimension::A && dimensionOfGeometryB==Dimension::L)) {
		return matches(matrix[Location::INTERIOR][Location::INTERIOR], 'T') &&
			   matches(matrix[Location::EXTERIOR][Location::INTERIOR], 'T');
	}
	if (dimensionOfGeometryA==Dimension::L && dimensionOfGeometryB==Dimension::L) {
		return matrix[Location::INTERIOR][Location::INTERIOR]==0;
	}
	return false;
}

bool IntersectionMatrix::isWithin() {
	return matches(matrix[Location::INTERIOR][Location::INTERIOR], 'T') &&
		   matrix[Location::INTERIOR][Location::EXTERIOR]==Dimension::False &&
		   matrix[Location::BOUNDARY][Location::EXTERIOR]==Dimension::False;
}

bool IntersectionMatrix::isContains() {
	return matches(matrix[Location::INTERIOR][Location::INTERIOR], 'T') &&
		   matrix[Location::EXTERIOR][Location::INTERIOR]==Dimension::False &&
		   matrix[Location::EXTERIOR][Location::BOUNDARY]==Dimension::False;
}

bool IntersectionMatrix::isEquals(int dimensionOfGeometryA, int dimensionOfGeometryB) {
	if (dimensionOfGeometryA != dimensionOfGeometryB) {
		return false;
	}
	return matches(matrix[Location::INTERIOR][Location::INTERIOR], 'T') &&
		   matrix[Location::EXTERIOR][Location::INTERIOR]==Dimension::False &&
		   matrix[Location::INTERIOR][Location::EXTERIOR]==Dimension::False &&
		   matrix[Location::EXTERIOR][Location::BOUNDARY]==Dimension::False &&
		   matrix[Location::BOUNDARY][Location::EXTERIOR]==Dimension::False;
}

bool IntersectionMatrix::isOverlaps(int dimensionOfGeometryA, int dimensionOfGeometryB) {
	if ((dimensionOfGeometryA==Dimension::P && dimensionOfGeometryB==Dimension::P) ||
		(dimensionOfGeometryA==Dimension::A && dimensionOfGeometryB==Dimension::A)) {
		return matches(matrix[Location::INTERIOR][Location::INTERIOR], 'T') &&
			   matches(matrix[Location::INTERIOR][Location::EXTERIOR], 'T') &&
			   matches(matrix[Location::EXTERIOR][Location::INTERIOR],'T');
	}
	if (dimensionOfGeometryA==Dimension::L && dimensionOfGeometryB==Dimension::L) {
		return matrix[Location::INTERIOR][Location::INTERIOR]==1 &&
			   matches(matrix[Location::INTERIOR][Location::EXTERIOR], 'T') &&
			   matches(matrix[Location::EXTERIOR][Location::INTERIOR], 'T');
	}
	return false;
}

bool IntersectionMatrix::matches(string requiredDimensionSymbols) {
	if (requiredDimensionSymbols.length() != 9) {
		char buffer[255];
		sprintf(buffer,"IllegalArgumentException: Should be length 9: %s\n",requiredDimensionSymbols.c_str());
		throw buffer;
	}
	for (int ai = 0; ai < 3; ai++) {
		for (int bi = 0; bi < 3; bi++) {
			if (!matches(matrix[ai][bi],requiredDimensionSymbols[3*ai+bi])) {
				return false;
			}
		}
	}
	return true;
}

//Not sure
IntersectionMatrix* IntersectionMatrix::transpose() {
	int temp = matrix[1][0];
	matrix[1][0] = matrix[0][1];
	matrix[0][1] = temp;
	temp = matrix[2][0];
	matrix[2][0] = matrix[0][2];
	matrix[0][2] = temp;
	temp = matrix[2][1];
	matrix[2][1] = matrix[1][2];
	matrix[1][2] = temp;
	return this;
}

string IntersectionMatrix::toString() {
	string result("");
	for (int ai = 0; ai < 3; ai++) {
		for (int bi = 0; bi < 3; bi++) {
			result+=Dimension::toDimensionSymbol(matrix[ai][bi]);
		}
	}
	return result;
}
