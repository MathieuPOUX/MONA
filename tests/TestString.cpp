#include "Mona/Mona.h"
#include "Mona/Format/String.h"

using namespace std;
using namespace Mona;


int main() {
  

	string test = UpperCase("hElLo WoRld");
	CHECK(test == "HELLO WORLD");

	test = LowerCase("hElLo WoRld");
	CHECK(test == "hello world");

	test = "hello world";
	CHECK(String::replace(test, "hello", "goodbye") == "goodbye");

	// TODO !

	return 0;
}
