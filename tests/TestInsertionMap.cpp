#include "Mona/Mona.h"
#include "Mona/Util/InsertionMap.h"
#include "Mona/Logs/Logs.h"

using namespace std;
using namespace Mona;

int main(int argc, char** argv) {
    InsertionMap<string, string> map;

    // Test basic insertions and iteration
    map["hello"] = "world";
    map["goodbye"] = "everyone";
    CHECK(!map.empty());
	CHECK(map.size() == 2);
	CHECK(map.begin()->first == "hello");
    CHECK(map.rbegin()->first == "goodbye");

	// Test at / exception
	CHECK(map.at("hello") == "world");
    bool thrown = false;
    try {
        map.at("missing");
    } catch (const std::out_of_range&) {
        thrown = true;
    }
    CHECK(thrown);

    // Test insert
    auto res = map.insert({"apple", "fruit"});
    CHECK(res.second == true);
    CHECK(map.size() == 3);

    // Test insert existing key
    res = map.insert({"apple", "newfruit"});
    CHECK(res.second == false);
    CHECK(map.at("apple") == "fruit");

    // Test insert_or_assign
    auto res2 = map.insert_or_assign("banana", "yellow");
    CHECK(res2.second == true);
    CHECK(map.size() == 4);
    CHECK(map.at("banana") == "yellow");

    // assign existing
    res2 = map.insert_or_assign("banana", "green");
    CHECK(res2.second == false);
    CHECK(map.at("banana") == "green");

    // Test emplace
    auto emp_res = map.emplace("cherry", "red");
    CHECK(emp_res.second == true);
    CHECK(map.size() == 5);
    CHECK(map.at("cherry") == "red");

    // Emplace existing key
    emp_res = map.emplace("cherry", "darkred");
    CHECK(emp_res.second == false);
    CHECK(map.at("cherry") == "red");

    // Test emplace_hint (ignoring the hint)
    auto emp_hint_it = map.emplace_hint(map.begin(), "date", "brown");
    CHECK(emp_hint_it->first == "date");
    CHECK(map.size() == 6);

    // Test find
    CHECK(map.find("banana") != map.end());
    CHECK(map.find("not_there") == map.end());

    // Test bounds
    // Current keys: hello, goodbye, apple, banana, cherry, date (in insertion order)
    // In map order (by key): apple, banana, cherry, date, goodbye, hello (sorted)
    CHECK(map.lower_bound("banana")->first == "banana");
    CHECK(map.upper_bound("banana")->first == "cherry");

    // key_comp / value_comp
    {
        auto kc = map.key_comp();
        CHECK(kc("apple", "banana"));
        CHECK(!kc("banana", "apple"));

        auto vc = map.value_comp();
        auto a = *map.find("apple");
        auto b = *map.find("banana");
        CHECK(vc(a, b));
        CHECK(!vc(b, a));
    }

    // Test erase by key
    map.erase("apple");
    CHECK(map.size() == 5);
    CHECK(map.find("apple") == map.end());

    // Erase by iterator
    auto it = map.find("hello");
    CHECK(it != map.end());
    it = map.erase(it);
    CHECK(map.size() == 4);
    CHECK(it->first == "goodbye");

    // swap
    InsertionMap<string, string> other;
    other["x"] = "1";
    other["y"] = "2";

    map.swap(other);
    CHECK(map.size() == 2);
    CHECK(other.size() == 4);

    // map has x,y
    CHECK(map.begin()->first == "x");
    CHECK((++map.begin())->first == "y");

    // other has goodbye, banana, cherry, date
    CHECK(other.find("goodbye") != other.end());
    CHECK(other.find("banana") != other.end());

    // clear
    other.clear();
    CHECK(other.empty());
    CHECK(other.size() == 0);

    return 0;
}
