#include "fix/fix_message.hpp"
#include <cstdint>
#include <cstddef>

/// LibFuzzer entry point for parser hardening
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size == 0 || size > 4096) return 0;
    
    // Interpret fuzz input as FIX message
    std::string_view raw(reinterpret_cast<const char*>(data), size);
    fix::FixMessage msg(raw);
    
    // Exercise all parsing paths
    msg.msgType();
    msg.get<fix::Tag::ClOrdID>();
    msg.get<fix::Tag::Symbol>();
    msg.get<fix::Tag::Price>();
    msg.get<fix::Tag::OrderQty>();
    msg.get<fix::Tag::Side>();
    msg.get<fix::Tag::OrdType>();
    msg.get<fix::Tag::MsgSeqNum>();
    msg.get<fix::Tag::SenderCompID>();
    msg.get<fix::Tag::TargetCompID>();
    msg.validateChecksum();
    
    return 0; // Non-zero return indicates bug found
}
