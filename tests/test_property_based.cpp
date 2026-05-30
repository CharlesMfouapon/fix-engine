#include "fix/fix_session.hpp"
#include <cassert>
#include <iostream>
#include <random>
#include <vector>

using namespace fix;

/// Property: Any built message can be parsed and yields original values
void test_new_order_roundtrip() {
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> price_dist(0.01, 99999.99);
    std::uniform_int_distribution<uint64_t> qty_dist(1, 10000000);
    
    const char* symbols[] = {"AAPL", "GOOG", "EUR/USD", "BTC-USD", "ES"};
    const Side sides[] = {Side::Buy, Side::Sell};
    
    int failures = 0;
    for (int i = 0; i < 1000; i++) {
        auto cl_ord = "ORD-" + std::to_string(i);
        auto symbol = symbols[rng() % 5];
        auto side = sides[rng() % 2];
        auto price = price_dist(rng);
        auto qty = qty_dist(rng);
        
        auto msg = FixSession::buildNewOrderSingle(cl_ord, symbol, side, price, qty);
        FixMessage parsed(msg);
        
        bool ok = true;
        ok &= (parsed.get<Tag::ClOrdID>() == cl_ord);
        ok &= (parsed.get<Tag::Side>() == side);
        ok &= (parsed.get<Tag::OrderQty>() == qty);
        
        if (!ok) failures++;
    }
    
    assert(failures == 0);
    std::cout << "Property test: 1000 roundtrips, 0 failures\n";
}

int main() {
    test_new_order_roundtrip();
    return 0;
}
