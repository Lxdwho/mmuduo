#include "../Logger.h"

using namespace muduo;


int main() {
    int a = 3;
    double b = 6.5;
    char c = '+';
    std::string s = "This is a test DEBUG.";
    LOG_DEBUG("%s | %04d | %.4f | %c", s.c_str(), a, b, c);
}