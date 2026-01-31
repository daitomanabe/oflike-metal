import SwiftUI

@main
struct TestPhase3App: App {
    var body: some Scene {
        WindowGroup {
            MetalView()
                .frame(minWidth: 900, minHeight: 700)
        }
        .windowStyle(.titleBar)
    }
}
