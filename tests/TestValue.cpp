#include "Mona/Mona.h"
#include "Mona/Util/Value.h"
#include "Mona/Logs/Logs.h"

using namespace std;
using namespace Mona;



int main(int argc, char** argv) {

	Value value;

	value.set({"val1", "val2"});

	int i = 0;
	for (const auto& it : value) {
		CHECK(it.index == i);
		++i;
		CHECK(it == String("val", i));
	}

	map<string, string> yoh({{"key1", "val1"}, {"key2", "val2"}});
	auto itMap = yoh.find("key1");
	itMap->second = "val3";

	//value.set(map<string, string>({{"key1","val1"}, {"key2","val2"}}));
	value.set({"val1","val2"});
	value.set({{"key1","val1"}, {"key2","val2"}});

	Value t1({"val1","val2"});
	Value t2({{"key1","val1"}, {"key2","val2"}});
	value.set({{"key1","val1"}, {"key2",{"key2","val2"}}});

	//salue= {{"happy", true}, {"pi", 3.141}};

	// bool isObject = val.is_object();



	// Value value = Value::binary(std::vector<uint8_t>({ 1,2 }));
	// value.get<std::string>();
	// const auto& it = value.type();

	// // Test a positive number
	// value = 123;
	// {
	// 	uint64_t result;
	// 	CHECK(value.get(result) && result == 123);
	// }
	// CHECK(value.get<uint32_t>() == 123);
	// CHECK(value.get<int32_t>() == 123);
	// CHECK(value.get<int64_t>() == 123);
	// CHECK(value.get<uint64_t>() == 123);
	// CHECK(value.get<std::string>() == "123");
	// CHECK(value.get<std::string>() == "123");
	// CHECK(value.get<std::string>("321") == "123");

	// CHECK(value.get<Packet>() == "123");
	// CHECK(value.get<Packet>("321") == "123");

	return 0;
}
