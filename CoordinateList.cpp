#include "geom.h"

CoordinateList::CoordinateList() {
	head=new List();
	tail=head;
	current=NULL;
	size=0;
}

CoordinateList::CoordinateList(Coordinate c) {
	head=new List(c);
	tail=head;
	current=NULL;
	size=1;
}

void CoordinateList::reset(){
	current=NULL;
}

vector<Coordinate> CoordinateList::toVector() {
	return vector<Coordinate>();
}

bool CoordinateList::isEmpty() {
	if (size==0) return true;
	else return false;
}

Coordinate CoordinateList::getNext(){
	if (current==NULL) {
		current=head;
	} else {
		current=current->next;
	}
	if (current==NULL) current=tail;
	return current->data;
}

bool CoordinateList::hasNext(){
	if (current==tail) {
		return false;
	} else {
		return true;
	}
}

void CoordinateList::add(Coordinate c){
	if (size==0) {
		head->data=Coordinate(c);
	} else {
		tail->next=new List(c);
		(tail->next)->prev=tail;
		tail=tail->next;
	}
	size++;
}

int CoordinateList::getSize(){
	return size;
}

void CoordinateList::moveTo(int pos){
	reset();
	for(int i=0; i<=pos; i++) {
		if (current==NULL) {
			current=head;
		} else {
			current=current->next;
		}
	}
}

Coordinate CoordinateList::getAt(int pos){
	moveTo(pos);
	return get();
}

void CoordinateList::setAt(Coordinate c, int pos){
	moveTo(pos);
	set(c);
}
void CoordinateList::deleteAt(int pos){
	moveTo(pos);
	remove();
}

Coordinate CoordinateList::get(){
	if (current!=NULL) {
		return current->data;
	} else {
		return Coordinate();
	}
}

void CoordinateList::set(Coordinate c){
	if (current!=NULL) {
		current->data=c;
	}
}

void CoordinateList::remove(){
	if (current!=NULL) {
		if (current==head) {
			head=current->next;
			head->prev=NULL;
			delete current;
			current=head;
		} else if (current==tail) {
			tail=current->prev;
			tail->next=NULL;
			delete current;
			current=tail;
		} else {
			(current->prev)->next=current->next;
			(current->next)->prev=current->prev;
			delete current;
			ListPtr tmp=current->next;
			current=tmp;
		}
	size--;
	}
}

CoordinateList::~CoordinateList() {}