#include <iostream>
#include "Regex.h"
//#include "noRegex.h"
using namespace std;

int main() {
    string file = "code.txt";
    runWithRegex(file);
    //runWithoutRegex(file);

    return 0;
}
