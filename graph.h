//#include <iostream>
#include <string>
#include <vector>
//#include <algorithm>
//#include "math.h"
//#include "platform.h"

using namespace std;

class Position {
public:
	static const int ON=0;
	static const int LEFT=1;
	static const int RIGHT=2;
	static int opposite(int position);
};

class TopologyLocation {
public:
	~TopologyLocation(){};
	TopologyLocation(vector<int> location);
	TopologyLocation(int on, int left, int right);
	TopologyLocation(int on);
	TopologyLocation(const TopologyLocation &gl);
	int get(int posIndex);
	bool isNull();
	bool isAnyNull();
	bool isEqualOnSide(TopologyLocation le, int locIndex);
	bool isArea();
	bool isLine();
	void flip();
	void setAllLocations(int locValue);
	void setAllLocationsIfNull(int locValue);
	void setLocation(int locIndex, int locValue);
	void setLocation(int locValue);
	vector<int> getLocations();
	void setLocations(int on, int left, int right);
	void setLocations(TopologyLocation gl);
	bool allPositionsEqual(int loc);
	void merge(TopologyLocation* gl);
	string toString();
protected:
	vector<int> location;
private:
	void init(int size);
};

class Label {
public:
	static Label toLineLabel(Label label);
	Label(int onLoc);
	Label(int geomIndex, int onLoc);
	Label(int onLoc, int leftLoc, int rightLoc);
	Label(const Label &l);
	Label();
	~Label();
	Label(int geomIndex,int onLoc,int leftLoc,int rightLoc);
	Label(int geomIndex,TopologyLocation gl);
	void flip();
	int getLocation(int geomIndex, int posIndex);
	int getLocation(int geomIndex);
	void setLocation(int geomIndex, int posIndex, int location);
	void setLocation(int geomIndex, int location);
	void setAllLocations(int geomIndex, int location);
	void setAllLocationsIfNull(int geomIndex, int location);
	void setAllLocationsIfNull(int location);
	void merge(Label lbl);
	int getGeometryCount();
	bool isNull(int geomIndex);
	bool isAnyNull(int geomIndex);
	bool isArea();
	bool isArea(int geomIndex);
	bool isLine(int geomIndex);
	bool isEqualOnSide(Label lbl, int side);
	bool allPositionsEqual(int geomIndex, int loc);
	void toLine(int geomIndex);
	string toString();
protected:
	TopologyLocation* elt[2];
private:
	void setGeometryLocation(int geomIndex, TopologyLocation* tl);
};

class Depth {
public:
	static int depthAtLocation(int location);
	Depth();
	~Depth();
	int getDepth(int geomIndex,int posIndex);
	void setDepth(int geomIndex,int posIndex,int depthValue);
	int getLocation(int geomIndex,int posIndex);
	void add(int geomIndex,int posIndex,int location);
	bool isNull();
	bool isNull(int geomIndex);
	bool isNull(int geomIndex,int posIndex);
	int getDelta(int geomIndex);
	void normalize();
	void add(Label lbl);
	string toString();
private:
	static const int DEPTHNULL=-1; //Replaces NULL
	int depth[2][3];
};

