#include "../../headers/indexStrtree.h"
#include "stdio.h"

ItemBoundable::ItemBoundable(void* newBounds,void* newItem){
	bounds=newBounds;
	item=newItem;
}

void* ItemBoundable::getBounds() {
	return bounds;
}

void* ItemBoundable::getItem() {
	return item;
}