import SwiftUI

@main
struct ValidationApp: App {
    var body: some Scene {
        WindowGroup {
            MetalView()
                .frame(minWidth: 800, minHeight: 600)
        }
        .windowStyle(.titleBar)
    }
}
