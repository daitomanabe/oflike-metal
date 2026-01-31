import SwiftUI

@main
struct TestVboMeshApp: App {
    var body: some Scene {
        WindowGroup {
            MetalView()
                .frame(minWidth: 900, minHeight: 700)
        }
        .windowStyle(.titleBar)
    }
}
