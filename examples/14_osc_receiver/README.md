# Example 14: OSC Receiver

Demonstrates **ofxOsc receiver capabilities** with thread-safe message queue and real-time parsing.

## Features Demonstrated

- Setting up an OSC receiver (`ofxOscReceiver::setup`)
- Starting listener thread (`start()`)
- Receiving messages with different argument types
- Parsing and displaying received data
- Thread-safe message queue handling
- Message history display

## What It Does

This example listens on **port 12345** and processes incoming OSC messages:

1. **Mouse Position**: Receives `/mouse` messages and displays crosshair
2. **Typed Messages**: Parses int32, float, string, and multi-argument messages
3. **Bundles**: Handles bundled messages from `/bundle/*` addresses
4. **Message History**: Displays the 10 most recent messages
5. **Statistics**: Shows total message count and queue size

## Running the Example

1. First run this receiver example (listens on port 12345)
2. Then run the **OSC Sender** example (13) to send messages
3. Watch the console and display for received messages

## Code Highlights

```cpp
// Setup receiver
ofxOscReceiver receiver;
receiver.setup(12345);
receiver.start(); // Start listener thread

// Receive messages
while (receiver.hasWaitingMessages()) {
    ofxOscMessage msg;
    receiver.getNextMessage(msg);

    // Parse message
    std::string address = msg.getAddress();
    if (address == "/mouse") {
        float x = msg.getArgAsFloat(0);
        float y = msg.getArgAsFloat(1);
    }
}

// Clean shutdown
receiver.stop();
receiver.shutdown();
```

## Threading Model

The receiver uses two modes:

1. **Polling Mode** (default): Call `hasWaitingMessages()` and `getNextMessage()` manually
2. **Listener Thread Mode**: Call `start()` to run background thread that queues messages automatically

This example uses listener thread mode for automatic message queueing.

## openFrameworks Compatibility

This example uses the same API as openFrameworks ofxOsc:
- `ofxOscReceiver` with `setup()` and `start()` methods
- `hasWaitingMessages()` / `getNextMessage()` polling
- Same argument access methods (`getArgAsInt32()`, `getArgAsFloat()`, etc.)

## Message Types Supported

- **Int32**: 32-bit integers
- **Int64**: 64-bit integers
- **Float**: 32-bit floats
- **Double**: 64-bit doubles
- **String**: UTF-8 strings
- **Blob**: Binary data
- **Bool**: Boolean values
- **Char**: Single characters
- **TimeTag**: OSC timestamps
- **RGBA Color**: Color values
- **MIDI Message**: MIDI data

## See Also

- **Example 13**: OSC Sender (sends messages to this example)
- **ofxOscMessage**: Message parsing and argument access
- **ofxOscReceiver**: Multicast support with `setupMulticast()`
