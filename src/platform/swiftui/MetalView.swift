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
    private var library: MTLLibrary?
    private var pipelineState: MTLRenderPipelineState?
    private var frameCount: UInt64 = 0
    private var startTime: CFTimeInterval = 0

    // C++ Bridge
    private var bridge: OFLBridge?

    override init() {
        super.init()
        startTime = CACurrentMediaTime()
        bridge = OFLBridge()
    }

    func setup(device: MTLDevice) {
        self.device = device
        self.commandQueue = device.makeCommandQueue()

        // Load shader library
        guard let library = loadShaderLibrary(device: device) else {
            print("Warning: Failed to load shader library")
            return
        }
        self.library = library

        // Create render pipeline state
        guard let pipelineState = createBasic2DPipeline(device: device, library: library) else {
            print("Warning: Failed to create render pipeline state")
            return
        }
        self.pipelineState = pipelineState

        print("Metal initialization complete: Device, CommandQueue, Library, PipelineState")

        // Initialize C++ bridge
        bridge?.setup()
    }

    // MARK: - Metal Initialization

    /// Load the default Metal shader library
    private func loadShaderLibrary(device: MTLDevice) -> MTLLibrary? {
        do {
            // Try to load default library (compiled shaders)
            if let library = device.makeDefaultLibrary() {
                print("Loaded default shader library")
                return library
            }

            // Fallback: Try to load from file
            let shaderPath = Bundle.main.path(forResource: "default", ofType: "metallib")
            if let path = shaderPath {
                let library = try device.makeLibrary(filepath: path)
                print("Loaded shader library from: \(path)")
                return library
            }

            print("Error: Could not find shader library")
            return nil
        } catch {
            print("Error loading shader library: \(error)")
            return nil
        }
    }

    /// Create a basic 2D render pipeline state
    private func createBasic2DPipeline(device: MTLDevice, library: MTLLibrary) -> MTLRenderPipelineState? {
        let vertexFunction = library.makeFunction(name: "vertex2D")
        let fragmentFunction = library.makeFunction(name: "fragment2D")

        guard vertexFunction != nil && fragmentFunction != nil else {
            print("Error: Could not find vertex2D or fragment2D functions")
            return nil
        }

        let pipelineDescriptor = MTLRenderPipelineDescriptor()
        pipelineDescriptor.label = "Basic2D Pipeline"
        pipelineDescriptor.vertexFunction = vertexFunction
        pipelineDescriptor.fragmentFunction = fragmentFunction
        pipelineDescriptor.colorAttachments[0].pixelFormat = .bgra8Unorm
        pipelineDescriptor.depthAttachmentPixelFormat = .depth32Float

        // Configure blending for alpha transparency
        pipelineDescriptor.colorAttachments[0].isBlendingEnabled = true
        pipelineDescriptor.colorAttachments[0].rgbBlendOperation = .add
        pipelineDescriptor.colorAttachments[0].alphaBlendOperation = .add
        pipelineDescriptor.colorAttachments[0].sourceRGBBlendFactor = .sourceAlpha
        pipelineDescriptor.colorAttachments[0].sourceAlphaBlendFactor = .sourceAlpha
        pipelineDescriptor.colorAttachments[0].destinationRGBBlendFactor = .oneMinusSourceAlpha
        pipelineDescriptor.colorAttachments[0].destinationAlphaBlendFactor = .oneMinusSourceAlpha

        do {
            let pipelineState = try device.makeRenderPipelineState(descriptor: pipelineDescriptor)
            print("Created Basic2D render pipeline state")
            return pipelineState
        } catch {
            print("Error creating pipeline state: \(error)")
            return nil
        }
    }

    // MARK: - MTKViewDelegate

    /// Called whenever view changes orientation or is resized
    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
        // Handle resize
        print("MetalView resized to: \(size.width) x \(size.height)")

        // Notify C++ layer through bridge
        bridge?.windowResized(width: Float(size.width), height: Float(size.height))
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

            // Execute C++ draw commands through bridge
            bridge?.draw()

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
