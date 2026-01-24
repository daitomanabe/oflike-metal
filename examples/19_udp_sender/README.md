# Example 19: UDP Sender

This example demonstrates UDP sending functionality using the ofxUdpManager addon.

## What This Example Shows

- **UDP Communication**: Connectionless datagram protocol
- **Message Sending**: Sending UDP packets to a receiver
- **Non-blocking I/O**: Asynchronous transmission
- **Rate Control**: Adjustable send intervals
- **Burst Sending**: Sending multiple messages rapidly
- **Statistics Tracking**: Monitoring bytes sent and message count
- **Visual Feedback**: Animated packet transmission

## Prerequisites

Before running this example, you need a UDP receiver listening on port 9000.

### Option 1: Use netcat
```bash
nc -u -l 9000
```

### Option 2: Use the udp_receiver example
Run the `20_udp_receiver` example in a separate instance.

## Features

### Automatic Sending
- Sends messages automatically at a configurable interval (default: 1 second)
- Each message includes a sequence number and timestamp
- Non-blocking operation maintains 60 FPS

### Rate Control
- Press `+` to increase send rate (decrease interval)
- Press `-` to decrease send rate (increase interval)
- Interval range: 0.1s to 10.0s

### Visual Feedback
- **Green indicator**: UDP sender is ready
- **Red indicator**: Setup failed
- **Animated packets**: Shows data flowing from sender to receiver
- **Rate bar**: Visual countdown to next send
- **Multiple packet trails**: Shows recent transmissions

### Interactive Controls
- **Space**: Send a message immediately
- **+/-**: Adjust send rate
- **B**: Send burst of 10 messages
- **R**: Reset statistics

## UDP vs TCP

### Key Differences

| Feature | UDP (this example) | TCP (Example 17) |
|---------|-------------------|------------------|
| Connection | Connectionless | Connection-oriented |
| Reliability | No guarantee | Guaranteed delivery |
| Order | May arrive out of order | Ordered delivery |
| Speed | Faster, lower overhead | Slower, more overhead |
| Use case | Real-time, streaming | Reliable data transfer |

### When to Use UDP
- Real-time applications (audio, video streaming)
- Game networking (position updates)
- IoT sensor data
- Multicast/broadcast scenarios
- When speed matters more than reliability

## API Functions Used

### ofxUdpManager
- `connect(host, port)` - Set target host for sending
- `send(data, length)` - Send data to connected host
- `close()` - Close the UDP connection
- `getError()` - Get last error message

### Network.framework Integration
Uses Apple's Network.framework for:
- Efficient UDP datagram transmission
- Modern networking API
- Non-blocking I/O

## How to Run

1. Start a UDP receiver:
   ```bash
   nc -u -l 9000
   ```

2. Modify `src/platform/bridge/SwiftBridge.mm`:
   ```cpp
   #include "../../examples/19_udp_sender/main.cpp"
   // In setup():
   testApp_ = std::make_unique<UdpSenderApp>();
   ```

3. Build and run the Xcode project

4. Watch messages appear in the netcat terminal

## Expected Output

### In the application:
- Visual packet animation from sender to receiver
- Statistics showing messages sent and bytes transmitted
- Rate bar indicating time until next send

### In netcat receiver:
```
UDP Message #0 | Time: 1.234567
UDP Message #1 | Time: 2.345678
UDP Message #2 | Time: 3.456789
...
```

## Testing Scenarios

### 1. Basic Sending
```bash
# Terminal 1: Start receiver
nc -u -l 9000

# Terminal 2: Run this example
# Messages appear in Terminal 1
```

### 2. High-Speed Burst
- Press 'B' to send burst
- Receiver should show 10 messages rapidly

### 3. Rate Adjustment
- Press '+' several times
- Observe faster message rate
- Press '-' to slow down

### 4. No Receiver
- Run without a receiver listening
- Messages still "send" (UDP is connectionless)
- No errors occur (fire-and-forget)

## Network Configuration

Default settings:
- **Remote host**: localhost (127.0.0.1)
- **Remote port**: 9000
- **Send interval**: 1.0 seconds
- **Non-blocking mode**: Enabled

To change the target, modify:
```cpp
std::string remoteHost = "localhost";
uint16_t remotePort = 9000;
```

## Performance

- Maintains 60 FPS even with burst sending
- Minimal CPU overhead
- Non-blocking ensures smooth operation
- Can send hundreds of messages per second

## Packet Loss

UDP does not guarantee delivery:
- Packets may be lost (especially on congested networks)
- No automatic retransmission
- No acknowledgment from receiver
- Application must handle reliability if needed

## Advanced Usage

### Custom Message Format
```cpp
// Send binary data
struct Packet {
    int id;
    float x, y, z;
};
Packet p = {messageCount, 1.0f, 2.0f, 3.0f};
udp.send((char*)&p, sizeof(Packet));
```

### Broadcast to Multiple Receivers
Use `sendTo()` to target multiple hosts:
```cpp
udp.sendTo("192.168.1.100", 9000, data, length);
udp.sendTo("192.168.1.101", 9000, data, length);
```

### Multicast (requires different setup)
For multicast, use the receiver's `joinMulticastGroup()` method.

## Troubleshooting

### Messages Not Arriving
- Verify receiver is running
- Check firewall settings
- Confirm port number matches
- Try localhost before remote hosts

### Performance Issues
- Reduce send rate
- Check message size
- Monitor system resources

## Next Steps

After mastering this example, try:
- **Example 20: UDP Receiver** - Receive the messages
- **Example 17: TCP Client** - Compare with TCP reliability
- Creating a UDP-based game networking system
- Implementing OSC-style message protocols
- Building a sensor data streaming system
- Combining with computer vision for real-time data
