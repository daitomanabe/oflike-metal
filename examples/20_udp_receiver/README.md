# Example 20: UDP Receiver

This example demonstrates UDP receiving functionality using the ofxUdpManager addon.

## What This Example Shows

- **UDP Reception**: Receiving UDP datagrams on a specific port
- **Connectionless Protocol**: No need to establish connections
- **Sender Identification**: Tracking where messages come from
- **Message Buffering**: Storing recent messages
- **Non-blocking I/O**: Asynchronous reception
- **Multiple Senders**: Handling messages from different sources
- **Visual Feedback**: Animated incoming packet visualization

## Features

### Automatic Reception
- Continuously listens for incoming UDP messages
- Non-blocking reception maintains 60 FPS
- Processes all available messages each frame
- Identifies sender IP and port for each message

### Message Buffer
- Stores most recent messages (default: 10)
- Adjustable buffer size (5-100 messages)
- Automatic overflow management (FIFO queue)
- Visual display of message history

### Statistics Tracking
- Total messages received
- Total bytes received
- Last receive timestamp
- Activity indicator

### Visual Feedback
- **Green indicator**: Receiver is bound and listening
- **Red indicator**: Failed to bind
- **Multiple sender visualization**: Shows up to 5 senders around receiver
- **Animated packets**: Visual representation of incoming data
- **Activity pulse**: Pulses when messages are being received

### Interactive Controls
- **C**: Clear message history
- **R**: Reset statistics
- **+/-**: Adjust message buffer size

## API Functions Used

### ofxUdpManager
- `bind(port)` - Bind to port for receiving
- `receiveFrom(buffer, maxLength, senderIP, senderPort)` - Receive with sender info
- `close()` - Close the receiver
- `getError()` - Get last error message

### Network.framework Integration
Uses Apple's Network.framework for:
- Efficient UDP datagram reception
- Modern networking API
- Non-blocking I/O
- Multiple sender support

## How to Run

1. Modify `src/platform/bridge/SwiftBridge.mm`:
   ```cpp
   #include "../../examples/20_udp_receiver/main.cpp"
   // In setup():
   testApp_ = std::make_unique<UdpReceiverApp>();
   ```

2. Build and run the Xcode project

3. Send messages using netcat or the udp_sender example

## Sending Messages to This Receiver

### Option 1: Using netcat
```bash
# Single message
echo "Hello UDP" | nc -u localhost 9000

# Interactive mode
nc -u localhost 9000
# Type messages and press Enter
```

### Option 2: Using udp_sender example
Run Example 19 (UDP Sender) to send automatic periodic messages.

### Option 3: Using command line loop
```bash
# Send 10 numbered messages
for i in {1..10}; do
  echo "Message $i" | nc -u localhost 9000
  sleep 0.5
done
```

## Expected Output

When receiving messages, you should see:
- Console logs showing sender IP, port, and message content
- Messages appearing in the visual list
- Animated packets flowing from sender visualizations to receiver
- Activity indicator pulsing
- Statistics updating

### Console Example:
```
Binding to port 9000
UDP receiver ready on port 9000
Waiting for messages...
Received from 127.0.0.1:54321 (10 bytes): Hello UDP
Received from 127.0.0.1:54321 (14 bytes): Another message
```

## Network Configuration

Default settings:
- **Listen port**: 9000
- **Max message buffer**: 10 messages
- **Non-blocking mode**: Enabled
- **Buffer size**: 8192 bytes per message

To change the port, modify:
```cpp
uint16_t listenPort = 9000;
```

## Message Handling

### Buffer Management
- Messages stored in a deque (double-ended queue)
- New messages added to back
- Old messages removed from front when buffer is full
- Most recent messages displayed first

### Sender Tracking
Each message includes:
- Sender IP address
- Sender port number
- Timestamp of reception
- Message content

### Multiple Senders
The receiver can handle messages from multiple senders:
- Each sender identified by IP:port combination
- No connection establishment needed
- Visualizes up to 5 different senders

## UDP Characteristics

### Advantages
- **Fast**: Lower overhead than TCP
- **Simple**: No connection management
- **Flexible**: Accept from any sender
- **Efficient**: Minimal protocol overhead

### Limitations
- **No delivery guarantee**: Packets may be lost
- **No ordering**: Messages may arrive out of order
- **No flow control**: Can be overwhelmed by fast senders
- **Limited size**: Maximum datagram size (~64KB, typically ~8KB)

## Testing Scenarios

### 1. Single Sender
```bash
# Send messages from one terminal
nc -u localhost 9000
```

### 2. Multiple Senders
```bash
# Terminal 1
nc -u localhost 9000

# Terminal 2
nc -u localhost 9000

# Terminal 3
nc -u localhost 9000
```
Watch the visualization show multiple senders.

### 3. High-Speed Messages
Use the UDP sender with burst mode ('B' key) to test high-speed reception.

### 4. Large Messages
```bash
# Send a large message
head -c 5000 /dev/urandom | base64 | nc -u localhost 9000
```

## Performance

- Non-blocking ensures smooth 60 FPS
- Can receive hundreds of messages per second
- Message buffer prevents memory overflow
- Efficient string handling

## Troubleshooting

### Not Receiving Messages
- Check firewall settings
- Verify port 9000 is not in use: `lsof -i :9000`
- Confirm sender is targeting correct port
- Try localhost (127.0.0.1) first

### Messages Getting Lost
- This is normal UDP behavior
- Reduce send rate if many messages are lost
- Consider TCP for reliable delivery
- Increase buffer size if processing can't keep up

### Buffer Overflow
- Increase buffer size with '+' key
- Reduce sender rate
- Process messages more efficiently

## Advanced Usage

### Custom Port
Change the listening port:
```cpp
uint16_t listenPort = 12345;
```

### Binary Data Reception
The receiver handles binary data:
```cpp
// In update():
struct Packet {
    int id;
    float x, y, z;
};
Packet* p = (Packet*)buffer;
// Use p->id, p->x, etc.
```

### Multicast Reception
To receive multicast messages:
```cpp
// In setup():
udp.bind("0.0.0.0", 9000);
udp.joinMulticastGroup("239.255.0.1");
```

### Selective Processing
Filter messages by sender:
```cpp
if (senderIP == "192.168.1.100") {
    // Process only messages from specific sender
}
```

## Integration Ideas

### Real-time Sensor Data
- Receive sensor readings via UDP
- Visualize data in real-time
- Log statistics

### OSC Integration
- UDP is the transport for OSC
- Can receive OSC-formatted messages
- See Example 14: OSC Receiver for OSC-specific handling

### Game Networking
- Receive player positions
- Handle game events
- Implement simple multiplayer

### Video Streaming
- Receive video frame data
- Display in ofImage/ofTexture
- Implement low-latency video

## Comparison with TCP Server

| Feature | UDP Receiver | TCP Server (Example 18) |
|---------|--------------|-------------------------|
| Connection | No connection needed | Must establish connection |
| Reliability | May lose packets | Guaranteed delivery |
| Setup | Simpler | More complex |
| Sender tracking | Per-packet | Per-connection |
| Use case | Real-time, broadcast | Reliable data transfer |

## Next Steps

After mastering this example, try:
- **Example 19: UDP Sender** - Send messages to this receiver
- **Example 18: TCP Server** - Compare with TCP reliability
- Creating a UDP-based data visualization system
- Implementing a simple chat protocol
- Building a sensor network receiver
- Combining with OSC for parameter control
- Creating a network monitoring dashboard
