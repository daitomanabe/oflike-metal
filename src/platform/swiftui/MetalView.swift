import SwiftUI
import MetalKit
import QuartzCore

// MARK: - MetalView
// Note: PerformanceStats is defined in PerformanceMonitor.swift

/// SwiftUI wrapper for MTKView with FPS overlay (Phase 1.5)
struct MetalView: View {
    @StateObject private var coordinator = MetalViewCoordinator()

    var body: some View {
        ZStack(alignment: .topLeading) {
            // Metal rendering view
            MetalViewRepresentable(coordinator: coordinator)

            // FPS overlay (Phase 1.5)
            VStack(alignment: .leading, spacing: 4) {
                Text("FPS: \(Int(coordinator.currentFPS))")
                    .font(.system(size: 14, weight: .medium, design: .monospaced))
                    .foregroundColor(.white)
                Text("Frame: \(coordinator.frameCount)")
                    .font(.system(size: 12, design: .monospaced))
                    .foregroundColor(.white.opacity(0.8))
                Text("Elapsed: \(Int(coordinator.getElapsedTime()))s")
                    .font(.system(size: 12, design: .monospaced))
                    .foregroundColor(.white.opacity(0.8))
            }
            .padding(12)
            .background(Color.black.opacity(0.6))
            .cornerRadius(8)
            .padding(12)
        }
    }
}

/// Internal NSViewRepresentable for MTKView
private struct MetalViewRepresentable: NSViewRepresentable {
    @ObservedObject var coordinator: MetalViewCoordinator

    func makeNSView(context: Context) -> MTKView {
        let mtkView = MouseTrackingMTKView()

        // Get Metal device
        guard let device = MTLCreateSystemDefaultDevice() else {
            fatalError("Metal is not supported on this device")
        }

        mtkView.device = device
        mtkView.delegate = coordinator
        mtkView.clearColor = MTLClearColor(red: 0.0, green: 0.0, blue: 0.0, alpha: 1.0)
        mtkView.colorPixelFormat = .bgra8Unorm
        mtkView.depthStencilPixelFormat = .depth32Float
        mtkView.framebufferOnly = false
        mtkView.enableSetNeedsDisplay = false
        mtkView.isPaused = false
        mtkView.preferredFramesPerSecond = 60

        // Set coordinator as mouse event receiver
        mtkView.mouseEventReceiver = coordinator

        // Initialize coordinator with device
        coordinator.setup(device: device)

        return mtkView
    }

    func updateNSView(_ nsView: MTKView, context: Context) {
        // Update view if needed
    }
}

/// Custom MTKView that tracks mouse events
/// Phase 13.1: Mouse event tracking
private class MouseTrackingMTKView: MTKView {
    weak var mouseEventReceiver: MouseEventReceiver?

    private var trackingArea: NSTrackingArea?

    override init(frame frameRect: NSRect, device: MTLDevice?) {
        super.init(frame: frameRect, device: device)
        // Register for file drag and drop
        registerForDraggedTypes([.fileURL])
    }

    required init(coder: NSCoder) {
        super.init(coder: coder)
        // Register for file drag and drop
        registerForDraggedTypes([.fileURL])
    }

    override func updateTrackingAreas() {
        super.updateTrackingAreas()

        // Remove old tracking area
        if let existingArea = trackingArea {
            removeTrackingArea(existingArea)
        }

        // Create new tracking area for the entire view bounds
        let options: NSTrackingArea.Options = [
            .mouseEnteredAndExited,
            .mouseMoved,
            .activeInKeyWindow,
            .inVisibleRect
        ]

        trackingArea = NSTrackingArea(
            rect: bounds,
            options: options,
            owner: self,
            userInfo: nil
        )

        if let area = trackingArea {
            addTrackingArea(area)
        }
    }

    override func mouseMoved(with event: NSEvent) {
        let location = convert(event.locationInWindow, from: nil)
        // Convert to oF convention: Y=0 at top, Y increases downward
        let ofY = bounds.size.height - location.y
        mouseEventReceiver?.mouseMoved(x: Float(location.x), y: Float(ofY))
    }

    override func mouseEntered(with event: NSEvent) {
        let location = convert(event.locationInWindow, from: nil)
        // Convert to oF convention: Y=0 at top, Y increases downward
        let ofY = bounds.size.height - location.y
        mouseEventReceiver?.mouseEntered(x: Float(location.x), y: Float(ofY))
    }

    override func mouseExited(with event: NSEvent) {
        let location = convert(event.locationInWindow, from: nil)
        // Convert to oF convention: Y=0 at top, Y increases downward
        let ofY = bounds.size.height - location.y
        mouseEventReceiver?.mouseExited(x: Float(location.x), y: Float(ofY))
    }

    override func mouseDragged(with event: NSEvent) {
        let location = convert(event.locationInWindow, from: nil)
        // Convert to oF convention: Y=0 at top, Y increases downward
        let ofY = bounds.size.height - location.y
        // Map NSEvent button number to openFrameworks convention
        // NSEvent: 0=left, 1=right, 2=middle
        // oF uses same convention: 0=left, 1=right, 2=middle
        let button = Int(event.buttonNumber)
        mouseEventReceiver?.mouseDragged(x: Float(location.x), y: Float(ofY), button: button)
    }

    override func mouseDown(with event: NSEvent) {
        let location = convert(event.locationInWindow, from: nil)
        // Convert to oF convention: Y=0 at top, Y increases downward
        let ofY = bounds.size.height - location.y
        // Left mouse button (0)
        mouseEventReceiver?.mousePressed(x: Float(location.x), y: Float(ofY), button: 0)
    }

    override func rightMouseDown(with event: NSEvent) {
        let location = convert(event.locationInWindow, from: nil)
        // Convert to oF convention: Y=0 at top, Y increases downward
        let ofY = bounds.size.height - location.y
        // Right mouse button (1)
        mouseEventReceiver?.mousePressed(x: Float(location.x), y: Float(ofY), button: 1)
    }

    override func otherMouseDown(with event: NSEvent) {
        let location = convert(event.locationInWindow, from: nil)
        // Convert to oF convention: Y=0 at top, Y increases downward
        let ofY = bounds.size.height - location.y
        // Middle or other button (2+)
        let button = Int(event.buttonNumber)
        mouseEventReceiver?.mousePressed(x: Float(location.x), y: Float(ofY), button: button)
    }

    override func mouseUp(with event: NSEvent) {
        let location = convert(event.locationInWindow, from: nil)
        // Convert to oF convention: Y=0 at top, Y increases downward
        let ofY = bounds.size.height - location.y
        // Left mouse button (0)
        mouseEventReceiver?.mouseReleased(x: Float(location.x), y: Float(ofY), button: 0)
    }

    override func rightMouseUp(with event: NSEvent) {
        let location = convert(event.locationInWindow, from: nil)
        // Convert to oF convention: Y=0 at top, Y increases downward
        let ofY = bounds.size.height - location.y
        // Right mouse button (1)
        mouseEventReceiver?.mouseReleased(x: Float(location.x), y: Float(ofY), button: 1)
    }

    override func otherMouseUp(with event: NSEvent) {
        let location = convert(event.locationInWindow, from: nil)
        // Convert to oF convention: Y=0 at top, Y increases downward
        let ofY = bounds.size.height - location.y
        // Middle or other button (2+)
        let button = Int(event.buttonNumber)
        mouseEventReceiver?.mouseReleased(x: Float(location.x), y: Float(ofY), button: button)
    }

    override func scrollWheel(with event: NSEvent) {
        let location = convert(event.locationInWindow, from: nil)
        // Convert to oF convention: Y=0 at top, Y increases downward
        let ofY = bounds.size.height - location.y
        // NSEvent provides scroll deltas in pixels (for precise scrolling)
        // scrollingDeltaX: horizontal scroll amount (positive = right, negative = left)
        // scrollingDeltaY: vertical scroll amount (positive = up, negative = down)
        mouseEventReceiver?.mouseScrolled(
            x: Float(location.x),
            y: Float(ofY),
            scrollX: Float(event.scrollingDeltaX),
            scrollY: Float(event.scrollingDeltaY)
        )
    }

    // MARK: - Keyboard Events (Phase 13.2)

    override var acceptsFirstResponder: Bool {
        return true
    }

    override func keyDown(with event: NSEvent) {
        // Get the key code from the event
        let keyCode = Int(event.keyCode)
        mouseEventReceiver?.keyPressed(key: keyCode)
    }

    override func keyUp(with event: NSEvent) {
        // Get the key code from the event
        let keyCode = Int(event.keyCode)
        mouseEventReceiver?.keyReleased(key: keyCode)
    }

    // MARK: - Drag and Drop

    override func draggingEntered(_ sender: NSDraggingInfo) -> NSDragOperation {
        return .copy
    }

    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {
        let location = convert(sender.draggingLocation, from: nil)
        // Convert to oF convention: Y=0 at top, Y increases downward
        let ofY = bounds.size.height - location.y
        mouseEventReceiver?.dragEvent(x: Float(location.x), y: Float(ofY))
        return true
    }
}

/// Protocol for receiving mouse and keyboard events from MouseTrackingMTKView
protocol MouseEventReceiver: AnyObject {
    func mouseMoved(x: Float, y: Float)
    func mouseDragged(x: Float, y: Float, button: Int)
    func mousePressed(x: Float, y: Float, button: Int)
    func mouseReleased(x: Float, y: Float, button: Int)
    func mouseScrolled(x: Float, y: Float, scrollX: Float, scrollY: Float)
    func mouseEntered(x: Float, y: Float)
    func mouseExited(x: Float, y: Float)
    func keyPressed(key: Int)
    func keyReleased(key: Int)
    func dragEvent(x: Float, y: Float)
}

/// Coordinator that implements MTKViewDelegate and MouseEventReceiver
class MetalViewCoordinator: NSObject, MTKViewDelegate, ObservableObject, MouseEventReceiver {
    // Static weak reference for accessing coordinator from C callbacks
    private static weak var sharedCoordinator: MetalViewCoordinator?

    // Phase 5.2: Command queue for renderFrame
    private var commandQueue: MTLCommandQueue?

    // Phase 1.5: Frame tracking for FPS display
    @Published var frameCount: UInt64 = 0
    @Published var currentFPS: Double = 0.0
    private var startTime: CFTimeInterval = 0
    private var lastFPSUpdate: CFTimeInterval = 0
    private var framesSinceLastUpdate: Int = 0

    // Phase 14.1: Window state
    @Published var windowTitle: String = "oflike-metal"
    @Published var isFullscreen: Bool = false

    // C++ Bridge
    private var bridge: OFLBridge?

    override init() {
        super.init()
        startTime = CACurrentMediaTime()
        lastFPSUpdate = startTime
        bridge = OFLBridge()
        // Set static reference to this coordinator
        MetalViewCoordinator.sharedCoordinator = self
    }

    func setup(device: MTLDevice) {
        // Phase 5.2: Create command queue for renderFrame
        self.commandQueue = device.makeCommandQueue()

        // Phase 3: Initialize global context with Metal device
        bridge?.initializeContext(withDevice: device)

        // Phase 14.1: Register window callbacks
        setupWindowCallbacks()

        // Initialize C++ bridge
        bridge?.setup()
    }

    // MARK: - MTKViewDelegate

    /// Called whenever view changes orientation or is resized
    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
        // Handle resize
        print("MetalView resized to: \(size.width) x \(size.height)")

        // Notify C++ layer through bridge
        bridge?.windowResizedWidth(Float(size.width), height: Float(size.height))
    }

    /// Called whenever the view needs to render a frame
    func draw(in view: MTKView) {
        // Frame update logic
        update()

        // Render frame
        render(view: view)

        // Phase 1.5: Update frame count and FPS
        frameCount += 1
        framesSinceLastUpdate += 1

        let currentTime = CACurrentMediaTime()
        let elapsed = currentTime - lastFPSUpdate

        // Update FPS every 0.5 seconds
        if elapsed >= 0.5 {
            currentFPS = Double(framesSinceLastUpdate) / elapsed
            framesSinceLastUpdate = 0
            lastFPSUpdate = currentTime
        }

        // Phase 16.2: Update performance statistics
        updatePerformanceStats()
    }

    // MARK: - Frame Loop

    /// Update logic (called every frame before render)
    private func update() {
        autoreleasepool {
            // Call C++ update through bridge
            bridge?.update()
        }
    }

    /// Render logic (called every frame)
    private func render(view: MTKView) {
        autoreleasepool {
            guard let drawable = view.currentDrawable,
                  let renderPassDescriptor = view.currentRenderPassDescriptor,
                  let commandQueue = commandQueue else {
                return
            }

            // Phase 5.2: Call C++ renderFrame with Metal resources
            // All Metal rendering is handled by C++ MetalRenderer via DrawList
            bridge?.renderFrame(drawable,
                               renderPassDescriptor: renderPassDescriptor,
                               commandQueue: commandQueue)
        }
    }

    // MARK: - Public API

    func getFrameCount() -> UInt64 {
        return frameCount
    }

    func getElapsedTime() -> CFTimeInterval {
        return CACurrentMediaTime() - startTime
    }

    // MARK: - MouseEventReceiver (Phase 13.1)

    func mouseMoved(x: Float, y: Float) {
        // Forward to C++ bridge
        bridge?.mouseMovedX(x, y: y)
    }

    func mouseDragged(x: Float, y: Float, button: Int) {
        // Forward to C++ bridge
        bridge?.mouseDraggedX(x, y: y, button: Int32(button))
    }

    func mousePressed(x: Float, y: Float, button: Int) {
        // Forward to C++ bridge
        bridge?.mousePressedX(x, y: y, button: Int32(button))
    }

    func mouseReleased(x: Float, y: Float, button: Int) {
        // Forward to C++ bridge
        bridge?.mouseReleasedX(x, y: y, button: Int32(button))
    }

    func mouseScrolled(x: Float, y: Float, scrollX: Float, scrollY: Float) {
        // Forward to C++ bridge
        bridge?.mouseScrolledX(x, y: y, scrollX: scrollX, scrollY: scrollY)
    }

    func mouseEntered(x: Float, y: Float) {
        // Forward to C++ bridge
        bridge?.mouseEnteredX(x, y: y)
    }

    func mouseExited(x: Float, y: Float) {
        // Forward to C++ bridge
        bridge?.mouseExitedX(x, y: y)
    }

    // MARK: - Keyboard Events (Phase 13.2)

    func keyPressed(key: Int) {
        // Forward to C++ bridge
        bridge?.keyPressed(Int32(key))
    }

    func keyReleased(key: Int) {
        // Forward to C++ bridge
        bridge?.keyReleased(Int32(key))
    }

    // MARK: - Drag and Drop Events (Phase 13.3)

    func dragEvent(x: Float, y: Float) {
        // Forward to C++ bridge
        bridge?.dragEventX(x, y: y)
    }

    // MARK: - Window Callbacks (Phase 14.1)

    private func setupWindowCallbacks() {
        // Create static function pointer for window resize callback
        func windowResizeCallback(width: Int32, height: Int32) {
            print("[Swift] Window resize requested: \(width)x\(height)")
            // Note: Window resize is handled by SwiftUI's frame modifiers
        }

        // Create static function pointer for window position callback
        func windowPositionCallback(x: Int32, y: Int32) {
            print("[Swift] Window position change requested: \(x),\(y)")
            // Note: Window position is managed by SwiftUI/AppKit
        }

        // Create static function pointer for window title callback
        func windowTitleCallback(titlePtr: UnsafePointer<Int8>?) {
            guard let titlePtr = titlePtr else { return }
            let title = String(cString: titlePtr)
            DispatchQueue.main.async {
                print("[Swift] Window title changed to: \(title)")
            }
        }

        // Create static function pointer for fullscreen callback
        func fullscreenCallback(fullscreen: Bool) {
            DispatchQueue.main.async {
                print("[Swift] Fullscreen mode requested: \(fullscreen ? "ON" : "OFF")")
                // Toggle fullscreen using NSApp.mainWindow
                if let window = NSApp.mainWindow {
                    if fullscreen && !window.styleMask.contains(.fullScreen) {
                        window.toggleFullScreen(nil)
                    } else if !fullscreen && window.styleMask.contains(.fullScreen) {
                        window.toggleFullScreen(nil)
                    }
                    // Update C++ state to reflect actual fullscreen state
                    // Note: The actual state change happens asynchronously, so we update
                    // based on what we requested
                    MetalViewCoordinator.sharedCoordinator?.bridge?.setFullscreenState(fullscreen)
                }
            }
        }

        // Register callbacks (these are now non-capturing closures that can convert to C function pointers)
        bridge?.setWindowResizeCallback(windowResizeCallback)
        bridge?.setWindowPositionCallback(windowPositionCallback)
        bridge?.setWindowTitleCallback(windowTitleCallback)
        bridge?.setFullscreenCallback(fullscreenCallback)
    }

    // MARK: - Phase 16.2: Performance Monitoring

    /// Update performance statistics from C++ renderer
    private func updatePerformanceStats() {
        autoreleasepool {
            var drawCalls: UInt32 = 0
            var vertices: UInt32 = 0
            var gpuTime: Double = 0.0

            bridge?.getPerformanceStats(&drawCalls, vertices: &vertices, gpuTime: &gpuTime)

            // TODO Phase 16.2: Update global PerformanceStats when PerformanceMonitor.swift is added to project
            // PerformanceStats.shared.updateFrame(
            //     drawCalls: drawCalls,
            //     vertices: vertices,
            //     gpuTime: gpuTime
            // )
        }
    }
}
