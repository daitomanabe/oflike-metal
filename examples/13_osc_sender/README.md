# Example 13: OSC Sender

Demonstrates **ofxOsc sender capabilities** with real-time parameter transmission.

## Features Demonstrated

- Setting up an OSC sender (`ofxOscSender::setup`)
- Sending messages with different argument types (int32, float, string, bool)
- Sending OSC bundles (grouped messages)
- Real-time mouse position transmission
- Multiple message examples (key press triggers)

## What It Does

This example sends OSC messages to `127.0.0.1:12345`:

1. **Continuous Mouse Position**: Sends normalized mouse coordinates every frame via `/mouse` address
2. **Key Press Messages**:
   - `1` → Send int32 value (42)
   - `2` → Send float value (3.14159)
   - `3` → Send string message ("Hello OSC!")
   - `4` → Send multi-argument message (int, float, string, bool)
   - `5` → Send OSC bundle with 3 messages

## Running the Example

1. First start the **OSC Receiver** example (14) to listen for messages
2. Then run this sender example
3. Move the mouse to send position updates
4. Press keys 1-5 to send different message types

## Code Highlights

```cpp
// Setup sender
ofxOscSender sender;
sender.setup("127.0.0.1", 12345);

// Send simple message
ofxOscMessage msg;
msg.setAddress("/mouse");
msg.addFloatArg(mouseX);
msg.addFloatArg(mouseY);
sender.sendMessage(msg);

// Send bundle
ofxOscBundle bundle;
bundle.addMessage(msg1);
bundle.addMessage(msg2);
sender.sendBundle(bundle);
```

## openFrameworks Compatibility

This example uses the same API as openFrameworks ofxOsc:
- `ofxOscSender`
- `ofxOscMessage` with `setAddress()` and `add*Arg()` methods
- `ofxOscBundle` for grouped messages

## See Also

- **Example 14**: OSC Receiver (receives messages from this example)
- **ofxOscMessage**: Message creation and argument types
- **ofxOscBundle**: Bundling multiple messages
