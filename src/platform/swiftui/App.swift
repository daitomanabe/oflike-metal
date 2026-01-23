import SwiftUI

@main
struct OFLikeApp: App {
    var body: some Scene {
        WindowGroup {
            ContentView()
                .frame(minWidth: 800, minHeight: 600)
        }
        .windowStyle(.hiddenTitleBar)
        .windowResizability(.contentSize)
    }
}

struct ContentView: View {
    var body: some View {
        ZStack {
            Color.black
            Text("oflike-metal")
                .foregroundColor(.white)
                .font(.system(size: 32, weight: .light))
        }
    }
}
