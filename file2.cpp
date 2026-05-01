#include <iostream>
using namespace std;

int main() {
    int a = 10;
    int b = 20;
    int sum = a + b;
    cout << "Sum = " << sum << endl;
    cout << "Different message here" << endl;
    return 0;
}

int multiply(int x, int y) {
    return x * y;
}

void printMessage() {
    cout << "This is a message" << endl;
}

void extraFunction() {
    cout << "Extra code added here" << endl;
    int z = 99;
}
