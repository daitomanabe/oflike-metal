import SwiftUI

// MARK: - GUI Parameter Types

/// Protocol for GUI parameters that can be exposed to SwiftUI
public protocol GuiParameter {
    var name: String { get }
    var group: String { get }
}

/// Float parameter with range for GUI
public class GuiFloatParameter: ObservableObject, GuiParameter {
    public let name: String
    public let group: String
    @Published public var value: Float
    public let min: Float
    public let max: Float

    public init(name: String, group: String, value: Float, min: Float = 0.0, max: Float = 1.0) {
        self.name = name
        self.group = group
        self.value = value
        self.min = min
        self.max = max
    }
}

/// Int parameter with range for GUI
public class GuiIntParameter: ObservableObject, GuiParameter {
    public let name: String
    public let group: String
    @Published public var value: Int
    public let min: Int
    public let max: Int

    public init(name: String, group: String, value: Int, min: Int = 0, max: Int = 100) {
        self.name = name
        self.group = group
        self.value = value
        self.min = min
        self.max = max
    }
}

/// Boolean parameter for GUI
public class GuiBoolParameter: ObservableObject, GuiParameter {
    public let name: String
    public let group: String
    @Published public var value: Bool

    public init(name: String, group: String, value: Bool) {
        self.name = name
        self.group = group
        self.value = value
    }
}

/// Color parameter for GUI (RGB 0-255)
public class GuiColorParameter: ObservableObject, GuiParameter {
    public let name: String
    public let group: String
    @Published public var color: Color

    public init(name: String, group: String, r: Int, g: Int, b: Int) {
        self.name = name
        self.group = group
        self.color = Color(
            red: Double(r) / 255.0,
            green: Double(g) / 255.0,
            blue: Double(b) / 255.0
        )
    }

    /// Get RGB values as 0-255 integers
    public var rgb: (r: Int, g: Int, b: Int) {
        #if canImport(AppKit)
        if let nsColor = NSColor(color) {
            return (
                r: Int(nsColor.redComponent * 255.0),
                g: Int(nsColor.greenComponent * 255.0),
                b: Int(nsColor.blueComponent * 255.0)
            )
        }
        #endif
        return (r: 0, g: 0, b: 0)
    }
}

/// Button parameter for GUI (callback-based)
public class GuiButtonParameter: ObservableObject, GuiParameter {
    public let name: String
    public let group: String
    public var action: (() -> Void)?

    public init(name: String, group: String, action: (() -> Void)? = nil) {
        self.name = name
        self.group = group
        self.action = action
    }
}

/// String parameter for GUI (text field)
public class GuiStringParameter: ObservableObject, GuiParameter {
    public let name: String
    public let group: String
    @Published public var value: String

    public init(name: String, group: String, value: String) {
        self.name = name
        self.group = group
        self.value = value
    }
}

// MARK: - GUI Parameter Store

/// Central store for all GUI parameters
/// This is the bridge between C++ and SwiftUI
public class GuiParameterStore: ObservableObject {
    public static let shared = GuiParameterStore()

    @Published public var floatParams: [GuiFloatParameter] = []
    @Published public var intParams: [GuiIntParameter] = []
    @Published public var boolParams: [GuiBoolParameter] = []
    @Published public var colorParams: [GuiColorParameter] = []
    @Published public var buttonParams: [GuiButtonParameter] = []
    @Published public var stringParams: [GuiStringParameter] = []

    private init() {}

    // MARK: - Registration Methods

    /// Register a float parameter
    public func registerFloat(name: String, group: String = "General", value: Float, min: Float = 0.0, max: Float = 1.0) -> GuiFloatParameter {
        let param = GuiFloatParameter(name: name, group: group, value: value, min: min, max: max)
        floatParams.append(param)
        return param
    }

    /// Register an int parameter
    public func registerInt(name: String, group: String = "General", value: Int, min: Int = 0, max: Int = 100) -> GuiIntParameter {
        let param = GuiIntParameter(name: name, group: group, value: value, min: min, max: max)
        intParams.append(param)
        return param
    }

    /// Register a boolean parameter
    public func registerBool(name: String, group: String = "General", value: Bool) -> GuiBoolParameter {
        let param = GuiBoolParameter(name: name, group: group, value: value)
        boolParams.append(param)
        return param
    }

    /// Register a color parameter
    public func registerColor(name: String, group: String = "General", r: Int, g: Int, b: Int) -> GuiColorParameter {
        let param = GuiColorParameter(name: name, group: group, r: r, g: g, b: b)
        colorParams.append(param)
        return param
    }

    /// Register a button parameter
    public func registerButton(name: String, group: String = "General", action: (() -> Void)? = nil) -> GuiButtonParameter {
        let param = GuiButtonParameter(name: name, group: group, action: action)
        buttonParams.append(param)
        return param
    }

    /// Register a string parameter
    public func registerString(name: String, group: String = "General", value: String) -> GuiStringParameter {
        let param = GuiStringParameter(name: name, group: group, value: value)
        stringParams.append(param)
        return param
    }

    // MARK: - Query Methods

    /// Get all unique group names
    public var groups: [String] {
        var groupSet = Set<String>()
        floatParams.forEach { groupSet.insert($0.group) }
        intParams.forEach { groupSet.insert($0.group) }
        boolParams.forEach { groupSet.insert($0.group) }
        colorParams.forEach { groupSet.insert($0.group) }
        buttonParams.forEach { groupSet.insert($0.group) }
        stringParams.forEach { groupSet.insert($0.group) }
        return Array(groupSet).sorted()
    }

    /// Clear all parameters
    public func clear() {
        floatParams.removeAll()
        intParams.removeAll()
        boolParams.removeAll()
        colorParams.removeAll()
        buttonParams.removeAll()
        stringParams.removeAll()
    }
}

// MARK: - OFLGuiPanel View

/// Main GUI Panel View for ofxGui
/// SwiftUI-based parameter panel with .ultraThinMaterial background
public struct OFLGuiPanel: View {
    @ObservedObject var store = GuiParameterStore.shared
    @State private var isExpanded = true
    @State private var selectedGroup: String?
    @State private var panelWidth: CGFloat = 300

    public init() {}

    public var body: some View {
        VStack(alignment: .leading, spacing: 0) {
            // Header
            headerView

            if isExpanded {
                Divider()

                // Group tabs
                if !store.groups.isEmpty {
                    groupTabsView
                    Divider()
                }

                // Parameters list
                parametersScrollView
            }
        }
        .frame(minWidth: 250, idealWidth: panelWidth, maxWidth: 400)
        .background(.ultraThinMaterial) // macOS-style translucent background
        .cornerRadius(12)
        .shadow(color: Color.black.opacity(0.2), radius: 10, x: 0, y: 5)
    }

    // MARK: - Subviews

    private var headerView: some View {
        HStack {
            Image(systemName: "slider.horizontal.3")
                .foregroundColor(.blue)
            Text("GUI Panel")
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
    }

    private var groupTabsView: some View {
        ScrollView(.horizontal, showsIndicators: false) {
            HStack(spacing: 8) {
                // "All" tab
                Button(action: {
                    selectedGroup = nil
                }) {
                    Text("All")
                        .font(.system(size: 12, weight: .medium))
                        .padding(.horizontal, 12)
                        .padding(.vertical, 6)
                        .background(
                            selectedGroup == nil
                                ? Color.blue.opacity(0.2)
                                : Color.clear
                        )
                        .cornerRadius(6)
                }
                .buttonStyle(.plain)

                // Group tabs
                ForEach(store.groups, id: \.self) { group in
                    Button(action: {
                        selectedGroup = group
                    }) {
                        Text(group)
                            .font(.system(size: 12, weight: .medium))
                            .padding(.horizontal, 12)
                            .padding(.vertical, 6)
                            .background(
                                selectedGroup == group
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
    }

    private var parametersScrollView: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 12) {
                // Float parameters
                ForEach(filteredFloatParams, id: \.name) { param in
                    FloatSliderView(parameter: param)
                }

                // Int parameters
                ForEach(filteredIntParams, id: \.name) { param in
                    IntSliderView(parameter: param)
                }

                // Bool parameters
                ForEach(filteredBoolParams, id: \.name) { param in
                    BoolToggleView(parameter: param)
                }

                // Color parameters
                ForEach(filteredColorParams, id: \.name) { param in
                    ColorPickerView(parameter: param)
                }

                // Button parameters
                ForEach(filteredButtonParams, id: \.name) { param in
                    ButtonView(parameter: param)
                }

                // String parameters
                ForEach(filteredStringParams, id: \.name) { param in
                    TextFieldView(parameter: param)
                }

                // Empty state
                if filteredFloatParams.isEmpty && filteredIntParams.isEmpty &&
                   filteredBoolParams.isEmpty && filteredColorParams.isEmpty &&
                   filteredButtonParams.isEmpty && filteredStringParams.isEmpty {
                    Text("No parameters")
                        .font(.system(size: 12))
                        .foregroundColor(.secondary)
                        .frame(maxWidth: .infinity, alignment: .center)
                        .padding(.vertical, 20)
                }
            }
            .padding(12)
        }
        .frame(maxHeight: 500)
    }

    // MARK: - Filtered Parameters

    private var filteredFloatParams: [GuiFloatParameter] {
        guard let group = selectedGroup else { return store.floatParams }
        return store.floatParams.filter { $0.group == group }
    }

    private var filteredIntParams: [GuiIntParameter] {
        guard let group = selectedGroup else { return store.intParams }
        return store.intParams.filter { $0.group == group }
    }

    private var filteredBoolParams: [GuiBoolParameter] {
        guard let group = selectedGroup else { return store.boolParams }
        return store.boolParams.filter { $0.group == group }
    }

    private var filteredColorParams: [GuiColorParameter] {
        guard let group = selectedGroup else { return store.colorParams }
        return store.colorParams.filter { $0.group == group }
    }

    private var filteredButtonParams: [GuiButtonParameter] {
        guard let group = selectedGroup else { return store.buttonParams }
        return store.buttonParams.filter { $0.group == group }
    }

    private var filteredStringParams: [GuiStringParameter] {
        guard let group = selectedGroup else { return store.stringParams }
        return store.stringParams.filter { $0.group == group }
    }
}

// MARK: - Widget Views

/// Float parameter slider view
private struct FloatSliderView: View {
    @ObservedObject var parameter: GuiFloatParameter

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

/// Int parameter slider view
private struct IntSliderView: View {
    @ObservedObject var parameter: GuiIntParameter

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

/// Boolean parameter toggle view
private struct BoolToggleView: View {
    @ObservedObject var parameter: GuiBoolParameter

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

/// Color parameter picker view
private struct ColorPickerView: View {
    @ObservedObject var parameter: GuiColorParameter

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

/// Button parameter view
private struct ButtonView: View {
    @ObservedObject var parameter: GuiButtonParameter

    var body: some View {
        Button(action: {
            parameter.action?()
        }) {
            Text(parameter.name)
                .font(.system(size: 12, weight: .medium))
                .frame(maxWidth: .infinity)
                .padding(.vertical, 6)
                .background(Color.blue.opacity(0.1))
                .cornerRadius(6)
        }
        .buttonStyle(.plain)
    }
}

/// String parameter text field view
private struct TextFieldView: View {
    @ObservedObject var parameter: GuiStringParameter

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(parameter.name)
                .font(.system(size: 12, weight: .medium))

            TextField("", text: $parameter.value)
                .textFieldStyle(.roundedBorder)
                .font(.system(size: 11))
        }
    }
}

// MARK: - Preview

#if DEBUG
struct OFLGuiPanel_Previews: PreviewProvider {
    static var previews: some View {
        // Setup preview data
        let store = GuiParameterStore.shared
        store.clear()

        _ = store.registerFloat(name: "Radius", group: "Shapes", value: 50.0, min: 10.0, max: 200.0)
        _ = store.registerFloat(name: "Speed", group: "Animation", value: 1.0, min: 0.1, max: 10.0)
        _ = store.registerInt(name: "Resolution", group: "Shapes", value: 32, min: 8, max: 128)
        _ = store.registerBool(name: "Wireframe", group: "Rendering", value: false)
        _ = store.registerBool(name: "Show FPS", group: "Rendering", value: true)
        _ = store.registerColor(name: "Background", group: "Colors", r: 0, g: 0, b: 0)
        _ = store.registerColor(name: "Foreground", group: "Colors", r: 255, g: 0, b: 0)
        _ = store.registerButton(name: "Reset", group: "Actions") {
            print("Reset button pressed")
        }
        _ = store.registerString(name: "Label", group: "Text", value: "Hello World")

        return ZStack(alignment: .topTrailing) {
            Color.black.ignoresSafeArea()

            OFLGuiPanel()
                .padding(20)
        }
        .frame(width: 800, height: 600)
    }
}
#endif
