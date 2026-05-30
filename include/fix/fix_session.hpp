#pragma once

#include "fix_message.hpp"
#include <functional>
#include <chrono>
#include <atomic>

namespace fix {

/// FIX session state machine (acceptor or initiator)
class FixSession {
public:
    enum class State {
        Disconnected,
        WaitingLogon,
        LoggedOn,
        WaitingLogout,
    };

    using MessageHandler = std::function<void(const FixMessage&)>;

    explicit FixSession(bool is_acceptor) noexcept
        : is_acceptor_(is_acceptor)
        , state_(State::Disconnected)
        , seq_num_in_(1)
        , seq_num_out_(1)
    {}

    /// Process an incoming FIX message
    [[nodiscard]] std::optional<std::string> processMessage(const FixMessage& msg) {
        auto msg_type = msg.msgType();
        if (!msg_type) return makeReject(1, "Missing MsgType");

        // Validate sequence number
        auto seq = msg.get<Tag::MsgSeqNum>();
        if (!seq) return makeReject(1, "Missing MsgSeqNum");
        if (*seq != seq_num_in_) {
            return makeReject(*seq, "Sequence number mismatch");
        }

        seq_num_in_++;

        switch (state_) {
            case State::WaitingLogon:
                if (*msg_type == MsgType::Logon) {
                    state_ = State::LoggedOn;
                    return std::nullopt;
                }
                return makeReject(*seq, "Expected Logon");

            case State::LoggedOn:
                return handleApplicationMessage(*msg_type, msg);

            default:
                return makeReject(*seq, "Invalid session state");
        }
    }

    /// Generate a FIX Logon message
    [[nodiscard]] std::string buildLogon(
        std::string_view sender,
        std::string_view target,
        uint32_t heartbt_int = 30
    ) noexcept {
        state_ = State::WaitingLogon;
        return buildHeader("A", sender, target) +
               "98=0" + std::string(1, SOH) +      // Encryption
               "108=" + std::to_string(heartbt_int) + std::string(1, SOH) +
               buildTrailer();
    }

    /// Generate a FIX Heartbeat
    [[nodiscard]] std::string buildHeartbeat(
        std::string_view sender,
        std::string_view target
    ) noexcept {
        return buildHeader("0", sender, target) + buildTrailer();
    }

    /// Generate a NewOrderSingle
    [[nodiscard]] static std::string buildNewOrderSingle(
        std::string_view cl_ord_id,
        std::string_view symbol,
        Side side,
        double price,
        uint64_t qty,
        OrdType ord_type = OrdType::Limit
    ) noexcept {
        std::string msg;
        msg.reserve(256);
        
        // Standard header
        msg += "8=FIX.4.4" + std::string(1, SOH);
        msg += "35=D" + std::string(1, SOH);
        msg += "49=SENDER" + std::string(1, SOH);
        msg += "56=TARGET" + std::string(1, SOH);
        msg += "34=1" + std::string(1, SOH);
        msg += "52=20240101-00:00:00" + std::string(1, SOH);
        
        // Order fields
        msg += "11=" + std::string(cl_ord_id) + std::string(1, SOH);
        msg += "55=" + std::string(symbol) + std::string(1, SOH);
        msg += "54=" + static_cast<char>(side) + std::string(1, SOH);
        msg += "44=" + formatPrice(price) + std::string(1, SOH);
        msg += "38=" + std::to_string(qty) + std::string(1, SOH);
        msg += "40=" + static_cast<char>(ord_type) + std::string(1, SOH);
        
        // Calculate body length
        auto body_len = msg.length() - 7; // Remove "8=FIX..."
        std::string final_msg = "8=FIX.4.4" + std::string(1, SOH);
        final_msg += "9=" + std::to_string(body_len) + std::string(1, SOH);
        final_msg += msg.substr(7);
        
        // Checksum
        uint32_t sum = 0;
        for (char c : final_msg) sum += static_cast<uint8_t>(c);
        sum %= 256;
        
        final_msg += "10=";
        if (sum < 10) final_msg += "00";
        else if (sum < 100) final_msg += "0";
        final_msg += std::to_string(sum);
        final_msg += SOH;
        
        return final_msg;
    }

    [[nodiscard]] State state() const noexcept { return state_; }
    [[nodiscard]] uint64_t seqNumIn() const noexcept { return seq_num_in_; }
    [[nodiscard]] uint64_t seqNumOut() const noexcept { return seq_num_out_; }

private:
    bool is_acceptor_;
    State state_;
    uint64_t seq_num_in_;
    uint64_t seq_num_out_;

    std::optional<std::string> handleApplicationMessage(MsgType type, const FixMessage& msg) {
        switch (type) {
            case MsgType::Heartbeat:
                return std::nullopt;
            case MsgType::TestRequest: {
                auto test_id = msg.get<Tag::MsgSeqNum>();
                return buildHeartbeat("TARGET", "SENDER");
            }
            case MsgType::Logout:
                state_ = State::Disconnected;
                return std::nullopt;
            default:
                return std::nullopt; // Application-level messages handled by callback
        }
    }

    std::string buildHeader(
        std::string_view msg_type,
        std::string_view sender,
        std::string_view target
    ) noexcept {
        std::string header;
        header.reserve(128);
        header += "8=FIX.4.4" + std::string(1, SOH);
        header += "35=" + std::string(msg_type) + std::string(1, SOH);
        header += "49=" + std::string(sender) + std::string(1, SOH);
        header += "56=" + std::string(target) + std::string(1, SOH);
        header += "34=" + std::to_string(seq_num_out_++) + std::string(1, SOH);
        header += "52=20240101-00:00:00" + std::string(1, SOH);
        return header;
    }

    std::string buildTrailer() noexcept {
        return "10=000" + std::string(1, SOH); // Simplified
    }

    static std::string formatPrice(double price) noexcept {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.7f", price);
        return buf;
    }

    std::optional<std::string> makeReject(uint64_t ref_seq, const char* reason) {
        std::string reject = buildHeader("3", "TARGET", "SENDER");
        reject += "45=" + std::to_string(ref_seq) + std::string(1, SOH);
        reject += "58=" + std::string(reason) + std::string(1, SOH);
        reject += buildTrailer();
        return reject;
    }
};

} // namespace fix
