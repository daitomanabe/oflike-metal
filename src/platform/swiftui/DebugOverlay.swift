import SwiftUI

// MARK: - Debug Parameter Types

/// Protocol for debug parameters that can be exposed to SwiftUI
protocol DebugParameter {
    var name: String { get }
    var group: String { get }
}

/// Float parameter with range
class FloatParameter: ObservableObject, DebugParameter {
    let name: String
    let group: String
    @Published var value: Float
    let min: Float
    let max: Float

    init(name: String, group: String, value: Float, min: Float = 0.0, max: Float = 1.0) {
        self.name = name
        self.group = group
        self.value = value
        self.min = min
        self.max = max
    }
}

/// Boolean parameter
class BoolParameter: ObservableObject, DebugParameter {
    let name: String
    let group: String
    @Published var value: Bool

    init(name: String, group: String, value: Bool) {
        self.name = name
        self.group = group
        self.value = value
    }
}

/// Color parameter (RGB 0-255)
class ColorParameter: ObservableObject, DebugParameter {
    let name: String
    let group: String
    @Published var color: Color

    init(name: String, group: String, r: Int, g: Int, b: Int) {
        self.name = name
        self.group = group
        self.color = Color(
            red: Double(r) / 255.0,
            green: Double(g) / 255.0,
            blue: Double(b) / 255.0
        )
    }
}

/// Int parameter with range
class IntParameter: ObservableObject, DebugParameter {
    let name: String
    let group: String
    @Published var value: Int
    let min: Int
    let max: Int

    init(name: String, group: String, value: Int, min: Int = 0, max: Int = 100) {
        self.name = name
        self.group = group
        self.value = value
        self.min = min
        self.max = max
    }
}

// MARK: - Debug Parameter Store

/// Central store for all debug parameters
class DebugParameterStore: ObservableObject {
    static let shared = DebugParameterStore()

    @Published var floatParams: [FloatParameter] = []
    @Published var boolParams: [BoolParameter] = []
    @Published var colorParams: [ColorParameter] = []
    @Published var intParams: [IntParameter] = []

    private init() {}

    /// Register a float parameter
    func registerFloat(name: String, group: String = "General", value: Float, min: Float = 0.0, max: Float = 1.0) -> FloatParameter {
        let param = FloatParameter(name: name, group: group, value: value, min: min, max: max)
        floatParams.append(param)
        return param
    }

    /// Register a boolean parameter
    func registerBool(name: String, group: String = "General", value: Bool) -> BoolParameter {
        let param = BoolParameter(name: name, group: group, value: value)
        boolParams.append(param)
        return param
    }

    /// Register a color parameter
    func registerColor(name: String, group: String = "General", r: Int, g: Int, b: Int) -> ColorParameter {
        let param = ColorParameter(name: name, group: group, r: r, g: g, b: b)
        colorParams.append(param)
        return param
    }

    /// Register an int parameter
    func registerInt(name: String, group: String = "General", value: Int, min: Int = 0, max: Int = 100) -> IntParameter {
        let param = IntParameter(name: name, group: group, value: value, min: min, max: max)
        intParams.append(param)
        return param
    }

    /// Get all unique group names
    var groups: [String] {
        var groupSet = Set<String>()
        floatParams.forEach { groupSet.insert($0.group) }
        boolParams.forEach { groupSet.insert($0.group) }
        colorParams.forEach { groupSet.insert($0.group) }
        intParams.forEach { groupSet.insert($0.group) }
        return Array(groupSet).sorted()
    }

    /// Clear all parameters (useful for cleanup)
    func clear() {
        floatParams.removeAll()
        boolParams.removeAll()
        colorParams.removeAll()
        intParams.removeAll()
    }
}

// MARK: - Debug Overlay View

/// SwiftUI Debug Overlay for runtime parameter adjustment
/// Only available in DEBUG builds
struct DebugOverlay: View {
    @ObservedObject var store = DebugParameterStore.shared
    @State private var isExpanded = true
    @State private var selectedGroup: String?

    var body: some View {
        #if DEBUG
        VStack(alignment: .leading, spacing: 0) {
            // Header
            HStack {
                Image(systemName: "gearshape.fill")
                    .foregroundColor(.blue)
                Text("Debug Parameters")
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

                // Group tabs
                if !store.groups.isEmpty {
                    ScrollView(.horizontal, showsIndicators: false) {
                        HStack(spacing: 8) {
                            ForEach(store.groups, id: \.self) { group in
                                Button(action: {
                                    selectedGroup = (selectedGroup == group) ? nil : group
                                }) {
                                    Text(group)
                                        .font(.system(size: 12, weight: .medium))
                                        .padding(.horizontal, 12)
                                        .padding(.vertical, 6)
                                        .background(
                                            selectedGroup == group || selectedGroup == nil
                                                ? Color.blue.opacity(0.2)
                                                : Color.clear
                                        )
                                        .cornerRadius(6)
                                }
                                .buttonStyle(.plain)
                            }
                        }
                        .padding(.horizontal, 12)
                        .padding(.vertical, 8)
                    }
                    .frame(height: 36)

                    Divider()
                }

                // Parameters list
                ScrollView {
                    VStack(alignment: .leading, spacing: 12) {
                        // Float parameters
                        ForEach(filteredFloatParams, id: \.name) { param in
                            FloatParameterView(parameter: param)
                        }

                        // Int parameters
                        ForEach(filteredIntParams, id: \.name) { param in
                            IntParameterView(parameter: param)
                        }

                        // Bool parameters
                        ForEach(filteredBoolParams, id: \.name) { param in
                            BoolParameterView(parameter: param)
                        }

                        // Color parameters
                        ForEach(filteredColorParams, id: \.name) { param in
                            ColorParameterView(parameter: param)
                        }

                        if filteredFloatParams.isEmpty && filteredIntParams.isEmpty &&
                           filteredBoolParams.isEmpty && filteredColorParams.isEmpty {
                            Text("No parameters registered")
                                .font(.system(size: 12))
                                .foregroundColor(.secondary)
                                .frame(maxWidth: .infinity, alignment: .center)
                                .padding(.vertical, 20)
                        }
                    }
                    .padding(12)
                }
                .frame(maxHeight: 400)
            }
        }
        .frame(minWidth: 280, maxWidth: 320)
        .background(.ultraThinMaterial)
        .cornerRadius(12)
        .shadow(radius: 10)
        .onAppear {
            // Select first group by default if none selected
            if selectedGroup == nil && !store.groups.isEmpty {
                selectedGroup = nil // Show all by default
            }
        }
        #else
        EmptyView()
        #endif
    }

    // Filter parameters by selected group
    private var filteredFloatParams: [FloatParameter] {
        guard let group = selectedGroup else { return store.floatParams }
        return store.floatParams.filter { $0.group == group }
    }

    private var filteredIntParams: [IntParameter] {
        guard let group = selectedGroup else { return store.intParams }
        return store.intParams.filter { $0.group == group }
    }

    private var filteredBoolParams: [BoolParameter] {
        guard let group = selectedGroup else { return store.boolParams }
        return store.boolParams.filter { $0.group == group }
    }

    private var filteredColorParams: [ColorParameter] {
        guard let group = selectedGroup else { return store.colorParams }
        return store.colorParams.filter { $0.group == group }
    }
}

// MARK: - Parameter Views

/// View for float parameter with slider
private struct FloatParameterView: View {
    @ObservedObject var parameter: FloatParameter

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            HStack {
                Text(parameter.name)
                    .font(.system(size: 12, weight: .medium))
                Spacer()
                Text(String(format: "%.2f", parameter.value))
                    .font(.system(size: 11, design: .monospaced))
                    .foregroundColor(.secondary)
            }

            Slider(value: $parameter.value, in: parameter.min...parameter.max)
                .controlSize(.small)
        }
    }
}

/// View for int parameter with slider
private struct IntParameterView: View {
    @ObservedObject var parameter: IntParameter

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            HStack {
                Text(parameter.name)
                    .font(.system(size: 12, weight: .medium))
                Spacer()
                Text("\(parameter.value)")
                    .font(.system(size: 11, design: .monospaced))
                    .foregroundColor(.secondary)
            }

            Slider(value: Binding(
                get: { Double(parameter.value) },
                set: { parameter.value = Int($0) }
            ), in: Double(parameter.min)...Double(parameter.max), step: 1.0)
                .controlSize(.small)
        }
    }
}

/// View for boolean parameter with toggle
private struct BoolParameterView: View {
    @ObservedObject var parameter: BoolParameter

    var body: some View {
        HStack {
            Text(parameter.name)
                .font(.system(size: 12, weight: .medium))
            Spacer()
            Toggle("", isOn: $parameter.value)
                .labelsHidden()
                .controlSize(.small)
        }
    }
}

/// View for color parameter with color picker
private struct ColorParameterView: View {
    @ObservedObject var parameter: ColorParameter

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(parameter.name)
                .font(.system(size: 12, weight: .medium))

            HStack(spacing: 8) {
                // Color preview
                RoundedRectangle(cornerRadius: 4)
                    .fill(parameter.color)
                    .frame(width: 32, height: 32)
                    .overlay(
                        RoundedRectangle(cornerRadius: 4)
                            .stroke(Color.primary.opacity(0.2), lineWidth: 1)
                    )

                // Color picker
                ColorPicker("", selection: $parameter.color, supportsOpacity: false)
                    .labelsHidden()
                    .frame(maxWidth: .infinity)
            }
        }
    }
}

// MARK: - Preview

#if DEBUG
struct DebugOverlay_Previews: PreviewProvider {
    static var previews: some View {
        // Setup preview data
        let store = DebugParameterStore.shared
        store.clear()
        _ = store.registerFloat(name: "Radius", group: "Shapes", value: 50.0, min: 10.0, max: 200.0)
        _ = store.registerFloat(name: "Speed", group: "Animation", value: 1.0, min: 0.1, max: 10.0)
        _ = store.registerBool(name: "Wireframe", group: "Rendering", value: false)
        _ = store.registerBool(name: "Show FPS", group: "Rendering", value: true)
        _ = store.registerColor(name: "Background", group: "Colors", r: 0, g: 0, b: 0)
        _ = store.registerColor(name: "Foreground", group: "Colors", r: 255, g: 0, b: 0)
        _ = store.registerInt(name: "Resolution", group: "Shapes", value: 32, min: 8, max: 128)

        return ZStack(alignment: .topTrailing) {
            Color.black.ignoresSafeArea()

            DebugOverlay()
                .padding(20)
        }
        .frame(width: 800, height: 600)
    }
}
#endif
