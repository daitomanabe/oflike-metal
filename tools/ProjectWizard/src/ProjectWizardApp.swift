import SwiftUI
import Foundation

@main
struct ProjectWizardApp: App {
    var body: some Scene {
        WindowGroup {
            ProjectWizardView()
        }
        .windowStyle(.titleBar)
    }
}

struct ProjectWizardView: View {
    @State private var appName: String = ""
    @State private var logText: String = ""
    @State private var statusText: String = "Ready"
    @State private var isBusy: Bool = false

    var body: some View {
        VStack(alignment: .leading, spacing: 16) {
            Text("Project Wizard")
                .font(.title)
                .bold()

            HStack(spacing: 12) {
                Text("App Name")
                    .frame(width: 80, alignment: .leading)
                TextField("Test7", text: $appName)
                    .textFieldStyle(.roundedBorder)
            }

            HStack(spacing: 12) {
                Button("Generate") {
                    generateProject()
                }
                .disabled(isBusy || normalizedAppName.isEmpty)

                Button("Run") {
                    runProject()
                }
                .disabled(isBusy || normalizedAppName.isEmpty)

                Spacer()
                Text(statusText)
                    .font(.footnote)
                    .foregroundColor(.secondary)
            }

            Divider()

            Text("Log")
                .font(.headline)

            TextEditor(text: $logText)
                .font(.system(.footnote, design: .monospaced))
                .frame(minHeight: 240)
                .overlay(
                    RoundedRectangle(cornerRadius: 8)
                        .stroke(Color.secondary.opacity(0.25), lineWidth: 1)
                )
        }
        .padding(20)
        .frame(minWidth: 640, minHeight: 420)
    }

    private var normalizedAppName: String {
        appName.trimmingCharacters(in: .whitespacesAndNewlines)
    }

    private func generateProject() {
        let name = normalizedAppName
        guard validateProjectName(name) else {
            appendLog("Error: Invalid app name. Use letters, numbers, '-' or '_'.\n")
            return
        }

        guard let rootPath = resolveRootPath() else {
            appendLog("Error: Could not locate repo root. Set OFLIKE_ROOT or launch from the repo.\n")
            statusText = "Error"
            return
        }
        appendLog("Resolved root: \(rootPath)\n")
        let generatorPath = "\(rootPath)/bin/oflike-gen"

        if FileManager.default.isExecutableFile(atPath: generatorPath) {
            runCommand(
                label: "Generating project...",
                executable: generatorPath,
                arguments: ["new", name],
                workingDirectory: rootPath,
                rootPath: rootPath
            )
        } else if let pathExecutable = findExecutableInPath("oflike-gen") {
            runCommand(
                label: "Generating project (oflike-gen)...",
                executable: pathExecutable,
                arguments: ["new", name],
                workingDirectory: rootPath,
                rootPath: rootPath
            )
        } else {
            appendLog("Error: oflike-gen not found. Build it with: cd tools/project_generator && cargo build --release && cp target/release/oflike-gen ../../bin/oflike-gen\n")
            statusText = "Error"
        }
    }

    private func runProject() {
        let name = normalizedAppName
        guard validateProjectName(name) else {
            appendLog("Error: Invalid app name. Use letters, numbers, '-' or '_'.\n")
            return
        }

        guard let rootPath = resolveRootPath() else {
            appendLog("Error: Could not locate repo root. Set OFLIKE_ROOT or launch from the repo.\n")
            statusText = "Error"
            return
        }
        appendLog("Resolved root: \(rootPath)\n")
        let scriptPath = "\(rootPath)/scripts/run_app.sh"

        guard FileManager.default.isExecutableFile(atPath: scriptPath) else {
            appendLog("Error: scripts/run_app.sh not found. Run from the repo root or set OFLIKE_ROOT.\n")
            return
        }

        runCommand(
            label: "Building and running...",
            executable: scriptPath,
            arguments: [name, "--build"],
            workingDirectory: rootPath,
            rootPath: rootPath
        )
    }

    private func resolveRootPath() -> String? {
        if let envRoot = ProcessInfo.processInfo.environment["OFLIKE_ROOT"], !envRoot.isEmpty {
            return envRoot
        }

        let fileManager = FileManager.default
        let currentDir = URL(fileURLWithPath: fileManager.currentDirectoryPath, isDirectory: true)
        let bundleURL = Bundle.main.bundleURL
        let bundleParent = bundleURL.deletingLastPathComponent()

        let startPoints = [currentDir, bundleParent, bundleURL]
        for start in startPoints {
            if let root = findRepoRoot(from: start) {
                return root.path
            }
        }

        return nil
    }

    private func validateProjectName(_ name: String) -> Bool {
        if name.isEmpty {
            return false
        }
        return name.allSatisfy { $0.isLetter || $0.isNumber || $0 == "-" || $0 == "_" }
    }

    private func findRepoRoot(from start: URL) -> URL? {
        let fileManager = FileManager.default
        var current = start
        for _ in 0..<8 {
            let generatorPath = current.appendingPathComponent("bin/oflike-gen").path
            let runScriptPath = current.appendingPathComponent("scripts/run_app.sh").path
            if fileManager.isExecutableFile(atPath: generatorPath) || fileManager.isExecutableFile(atPath: runScriptPath) {
                return current
            }
            if current.path == "/" {
                break
            }
            current = current.deletingLastPathComponent()
        }
        return nil
    }

    private func findExecutableInPath(_ name: String) -> String? {
        guard let pathValue = ProcessInfo.processInfo.environment["PATH"], !pathValue.isEmpty else {
            return nil
        }
        for dir in pathValue.split(separator: ":") {
            let candidate = URL(fileURLWithPath: String(dir)).appendingPathComponent(name).path
            if FileManager.default.isExecutableFile(atPath: candidate) {
                return candidate
            }
        }
        return nil
    }

    private func runCommand(
        label: String,
        executable: String,
        arguments: [String],
        workingDirectory: String,
        rootPath: String?
    ) {
        isBusy = true
        statusText = label
        appendLog("$ \(executable) \(arguments.joined(separator: " "))\n")

        DispatchQueue.global(qos: .userInitiated).async {
            let process = Process()
            process.executableURL = URL(fileURLWithPath: executable)
            process.arguments = arguments
            process.currentDirectoryURL = URL(fileURLWithPath: workingDirectory)
            process.environment = buildProcessEnvironment(rootPath: rootPath)

            let pipe = Pipe()
            process.standardOutput = pipe
            process.standardError = pipe

            pipe.fileHandleForReading.readabilityHandler = { handle in
                let data = handle.availableData
                if data.isEmpty {
                    return
                }
                if let text = String(data: data, encoding: .utf8), !text.isEmpty {
                    DispatchQueue.main.async {
                        self.appendLog(text)
                    }
                }
            }

            do {
                try process.run()
                process.waitUntilExit()
                pipe.fileHandleForReading.readabilityHandler = nil

                let status = process.terminationStatus
                DispatchQueue.main.async {
                    if status == 0 {
                        self.statusText = "Done"
                    } else {
                        self.statusText = "Failed (\(status))"
                        self.appendLog("Command failed with code \(status)\n")
                    }
                    self.isBusy = false
                }
            } catch {
                DispatchQueue.main.async {
                    self.appendLog("Error: \(error)\n")
                    self.statusText = "Error"
                    self.isBusy = false
                }
            }
        }
    }

    private func buildProcessEnvironment(rootPath: String?) -> [String: String] {
        var env = ProcessInfo.processInfo.environment
        let fallbackPaths = ["/opt/homebrew/bin", "/usr/local/bin"]
        let currentPath = env["PATH"] ?? ""
        var components = currentPath.split(separator: ":").map(String.init)
        for path in fallbackPaths.reversed() {
            if !components.contains(path) {
                components.insert(path, at: 0)
            }
        }
        env["PATH"] = components.joined(separator: ":")
        if let rootPath = rootPath {
            env["OFLIKE_ROOT"] = rootPath
        }
        return env
    }

    private func appendLog(_ text: String) {
        logText.append(text)
        if !text.hasSuffix("\n") {
            logText.append("\n")
        }
    }
}
