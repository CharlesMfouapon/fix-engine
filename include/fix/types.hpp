#pragma once

#include <cstdint>
#include <string_view>
#include <array>

namespace fix {

// FIX protocol constants
constexpr char SOH = '\x01';          // Field delimiter
constexpr char EQUALS = '=';
constexpr char PIPE = '|';

// Standard FIX tags
enum class Tag : uint32_t {
    BeginString     = 8,
    BodyLength      = 9,
    MsgType         = 35,
    SenderCompID    = 49,
    TargetCompID    = 56,
    MsgSeqNum       = 34,
    SendingTime     = 52,
    Price           = 44,
    OrderQty        = 38,
    Symbol          = 55,
    Side            = 54,
    OrdType         = 40,
    ClOrdID         = 11,
    OrderID         = 37,
    ExecType        = 150,
    LeavesQty       = 151,
    CumQty          = 14,
    AvgPx           = 6,
    CheckSum        = 10,
};

// FIX message types
enum class MsgType : char {
    Heartbeat       = '0',
    TestRequest     = '1',
    ResendRequest   = '2',
    Reject          = '3',
    Logon           = 'A',
    Logout          = '5',
    NewOrderSingle  = 'D',
    ExecutionReport = '8',
    CancelRequest   = 'F',
    CancelReject    = '9',
};

// Order side
enum class Side : char {
    Buy  = '1',
    Sell = '2',
};

// Order types
enum class OrdType : char {
    Market = '1',
    Limit  = '2',
    Stop   = '3',
};

// Execution types
enum class ExecType : char {
    New     = '0',
    Fill    = '1',
    Partial = '2',
    Cancel  = '4',
    Reject  = '8',
};

// Compile-time tag-to-type mapping for zero-overhead parsing
template<Tag T>
struct TagType;

template<> struct TagType<Tag::Price>     { using type = double; };
template<> struct TagType<Tag::OrderQty>  { using type = uint64_t; };
template<> struct TagType<Tag::MsgSeqNum> { using type = uint64_t; };
template<> struct TagType<Tag::Symbol>    { using type = std::string_view; };
template<> struct TagType<Tag::Side>      { using type = Side; };
template<> struct TagType<Tag::OrdType>   { using type = OrdType; };
template<> struct TagType<Tag::ExecType>  { using type = ExecType; };
template<> struct TagType<Tag::ClOrdID>   { using type = std::string_view; };
template<> struct TagType<Tag::SenderCompID> { using type = std::string_view; };
template<> struct TagType<Tag::TargetCompID> { using type = std::string_view; };

} // namespace fix
