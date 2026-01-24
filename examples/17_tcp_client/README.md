# Example 17: TCP Client

This example demonstrates TCP client functionality using the ofxTcpClient addon.

## What This Example Shows

- **TCP Connection**: Connecting to a TCP server
- **Message Sending**: Sending text messages with delimiters
- **Message Receiving**: Receiving responses from the server
- **Connection Status**: Monitoring connection state
- **Auto-reconnect**: Reconnecting on demand
- **Error Handling**: Checking connection errors
- **Non-blocking I/O**: Asynchronous communication using Network.framework

## Prerequisites

Before running this example, you need a TCP server running on `localhost:8080`.

### Option 1: Use netcat
```bash
nc -l 8080
```

### Option 2: Use the tcp_server example
Run the `18_tcp_server` example in a separate instance.

## Features

### Automatic Messages
- The client automatically sends a message every 2 seconds
- Each message is numbered for tracking
- Messages are sent with newline delimiter

### Visual Feedback
- **Green indicator**: Connected to server
- **Red indicator**: Disconnected
- **Animated packets**: Visual representation of data flow
- **Info boxes**: Display connection details and received data

### Interactive Controls
- **Space**: Send a manual message immediately
- **R**: Reconnect to the server
- **D**: Disconnect from the server

## API Functions Used

### ofxTcpClient
- `setup(host, port)` - Connect to server
- `close()` - Close connection
- `isConnected()` - Check connection status
- `sendMessage(message, delimiter)` - Send message with delimiter
- `receiveMessage(delimiter)` - Receive message until delimiter
- `getRemoteIP()` - Get server IP address
- `getRemotePort()` - Get server port
- `getError()` - Get last error message

### Network.framework Integration
The implementation uses Apple's Network.framework for:
- Asynchronous, non-blocking I/O
- Modern networking on macOS
- Efficient connection management

## How to Run

1. Start a TCP server:
   ```bash
   nc -l 8080
   ```

2. Modify `src/platform/bridge/SwiftBridge.mm`:
   ```cpp
   #include "../../examples/17_tcp_client/main.cpp"
   // In setup():
   testApp_ = std::make_unique<TcpClientApp>();
   ```

3. Build and run the Xcode project

4. Watch the console for connection messages and data exchange

## Expected Output

When connected, you should see:
- Console logs showing connection success
- Periodic "Sent: Hello from TCP Client! #N" messages
- Server responses appearing in the received data box
- Animated data packets flowing between client and server visualizations
- FPS bar showing 60 FPS

## Testing with netcat

1. Start netcat server:
   ```bash
   nc -l 8080
   ```

2. Run this example
3. In the netcat terminal, you'll see incoming messages
4. Type responses and press Enter to send them back
5. The example will display received messages

## Error Handling

The example handles common scenarios:
- **Connection refused**: Shows error in console
- **Server disconnects**: Updates connection status
- **Send failures**: Logs error messages
- **Reconnection**: Manual reconnection via 'R' key

## Network Configuration

Default settings:
- **Host**: localhost (127.0.0.1)
- **Port**: 8080
- **Send interval**: 2 seconds
- **Message delimiter**: newline (\n)
- **Non-blocking mode**: Enabled

To change the server address or port, modify:
```cpp
std::string serverHost = "localhost";
uint16_t serverPort = 8080;
```

## Next Steps

After mastering this example, try:
- **Example 18: TCP Server** - Create a server to accept connections
- **Example 19: UDP Sender** - Send UDP datagrams
- **Example 20: UDP Receiver** - Receive UDP datagrams
- Combining TCP client with other features (OSC, GUI parameters)
- Creating a chat application
- Implementing a custom protocol
