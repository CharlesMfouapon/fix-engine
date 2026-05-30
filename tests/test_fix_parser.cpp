#include "fix/fix_message.hpp"
#include <cassert>
#include <iostream>
#include <string>

using namespace fix;

static int tests_run = 0;
static int tests_passed = 0;

void test_parse_new_order_single() {
    std::string raw = 
        "8=FIX.4.4\x01"
        "35=D\x01"
        "49=SENDER\x01"
        "56=TARGET\x01"
        "34=1\x01"
        "52=20240101-00:00:00\x01"
        "11=ORD-001\x01"
        "55=AAPL.OQ\x01"
        "54=1\x01"
        "44=150.2500000\x01"
        "38=1000\x01"
        "40=2\x01"
        "10=128\x01";

    FixMessage msg(raw);
    tests_run++;

    assert(msg.msgType() == MsgType::NewOrderSingle);
    assert(msg.get<Tag::ClOrdID>() == "ORD-001");
    assert(msg.get<Tag::Symbol>() == "AAPL.OQ");
    assert(msg.get<Tag::Side>() == Side::Buy);
    assert(msg.get<Tag::OrderQty>() == 1000);
    assert(msg.get<Tag::OrdType>() == OrdType::Limit);
    
    auto price = msg.get<Tag::Price>();
    assert(price.has_value() && *price > 150.0 && *price < 151.0);
    
    tests_passed++;
}

void test_validate_checksum() {
    // Valid message with correct checksum
    std::string valid = "8=FIX.4.4\x01" "9=5\x01" "35=0\x01" "10=164\x01";
    FixMessage msg1(valid);
    tests_run++;
    assert(msg1.validateChecksum());
    tests_passed++;

    // Tampered message
    std::string tampered = "8=FIX.4.4\x01" "9=5\x01" "35=1\x01" "10=164\x01";
    FixMessage msg2(tampered);
    tests_run++;
    assert(!msg2.validateChecksum());
    tests_passed++;
}

void test_missing_field() {
    std::string raw = "8=FIX.4.4\x01" "35=0\x01" "10=064\x01";
    FixMessage msg(raw);
    tests_run++;
    assert(!msg.get<Tag::Symbol>().has_value());
    tests_passed++;
}

void test_build_new_order() {
    auto msg = FixSession::buildNewOrderSingle(
        "CL-001", "EUR/USD", Side::Sell, 1.1050, 1000000
    );
    tests_run++;
    
    FixMessage parsed(msg);
    assert(parsed.get<Tag::ClOrdID>() == "CL-001");
    assert(parsed.get<Tag::Side>() == Side::Sell);
    assert(parsed.get<Tag::OrderQty>() == 1000000);
    tests_passed++;
}

int main() {
    test_parse_new_order_single();
    test_validate_checksum();
    test_missing_field();
    test_build_new_order();
    
    std::cout << tests_passed << "/" << tests_run << " tests passed\n";
    return tests_passed == tests_run ? 0 : 1;
}
