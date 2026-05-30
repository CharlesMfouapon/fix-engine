#include "fix/fix_session.hpp"
#include <cassert>
#include <iostream>

using namespace fix;

void test_logon_sequence() {
    FixSession session(true); // Acceptor
    
    // Build logon from initiator perspective
    auto logon = session.buildLogon("CLIENT", "SERVER");
    
    // Simulate receiving logon
    FixMessage msg(logon);
    auto result = session.processMessage(msg);
    
    assert(!result.has_value()); // No reject
    assert(session.state() == FixSession::State::LoggedOn);
    assert(session.seqNumIn() == 2);
}

void test_sequence_gap_rejected() {
    FixSession session(true);
    session.buildLogon("C", "S");
    
    // Accept logon with seq 1
    std::string logon = "8=FIX.4.4\x01" "35=A\x01" "49=C\x01" "56=S\x01" "34=1\x01" "52=20240101-00:00:00\x01" "98=0\x01" "108=30\x01" "10=000\x01";
    FixMessage msg1(logon);
    session.processMessage(msg1);
    
    // Send message with wrong sequence (should be 2, sending 5)
    std::string bad_seq = "8=FIX.4.4\x01" "35=0\x01" "34=5\x01" "10=000\x01";
    FixMessage msg2(bad_seq);
    auto reject = session.processMessage(msg2);
    
    assert(reject.has_value()); // Should reject
    assert(reject->find("45=5") != std::string::npos);
}

int main() {
    test_logon_sequence();
    test_sequence_gap_rejected();
    std::cout << "Session tests passed\n";
    return 0;
}
