#include "util.h"
#include "geom.h"

void Assert::isTrue(bool assertion) {
	isTrue(assertion, string());
}

void Assert::isTrue(bool assertion, string message) {
	if (!assertion) {
		if (message.empty()) {
			throw AssertionFailedException();
		} else {
			throw AssertionFailedException(message);
		}
	}
}

void Assert::equals(Coordinate& expectedValue, Coordinate& actualValue){
	equals(expectedValue, actualValue, string());
}

void Assert::equals(Coordinate& expectedValue, Coordinate& actualValue, string message){
	if (!(actualValue==expectedValue)) {
		throw AssertionFailedException("Expected " + expectedValue.toString() + " but encountered "
			+ actualValue.toString() + (!message.empty() ? ": " + message : ""));
	}
}


void Assert::shouldNeverReachHere() {
	shouldNeverReachHere(string());
}

void Assert::shouldNeverReachHere(string message) {
	throw AssertionFailedException("Should never reach here"
		+ (!message.empty() ? ": " + message : ""));
}

