#include "fix/fix_message.hpp"
#include "fix/fix_session.hpp"
#include <chrono>
#include <iostream>
#include <vector>

using namespace fix;
using namespace std::chrono;

int main() {
    // Generate test messages
    std::vector<std::string> messages;
    for (int i = 0; i < 10000; i++) {
        messages.push_back(FixSession::buildNewOrderSingle(
            "ORD-" + std::to_string(i),
            "AAPL",
            i % 2 == 0 ? Side::Buy : Side::Sell,
            150.0 + (i % 100),
            100 + (i % 1000)
        ));
    }
    
    // Benchmark parsing
    auto start = high_resolution_clock::now();
    uint64_t total_qty = 0;
    
    for (const auto& raw : messages) {
        FixMessage msg(raw);
        auto qty = msg.get<Tag::OrderQty>();
        if (qty) total_qty += *qty;
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start).count();
    
    double msg_per_sec = (messages.size() * 1'000'000.0) / duration;
    double ns_per_msg = (duration * 1000.0) / messages.size();
    
    std::cout << "Parsed " << messages.size() << " messages\n";
    std::cout << "Total Qty: " << total_qty << "\n";
    std::cout << "Time: " << duration << " μs\n";
    std::cout << "Throughput: " << msg_per_sec << " msgs/sec\n";
    std::cout << "Latency: " << ns_per_msg << " ns/msg\n";
    
    return 0;
}
