#include "../Timestamp.h"

int main() {
    Timestamp ttm = Timestamp::now();
    std::cout << ttm.toString() << std::endl;
}