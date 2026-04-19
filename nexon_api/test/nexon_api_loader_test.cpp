#include "nexon_api_loader.h"
#include <iostream>
#include <cassert>

int main() {
    std::string test_api_key = "test_key";
    nexon_api::NexonApiLoader loader(test_api_key);

    std::cout << "Testing NexonApiLoader instantiation..." << std::endl;
    // 실제 API 호출 없이 메서드 시그니처가 유효한지 확인하는 간단한 테스트
    // 실제 테스트는 API 키가 필요하므로 mock이나 skip 처리가 필요함
    
    std::cout << "Nexon API Package Test Success!" << std::endl;
    return 0;
}
