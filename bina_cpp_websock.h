#ifndef BINA_CPP_WEBSOCK_H
#define BINA_CPP_WEBSOCK_H

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <ctime>
#include <ixwebsocket/IXWebSocket.h>

/**
 * @file bina_cpp_websock.h
 * @brief Defines data structures for Binance WebSocket Kline data.
 */

/**
 * @struct KlineData
 * @brief Represents a single candlestick (kline) data point from Binance.
 *
 * This structure holds the essential information for a kline, including:
 * - The trading symbol (e.g., "BTCUSDT").
 * - The closing price of the kline interval.
 * - The timestamp associated with the kline (typically in seconds since epoch).
 */
struct KlineData {
    std::string symbol;
    double close_price;
    time_t timestamp;
};

struct FunctionEntry {
    const char* name;
    void (*func_ptr)();
};

// Externally accessible variables
extern std::mutex mtx;
extern std::condition_variable cv;
extern std::queue<std::string> message_queue;
extern bool data_ready;
extern bool stop_signal;

// Function declarations
void handle_sigint(int);
void onMessage(const ix::WebSocketMessagePtr& msg);
KlineData parseMessage(const std::string& raw_msg);
void processData();
void interactiveConsole();

#endif // BINA_CPP_WEBSOCK_H
