#include <iostream>
#include <cstdint>
#include <map>
#include <random>
#include <string>

using namespace std;

class someObject {
public:
    uint64_t *id;
    string name;
public:
    someObject(uint64_t* i, string n) {
        id = i;
        name = n;
    }
};

map<someObject*, string> objectMap;


int main(int argc, char* argv[]) {
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(1,6);
    int dice_roll = distribution(generator);
    // create some objects
    for (int i = 1; i < 10; i++) {
        uint64_t* ptr = new uint64_t[10000*dice_roll];  
        ptr[0] = i;
        string objectName = "someName_" + to_string(i);
        someObject *s = new someObject(ptr,objectName);
        string objectValue = "someValue_" + to_string(i);
        objectMap.insert(std::make_pair(s,objectValue));
    }
    // print those objects
    for(auto B = objectMap.begin(), E = objectMap.end();
            B != E; B++) {
        // cout << B->first->id << ",";
        cout << B->first->name << ": ";
        cout << B->second << "\n";
    }

}
