import SwiftUI
import MetalKit

@main
struct AutoXcodeGenTestApp: App {
    @StateObject private var appState = AppState()

    var body: some Scene {
        WindowGroup {
            MetalView(appState: appState)
                .frame(minWidth: 800, minHeight: 600)
        }
        .windowStyle(.titleBar)
    }
}

class AppState: ObservableObject {
    @Published var isRunning = true
}
