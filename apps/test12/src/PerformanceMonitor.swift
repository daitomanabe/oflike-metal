import SwiftUI
import QuartzCore

// MARK: - Performance Statistics

/// Performance statistics for monitoring rendering performance
class PerformanceStats: ObservableObject {
    static let shared = PerformanceStats()

    @Published var fps: Double = 0.0
    @Published var frameTime: Double = 0.0  // milliseconds
    @Published var drawCalls: UInt32 = 0
    @Published var vertexCount: UInt32 = 0
    @Published var gpuTime: Double = 0.0    // milliseconds

    private var lastUpdateTime: CFTimeInterval = 0
    private var frameCount: Int = 0
    private var fpsAccumulator: Double = 0.0
    private let fpsUpdateInterval: CFTimeInterval = 0.5  // Update every 0.5 seconds

    private init() {
        lastUpdateTime = CACurrentMediaTime()
    }

    /// Update frame statistics (called every frame)
    func updateFrame(drawCalls: UInt32, vertices: UInt32, gpuTime: Double) {
        let currentTime = CACurrentMediaTime()
        let deltaTime = currentTime - lastUpdateTime

        frameCount += 1

        // Calculate instantaneous FPS
        let instantFPS = 1.0 / deltaTime
        fpsAccumulator += instantFPS

        // Update published values every interval
        if deltaTime >= fpsUpdateInterval {
            self.fps = fpsAccumulator / Double(frameCount)
            self.frameTime = (deltaTime / Double(frameCount)) * 1000.0  // Convert to ms

            // Reset for next interval
            fpsAccumulator = 0.0
            frameCount = 0
            lastUpdateTime = currentTime
        }

        // Always update per-frame stats
        self.drawCalls = drawCalls
        self.vertexCount = vertices
        self.gpuTime = gpuTime
    }

    /// Reset all statistics
    func reset() {
        fps = 0.0
        frameTime = 0.0
        drawCalls = 0
        vertexCount = 0
        gpuTime = 0.0
        frameCount = 0
        fpsAccumulator = 0.0
        lastUpdateTime = CACurrentMediaTime()
    }
}

// MARK: - Performance Monitor View

/// SwiftUI view for displaying real-time performance statistics
/// Only available in DEBUG builds
struct PerformanceMonitor: View {
    @ObservedObject var stats = PerformanceStats.shared
    @State private var isExpanded = true

    var body: some View {
        #if DEBUG
        VStack(alignment: .leading, spacing: 0) {
            // Header
            HStack {
                Image(systemName: "gauge")
                    .foregroundColor(.green)
                Text("Performance")
                    .font(.system(size: 14, weight: .semibold))
                Spacer()
                Button(action: {
                    withAnimation(.easeInOut(duration: 0.2)) {
                        isExpanded.toggle()
                    }
                }) {
                    Image(systemName: isExpanded ? "chevron.up" : "chevron.down")
                        .foregroundColor(.primary)
                }
                .buttonStyle(.plain)
            }
            .padding(12)

            if isExpanded {
                Divider()

                // Performance metrics
                VStack(alignment: .leading, spacing: 10) {
                    // FPS
                    HStack {
                        Text("FPS")
                            .font(.system(size: 12, weight: .medium))
                        Spacer()
                        Text(String(format: "%.1f", stats.fps))
                            .font(.system(size: 12, design: .monospaced))
                            .foregroundColor(fpsColor)
                    }

                    // Frame Time
                    HStack {
                        Text("Frame Time")
                            .font(.system(size: 12, weight: .medium))
                        Spacer()
                        Text(String(format: "%.2f ms", stats.frameTime))
                            .font(.system(size: 12, design: .monospaced))
                            .foregroundColor(frameTimeColor)
                    }

                    Divider()
                        .padding(.vertical, 4)

                    // Draw Calls
                    HStack {
                        Text("Draw Calls")
                            .font(.system(size: 12, weight: .medium))
                        Spacer()
                        Text("\(stats.drawCalls)")
                            .font(.system(size: 12, design: .monospaced))
                            .foregroundColor(drawCallsColor)
                    }

                    // Vertex Count
                    HStack {
                        Text("Vertices")
                            .font(.system(size: 12, weight: .medium))
                        Spacer()
                        Text(formatNumber(stats.vertexCount))
                            .font(.system(size: 12, design: .monospaced))
                            .foregroundColor(.secondary)
                    }

                    Divider()
                        .padding(.vertical, 4)

                    // GPU Time
                    HStack {
                        Text("GPU Time")
                            .font(.system(size: 12, weight: .medium))
                        Spacer()
                        Text(String(format: "%.2f ms", stats.gpuTime))
                            .font(.system(size: 12, design: .monospaced))
                            .foregroundColor(.secondary)
                    }
                }
                .padding(12)
            }
        }
        .frame(minWidth: 200, maxWidth: 240)
        .background(.ultraThinMaterial)
        .cornerRadius(12)
        .shadow(radius: 10)
        #else
        EmptyView()
        #endif
    }

    // Color coding for FPS
    private var fpsColor: Color {
        if stats.fps >= 58.0 {
            return .green
        } else if stats.fps >= 30.0 {
            return .orange
        } else {
            return .red
        }
    }

    // Color coding for frame time
    private var frameTimeColor: Color {
        if stats.frameTime <= 17.0 {  // ~60 FPS
            return .green
        } else if stats.frameTime <= 33.0 {  // ~30 FPS
            return .orange
        } else {
            return .red
        }
    }

    // Color coding for draw calls
    private var drawCallsColor: Color {
        if stats.drawCalls < 300 {
            return .green
        } else if stats.drawCalls < 500 {
            return .orange
        } else {
            return .red
        }
    }

    // Format large numbers with K/M suffix
    private func formatNumber(_ num: UInt32) -> String {
        if num >= 1_000_000 {
            return String(format: "%.1fM", Double(num) / 1_000_000.0)
        } else if num >= 1_000 {
            return String(format: "%.1fK", Double(num) / 1_000.0)
        } else {
            return "\(num)"
        }
    }
}

// MARK: - Preview

#if DEBUG
struct PerformanceMonitor_Previews: PreviewProvider {
    static var previews: some View {
        // Setup preview data
        let stats = PerformanceStats.shared
        stats.fps = 59.8
        stats.frameTime = 16.7
        stats.drawCalls = 127
        stats.vertexCount = 45_621
        stats.gpuTime = 8.3

        return ZStack(alignment: .topLeading) {
            Color.black.ignoresSafeArea()

            PerformanceMonitor()
                .padding(20)
        }
        .frame(width: 400, height: 300)
    }
}
#endif
