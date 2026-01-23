import SwiftUI
import MetalKit

/// SwiftUI wrapper for MTKView
struct MetalView: NSViewRepresentable {
    @StateObject private var coordinator = MetalViewCoordinator()

    func makeNSView(context: Context) -> MTKView {
        let mtkView = MTKView()

        // Get Metal device
        guard let device = MTLCreateSystemDefaultDevice() else {
            fatalError("Metal is not supported on this device")
        }

        mtkView.device = device
        mtkView.delegate = context.coordinator
        mtkView.clearColor = MTLClearColor(red: 0.0, green: 0.0, blue: 0.0, alpha: 1.0)
        mtkView.colorPixelFormat = .bgra8Unorm
        mtkView.depthStencilPixelFormat = .depth32Float
        mtkView.framebufferOnly = false
        mtkView.enableSetNeedsDisplay = false
        mtkView.isPaused = false
        mtkView.preferredFramesPerSecond = 60

        // Initialize coordinator with device
        context.coordinator.setup(device: device)

        return mtkView
    }

    func updateNSView(_ nsView: MTKView, context: Context) {
        // Update view if needed
    }

    func makeCoordinator() -> MetalViewCoordinator {
        return coordinator
    }
}

/// Coordinator that implements MTKViewDelegate
class MetalViewCoordinator: NSObject, MTKViewDelegate, ObservableObject {
    private var device: MTLDevice?
    private var commandQueue: MTLCommandQueue?
    private var frameCount: UInt64 = 0
    private var startTime: CFTimeInterval = 0

    override init() {
        super.init()
        startTime = CACurrentMediaTime()
    }

    func setup(device: MTLDevice) {
        self.device = device
        self.commandQueue = device.makeCommandQueue()
    }

    // MARK: - MTKViewDelegate

    /// Called whenever view changes orientation or is resized
    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
        // Handle resize
        // TODO: Notify C++ layer about window resize
        print("MetalView resized to: \(size.width) x \(size.height)")
    }

    /// Called whenever the view needs to render a frame
    func draw(in view: MTKView) {
        // Frame update logic
        update()

        // Render frame
        render(view: view)

        frameCount += 1
    }

    // MARK: - Frame Loop

    /// Update logic (called every frame before render)
    private func update() {
        autoreleasepool {
            // TODO: Call C++ update() callback
            let elapsed = CACurrentMediaTime() - startTime
            // Placeholder for C++ bridge call
            // bridge.update(elapsed: elapsed)
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

            // Create command buffer
            guard let commandBuffer = commandQueue.makeCommandBuffer() else {
                return
            }

            // Create render command encoder
            guard let renderEncoder = commandBuffer.makeRenderCommandEncoder(
                descriptor: renderPassDescriptor
            ) else {
                return
            }

            // TODO: Execute C++ draw commands
            // bridge.draw()

            // End encoding
            renderEncoder.endEncoding()

            // Present drawable
            commandBuffer.present(drawable)

            // Commit command buffer
            commandBuffer.commit()
        }
    }

    // MARK: - Public API

    func getFrameCount() -> UInt64 {
        return frameCount
    }

    func getElapsedTime() -> CFTimeInterval {
        return CACurrentMediaTime() - startTime
    }
}
