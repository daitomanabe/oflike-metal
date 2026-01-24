# Example 18: TCP Server

This example demonstrates TCP server functionality using the ofxTcpServer addon.

## What This Example Shows

- **TCP Server Setup**: Starting a TCP server on a specific port
- **Client Management**: Accepting and managing multiple client connections
- **Message Receiving**: Receiving messages from connected clients
- **Echo Response**: Sending responses back to individual clients
- **Broadcasting**: Sending messages to all connected clients
- **Connection Monitoring**: Tracking client connections and disconnections
- **Non-blocking I/O**: Asynchronous communication using Network.framework

## Features

### Automatic Echo Server
- Receives messages from any connected client
- Echoes each message back to the sender with "Echo: " prefix
- Logs all received messages to console

### Periodic Broadcasting
- Broadcasts a status message to all clients every 5 seconds
- Shows the current number of connected clients

### Visual Feedback
- **Green indicator**: Server is listening
- **Red indicator**: Server is not running
- **Client visualization**: Shows connected clients arranged in a circle
- **Connection lines**: Indicate active connections
- **Animated packets**: Visual representation of data flow from clients to server

### Interactive Controls
- **Space**: Broadcast a manual message to all clients
- **D**: Disconnect the first client
- **X**: Clear message history

## API Functions Used

### ofxTcpServer
- `setup(port)` - Start server on specified port
- `close()` - Close server and all connections
- `isListening()` - Check if server is running
- `setMaxClients(max)` - Set maximum number of clients
- `waitForClient(timeout)` - Accept new client connections
- `getNumClients()` - Get number of connected clients
- `isClientConnected(index)` - Check if specific client is connected
- `receive(index)` - Receive data from specific client
- `send(index, data)` - Send data to specific client
- `sendToAll(data)` - Broadcast to all clients
- `disconnectClient(index)` - Disconnect specific client
- `getClientIP(index)` - Get client IP address
- `getError()` - Get last error message

### Network.framework Integration
The implementation uses Apple's Network.framework for:
- Asynchronous, non-blocking I/O
- Modern networking on macOS
- Efficient multi-client management
- Automatic connection handling

## How to Run

1. Modify `src/platform/bridge/SwiftBridge.mm`:
   ```cpp
   #include "../../examples/18_tcp_server/main.cpp"
   // In setup():
   testApp_ = std::make_unique<TcpServerApp>();
   ```

2. Build and run the Xcode project

3. Connect clients using netcat or the tcp_client example

## Testing with netcat

### Connect a client:
```bash
nc localhost 8080
```

### Send messages:
Type any message and press Enter. The server will echo it back.

### Multiple clients:
Open multiple terminal windows and run `nc localhost 8080` in each.

## Testing with tcp_client Example

Run the `17_tcp_client` example (requires separate app instance or modification to test app loading).

## Expected Output

When running, you should see:
- Console logs showing server startup
- "Waiting for clients..." message
- Connection logs when clients connect
- Received messages from each client
- Echo confirmations
- Periodic broadcast messages
- Visual representation of connected clients

### Console Example:
```
TCP Server started on port 8080
Waiting for clients...
Max clients: 10
Client 0 (127.0.0.1): Hello Server!
Sent echo to client 0
Broadcast: 1 clients connected
Client 0 (127.0.0.1): Another message
Sent echo to client 0
```

## Network Configuration

Default settings:
- **Port**: 8080
- **Max clients**: 10
- **Broadcast interval**: 5 seconds
- **Non-blocking mode**: Enabled

To change the port or max clients, modify:
```cpp
uint16_t serverPort = 8080;
int maxClients = 10;
```

## Multi-Client Testing

The server can handle multiple simultaneous connections:

1. Start the server
2. Open multiple terminal windows
3. In each window, run: `nc localhost 8080`
4. Type messages in any window
5. See echoes in the same window
6. Broadcasts appear in all windows

## Client Management

### Client Indexing
- Clients are indexed 0, 1, 2, etc. in order of connection
- Use `getNumClients()` to iterate through all clients
- Check `isClientConnected(i)` before sending to a specific client

### Disconnection Handling
- Clients are automatically removed when they disconnect
- Use `disconnectClient(i)` to manually disconnect a client
- The 'D' key disconnects the first client for testing

## Error Handling

The example handles:
- **Port already in use**: Shows error on startup
- **Client disconnections**: Detected automatically
- **Send failures**: Logged to console
- **Invalid client index**: Checked before operations

## Performance

- Non-blocking I/O ensures smooth 60 FPS operation
- Can handle multiple clients simultaneously
- Efficient message processing per frame
- Visual feedback doesn't impact network performance

## Next Steps

After mastering this example, try:
- **Example 17: TCP Client** - Connect to this server
- **Example 19: UDP Sender** - Compare with connectionless UDP
- **Example 20: UDP Receiver** - UDP alternative
- Creating a chat room server
- Implementing custom protocols
- Combining with OSC or GUI for parameter control
- Building a multi-user collaborative application
