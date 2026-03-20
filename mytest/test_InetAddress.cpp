#include "../InetAddress.h"
#include <iostream>

int main() {
    InetAddress it(8080);
    std::cout << it.toIpPort() << std::endl;
}
