import SwiftUI

@main
struct TestPhase1App: App {
    var body: some Scene {
        WindowGroup {
            MetalView()
                .frame(minWidth: 900, minHeight: 700)
        }
        .windowStyle(.titleBar)
    }
}
