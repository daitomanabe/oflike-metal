import SwiftUI
import MetalKit

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
        let mtkView = MTKView()

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

        // Initialize coordinator with device
        coordinator.setup(device: device)

        return mtkView
    }

    func updateNSView(_ nsView: MTKView, context: Context) {
        // Update view if needed
    }
}

/// Coordinator that implements MTKViewDelegate
class MetalViewCoordinator: NSObject, MTKViewDelegate, ObservableObject {
    private var device: MTLDevice?
    private var commandQueue: MTLCommandQueue?
    private var library: MTLLibrary?
    private var pipelineState: MTLRenderPipelineState?

    // Phase 1.5: Frame tracking for FPS display
    @Published var frameCount: UInt64 = 0
    @Published var currentFPS: Double = 0.0
    private var startTime: CFTimeInterval = 0
    private var lastFPSUpdate: CFTimeInterval = 0
    private var framesSinceLastUpdate: Int = 0

    // C++ Bridge
    private var bridge: OFLBridge?

    override init() {
        super.init()
        startTime = CACurrentMediaTime()
        lastFPSUpdate = startTime
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
                  let commandQueue = commandQueue,
                  let pipelineState = pipelineState,
                  let device = device else {
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

            // Phase 1.5: Render test triangle
            renderTestTriangle(encoder: renderEncoder, device: device, pipelineState: pipelineState)

            // End encoding
            renderEncoder.endEncoding()

            // Present drawable
            commandBuffer.present(drawable)

            // Commit command buffer
            commandBuffer.commit()
        }
    }

    // MARK: - Phase 1.5 Triangle Test

    /// Render a simple test triangle (Phase 1.5 verification)
    private func renderTestTriangle(
        encoder: MTLRenderCommandEncoder,
        device: MTLDevice,
        pipelineState: MTLRenderPipelineState
    ) {
        // Define triangle vertices (Vertex2D structure from Common.h)
        // position (float2), texCoord (float2), color (float4)
        let vertices: [Float] = [
            // Position      TexCoord    Color (RGBA)
            0.0,  0.5,      0.5, 0.0,   1.0, 0.0, 0.0, 1.0,  // Top (red)
           -0.5, -0.5,      0.0, 1.0,   0.0, 1.0, 0.0, 1.0,  // Bottom-left (green)
            0.5, -0.5,      1.0, 1.0,   0.0, 0.0, 1.0, 1.0,  // Bottom-right (blue)
        ]

        // Create vertex buffer
        guard let vertexBuffer = device.makeBuffer(
            bytes: vertices,
            length: vertices.count * MemoryLayout<Float>.stride,
            options: .cpuCacheModeWriteCombined
        ) else {
            return
        }

        // Create identity uniforms (Uniforms2D from Common.h)
        var uniforms: [Float] = [
            // Projection matrix (identity)
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            // ModelView matrix (identity)
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
        ]

        guard let uniformBuffer = device.makeBuffer(
            bytes: &uniforms,
            length: uniforms.count * MemoryLayout<Float>.stride,
            options: .cpuCacheModeWriteCombined
        ) else {
            return
        }

        // Set render pipeline state
        encoder.setRenderPipelineState(pipelineState)

        // Set vertex buffers
        encoder.setVertexBuffer(vertexBuffer, offset: 0, index: 0)
        encoder.setVertexBuffer(uniformBuffer, offset: 0, index: 1)

        // Draw triangle
        encoder.drawPrimitives(type: .triangle, vertexStart: 0, vertexCount: 3)
    }

    // MARK: - Public API

    func getFrameCount() -> UInt64 {
        return frameCount
    }

    func getElapsedTime() -> CFTimeInterval {
        return CACurrentMediaTime() - startTime
    }
}
