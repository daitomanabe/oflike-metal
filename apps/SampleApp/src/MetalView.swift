import SwiftUI
import MetalKit

struct MetalView: View {
    @ObservedObject var appState: AppState

    var body: some View {
        MetalViewRepresentable()
    }
}

private struct MetalViewRepresentable: NSViewRepresentable {
    func makeNSView(context: Context) -> MTKView {
        let view = MTKView()
        view.device = MTLCreateSystemDefaultDevice()
        return view
    }

    func updateNSView(_ nsView: MTKView, context: Context) {
        // No-op
    }
}
