import SwiftUI

/// Phase 11.2: Performance statistics validation
/// This app displays the PerformanceMonitor overlay to verify that
/// drawCalls, vertices, and gpuTime are non-zero during rendering
@main
struct PerformanceTestApp: App {
    var body: some Scene {
        WindowGroup {
            ZStack(alignment: .topLeading) {
                // Main Metal rendering view
                MetalView()

                // Performance overlay (always visible for validation)
                PerformanceMonitor()
                    .padding(16)
            }
            .frame(minWidth: 800, minHeight: 600)
        }
        .windowStyle(.titleBar)
    }
}
