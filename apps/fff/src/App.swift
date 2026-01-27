import SwiftUI

@main
struct FffApp: App {
    var body: some Scene {
        WindowGroup {
            MetalView()
                .frame(minWidth: 800, minHeight: 600)
        }
        .windowStyle(.titleBar)
    }
}
