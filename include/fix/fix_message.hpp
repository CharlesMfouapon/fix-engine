#pragma once

#include "types.hpp"
#include <string_view>
#include <optional>
#include <cstring>
#include <charconv>

namespace fix {

/// A zero-copy view into a raw FIX message buffer.
/// Never allocates. All parsing happens on-demand.
class FixMessage {
public:
    /// Constructs from raw buffer (must remain valid for lifetime)
    explicit FixMessage(std::string_view raw) noexcept
        : raw_(raw) {}

    /// Returns raw message bytes
    [[nodiscard]] std::string_view raw() const noexcept { return raw_; }

    /// Extracts a field value by tag. No allocation.
    template<Tag T>
    [[nodiscard]] std::optional<typename TagType<T>::type> get() const noexcept {
        const char* tag_start = findTag(T);
        if (!tag_start) return std::nullopt;
        return parseValue<T>(tag_start);
    }

    /// Validates checksum
    [[nodiscard]] bool validateChecksum() const noexcept {
        if (raw_.size() < 7) return false;
        // Checksum is last 3 digits before trailing SOH
        auto sum_pos = raw_.find("10=");
        if (sum_pos == std::string_view::npos) return false;
        
        uint32_t sum = 0;
        for (size_t i = 0; i < sum_pos; ++i) {
            sum += static_cast<uint8_t>(raw_[i]);
        }
        sum %= 256;
        
        uint32_t msg_sum = 0;
        auto val = raw_.substr(sum_pos + 3, 3);
        std::from_chars(val.data(), val.data() + val.size(), msg_sum);
        
        return sum == msg_sum;
    }

    /// Extracts message type
    [[nodiscard]] std::optional<MsgType> msgType() const noexcept {
        auto val = get<Tag::MsgType>();
        if (!val) return std::nullopt;
        return val;
    }

private:
    std::string_view raw_;

    [[nodiscard]] const char* findTag(Tag tag) const noexcept {
        // Build search pattern: SOH + tag + '='
        char pattern[32];
        int len = snprintf(pattern, sizeof(pattern), "%c%u=", SOH, static_cast<uint32_t>(tag));
        
        // Search from beginning (tag 8 is at start with no SOH prefix)
        if (tag == Tag::BeginString && raw_.starts_with("8=")) {
            return raw_.data() + 2;
        }
        
        auto pos = raw_.find(pattern, 1); // Skip first char in case of tag 8
        if (pos == std::string_view::npos) return nullptr;
        return raw_.data() + pos + len;
    }

    template<Tag T>
    static std::optional<typename TagType<T>::type> parseValue(const char* pos) noexcept {
        if constexpr (std::is_same_v<typename TagType<T>::type, double>) {
            char* end = nullptr;
            double val = strtod(pos, &end);
            if (end == pos) return std::nullopt;
            return val;
        } else if constexpr (std::is_same_v<typename TagType<T>::type, uint64_t>) {
            uint64_t val = 0;
            auto result = std::from_chars(pos, pos + 20, val);
            if (result.ec != std::errc{}) return std::nullopt;
            return val;
        } else if constexpr (std::is_same_v<typename TagType<T>::type, std::string_view>) {
            const char* end = strchr(pos, SOH);
            if (!end) end = pos + strlen(pos);
            return std::string_view(pos, static_cast<size_t>(end - pos));
        } else if constexpr (std::is_same_v<typename TagType<T>::type, Side>) {
            if (*pos == '1') return Side::Buy;
            if (*pos == '2') return Side::Sell;
            return std::nullopt;
        } else if constexpr (std::is_same_v<typename TagType<T>::type, OrdType>) {
            return static_cast<OrdType>(*pos);
        } else if constexpr (std::is_same_v<typename TagType<T>::type, ExecType>) {
            return static_cast<ExecType>(*pos);
        }
        return std::nullopt;
    }
};

} // namespace fix
