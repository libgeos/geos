#include "../headers/util.h"
#include "../headers/geom.h"

void Assert::isTrue(bool assertion) {
	isTrue(assertion, string());
}

void Assert::isTrue(bool assertion, string message) {
	if (!assertion) {
		if (message.empty()) {
			throw new AssertionFailedException();
		} else {
			throw new AssertionFailedException(message);
		}
	}
}

void Assert::equals(Coordinate& expectedValue, Coordinate& actualValue){
	equals(expectedValue, actualValue, string());
}

void Assert::equals(Coordinate& expectedValue, Coordinate& actualValue, string message){
	if (!(actualValue==expectedValue)) {
		throw new AssertionFailedException("Expected " + expectedValue.toString() + " but encountered "
			+ actualValue.toString() + (!message.empty() ? ": " + message : ""));
	}
}


void Assert::shouldNeverReachHere() {
	shouldNeverReachHere(string());
}

void Assert::shouldNeverReachHere(string message) {
	throw new AssertionFailedException("Should never reach here"
		+ (!message.empty() ? ": " + message : ""));
}

