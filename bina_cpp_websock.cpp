#include "bina_cpp_websock.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <thread>
#include <csignal>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;
using WebSocketMessagePtr = ix::WebSocketMessagePtr;

// Global synchronization primitives and shared state
mutex mtx;                        // Mutex for thread-safe access to shared data
condition_variable cv;            // Condition variable for signaling between threads
queue<string> message_queue;      // Queue to store incoming WebSocket messages
bool data_ready = false;          // Flag indicating new data is available
bool stop_signal = false;         // Flag to signal program termination

// Signal handler for Ctrl+C (SIGINT)
void handle_sigint(int) {
    cout << "\nCtrl+C received, stopping..." << endl;
    {
        unique_lock<mutex> lock(mtx);
        stop_signal = true;       // Set stop flag
        data_ready = true;        // Wake up waiting threads
    }
    cv.notify_all();              // Notify all waiting threads
}

// WebSocket message handler
void onMessage(const WebSocketMessagePtr& msg) {
    if (msg->type == ix::WebSocketMessageType::Message) {
        // Received a normal message
        cout << "[onMessage] Received message of size: " << msg->str.size() << endl;
        unique_lock<mutex> lock(mtx);
        message_queue.push(msg->str); // Add message to queue
        data_ready = true;            // Indicate new data is available
        cv.notify_one();              // Notify one waiting thread
    } else if (msg->type == ix::WebSocketMessageType::Error) {
        // Received an error message
        cerr << "[WebSocket Error] Reason: " << msg->errorInfo.reason << endl;
    }
}

// Parse a raw JSON message string into a KlineData struct
KlineData parseMessage(const string& raw_msg) {
    auto message = json::parse(raw_msg); // Parse JSON
    auto data = message["data"];
    auto kline = data["k"];

    string symbol = data["s"];
    double close_price = stod(kline["c"].get<string>());
    time_t timestamp = data["E"].get<time_t>() / 1000; // Convert ms to seconds

    return {symbol, close_price, timestamp};
}

// Thread function to process incoming messages and write to CSV
void processData() {
    vector<KlineData> collected; // Store all parsed kline data
    cout << "Collecting kline messages (Ctrl+C to stop)...\n";

    while (true) {
        unique_lock<mutex> lock(mtx);
        // Wait for new data or stop signal, with timeout
        bool notified = cv.wait_for(lock, chrono::milliseconds(100), [] { return data_ready || stop_signal; });

        if (stop_signal) {
            cout << "Stop signal received in processData.\n";
            break; // Exit loop on stop signal
        }

        if (!notified) continue; // Timeout, loop again

        // Process all messages in the queue
        while (!message_queue.empty()) {
            string msg = message_queue.front();
            message_queue.pop();
            try {
                auto parsed = parseMessage(msg);
                collected.push_back(parsed);

                // Print parsed data to console
                char buffer[64];
                strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&parsed.timestamp));
                cout << "Timestamp: " << buffer << ", Symbol: " << parsed.symbol << ", Close Price: " << parsed.close_price << endl;

            } catch (const exception& e) {
                cerr << "Error parsing message: " << e.what() << endl;
            }
        }

        data_ready = false; // Reset data flag
    }

    // Write collected data to CSV file
    ofstream csv_file("kline_output.csv");
    if (!csv_file) {
        cerr << "Failed to create CSV file.\n";
        return;
    }

    csv_file << "timestamp,symbol,close_price\n";
    for (const auto& data : collected) {
        char buffer[64];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&data.timestamp));
        csv_file << buffer << "," << data.symbol << "," << data.close_price << "\n";
    }

    cout << "CSV file 'kline_output.csv' written successfully.\n";
}

// Simple interactive developer console to invoke functions by name
// This function provides a command-line interface for developers to call
// specific functions at runtime by typing their names.
void interactiveConsole() {
    // Table of available functions: each entry maps a function name to a callable
    FunctionEntry function_table[] = {
        {"handle_sigint", []() { handle_sigint(SIGINT); }}, // Simulate Ctrl+C
        {"processData", processData}                        // Start processing data
    };

    constexpr size_t function_table_size = sizeof(function_table) / sizeof(FunctionEntry);

    // Print available functions to the user
    cout << "\n[Developer Console] Type a function name to invoke (or 'exit' to quit):\n";
    for (size_t i = 0; i < function_table_size; ++i) {
        cout << "  - " << function_table[i].name << "\n";
    }

    // Main input loop
    while (true) {
        // If a stop signal is received (e.g., Ctrl+C), exit the console
        if (stop_signal) {
            cout << "[Console] Received stop signal. Exiting console.\n";
            break;
        }

        cout << "> ";
        string input;
        getline(cin, input); // Read user input

        if (input == "exit") break; // Exit on "exit" command

        bool found = false;
        // Search for the function by name and invoke it if found
        for (size_t i = 0; i < function_table_size; ++i) {
            if (input == function_table[i].name) {
                cout << "[Console] Invoking: " << input << "()\n";
                function_table[i].func_ptr(); // Call the function
                found = true;
                break;
            }
        }

        // Inform the user if the function name was not found
        if (!found) {
            cout << "Function not found: " << input << "\n";
        }
    }
}
