#include "graph.h"

int Position::opposite(int position){
	if (position==LEFT) return RIGHT;
	if (position==RIGHT) return LEFT;
	return position;
}
