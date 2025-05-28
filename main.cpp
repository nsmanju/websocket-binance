#include "bina_cpp_websock.h" // Custom header for Binance WebSocket handling (user-defined)
#include <ixwebsocket/IXWebSocket.h> // ixWebSocket library for WebSocket communication
#include <iostream> // For input/output streams
#include <thread>   // For multithreading support
#include <csignal>  // For signal handling (e.g., SIGINT)

using namespace std;

int main() {
    // Register signal handler for SIGINT (Ctrl+C)
    signal(SIGINT, handle_sigint);

    // List of cryptocurrency trading pairs to subscribe to
    vector<string> assets = {"btcusdt", "ethusdt", "bnbusdt", "xrpusdt", "ltcusdt"};
    // Append the kline (candlestick) stream suffix for 1-minute intervals
    for (auto& asset : assets) {
        asset += "@kline_1m";
    }

    // Build the stream path for the WebSocket URL
    string asset_stream = assets[0];
    for (size_t i = 1; i < assets.size(); ++i) {
        asset_stream += "/" + assets[i];
    }

    // Construct the full Binance WebSocket URL with all streams
    string socket_url = "wss://stream.binance.com:9443/stream?streams=" + asset_stream;

    cout << "Connecting to Binance WebSocket at:\n" << socket_url << endl;

    // Create and configure the WebSocket client
    ix::WebSocket ws;
    ws.setUrl(socket_url); // Set the WebSocket endpoint
    ws.setOnMessageCallback(onMessage); // Set the callback for incoming messages

    ws.start(); // Start the WebSocket connection
    cout << "[Connected] WebSocket connection established.\n";

    // Wait for 3 seconds to ensure connection is established
    this_thread::sleep_for(chrono::seconds(3));

    // Start two threads: one for processing data, one for interactive console input
    thread processor(processData);
    thread console(interactiveConsole);

    // Wait for both threads to finish
    processor.join();
    console.join();

    // Stop the WebSocket connection
    ws.stop();
    cout << "[Disconnected] WebSocket closed.\n";

    return 0;
}
