# OSC Parameter Sync Example

Demonstrates bidirectional parameter synchronization between your application and external OSC controllers (TouchOSC, Max/MSP, Pure Data, etc.).

## What It Does

- **Automatic Bidirectional Sync**: Changes in your app are sent via OSC, and incoming OSC messages update parameters
- **Parameter System**: Uses ofParameter<T> for type-safe parameter management
- **Group Support**: Can sync entire parameter groups with hierarchical OSC addresses
- **Change Callbacks**: Parameters notify listeners when values change

## OSC Configuration

This example:
- **Sends** OSC to: `localhost:9001`
- **Receives** OSC on: `localhost:9000`

## OSC Addresses

| Address | Type | Range | Description |
|---------|------|-------|-------------|
| `/audio/volume` | Float | 0.0 - 1.0 | Audio volume level |
| `/audio/frequency` | Int | 100 - 1000 | Frequency in Hz |
| `/audio/enabled` | Bool | 0 / 1 | Enable/disable audio |

## Controls

- **Q / A**: Increase / decrease volume
- **W / S**: Increase / decrease frequency
- **E**: Toggle enabled state
- **Space**: Send all parameters

## Testing with TouchOSC

1. Create a TouchOSC layout with:
   - Fader: `/audio/volume`
   - Rotary: `/audio/frequency`
   - Toggle: `/audio/enabled`

2. Configure TouchOSC:
   - Host: `localhost` or your computer's IP
   - Send Port: `9000` (app receives)
   - Receive Port: `9001` (app sends)

## Testing with Max/MSP

```max
// Sender (controls the app)
[udpsend localhost 9000]
|
[prepend /audio/volume]
|
[slider] 0. - 1.

// Receiver (monitors the app)
[udpreceive 9001]
|
[route /audio/volume /audio/frequency /audio/enabled]
|         |         |
[float]  [int]    [toggle]
```

## Testing with Pure Data

```pd
// Sender
[netsend -u -b]
connect localhost 9000

[pack /audio/volume f]
|
[slider]

// Receiver
[netreceive -u -b 9001]
|
[route /audio/volume /audio/frequency /audio/enabled]
```

## Code Structure

```cpp
// Create parameters
ofParameter<float> volume("volume", 0.5f, 0.0f, 1.0f);
ofParameter<int> frequency("frequency", 440, 100, 1000);
ofParameter<bool> enabled("enabled", true);

// Setup OSC sync
ofxOscParameterSync oscSync;
oscSync.setup("localhost", 9001, 9000);

// Add parameters
oscSync.add(volume, "/audio/volume");
oscSync.add(frequency, "/audio/frequency");
oscSync.add(enabled, "/audio/enabled");

// Or add entire group
ofParameterGroup audioParams("audio");
audioParams.add(volume);
audioParams.add(frequency);
audioParams.add(enabled);
oscSync.add(audioParams, "/audio");

// In update loop
oscSync.update();
```

## Features

### Automatic Sending
When you change a parameter value in code, it's automatically sent via OSC:

```cpp
volume.set(0.8f);  // Automatically sends /audio/volume 0.8
```

### Automatic Receiving
When an OSC message arrives, the parameter is automatically updated:

```
/audio/volume 0.6  // Parameter automatically updated to 0.6
```

### Manual Control
```cpp
// Disable auto-send, then manually send
oscSync.setAutoSend(false);
volume.set(0.5f);
oscSync.sendAll();  // Send all parameters now

// Disable auto-receive
oscSync.setAutoReceive(false);
oscSync.update();  // Won't update parameters
```

## Architecture Notes

- Uses SwiftUI-style parameter binding pattern
- Type-safe with template parameters
- Zero-copy listeners for efficient callbacks
- Compatible with ofxGui for visual parameter editing

## Related Examples

- `18_osc_sender`: One-way OSC sending
- `18_osc_receiver`: One-way OSC receiving
- `21_gui_basic`: GUI parameter controls
