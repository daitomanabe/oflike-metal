import AppKit

// MARK: - Project Manager
class ProjectManager {
    var projectDir: String = ""
    var buildDir: String = ""
    var projectName: String = ""
    var isBuilt: Bool = false

    var buildProcess: Process?
    var runProcess: Process?
    var buildOutput: String = ""

    weak var delegate: ProjectManagerDelegate?

    func selectProject(path: String) {
        projectDir = path
        projectName = URL(fileURLWithPath: path).lastPathComponent

        // Find the root project directory (where CMakeLists.txt is)
        var currentPath = URL(fileURLWithPath: path)
        while currentPath.path != "/" {
            let parentPath = currentPath.deletingLastPathComponent()
            let cmakePath = parentPath.appendingPathComponent("CMakeLists.txt")
            if FileManager.default.fileExists(atPath: cmakePath.path) {
                buildDir = parentPath.appendingPathComponent("build").path
                break
            }
            currentPath = parentPath
        }

        checkIfBuilt()
    }

    func checkIfBuilt() {
        let execPath = getExecutablePath()
        isBuilt = FileManager.default.isExecutableFile(atPath: execPath)
    }

    func getExecutablePath() -> String {
        return buildDir + "/examples/" + projectName
    }

    func build(completion: @escaping (Bool) -> Void) {
        buildOutput = ""
        delegate?.buildOutputUpdated("")

        DispatchQueue.global(qos: .userInitiated).async { [weak self] in
            guard let self = self else { return }

            let fileManager = FileManager.default

            // Create build directory if needed
            if !fileManager.fileExists(atPath: self.buildDir) {
                self.appendOutput("Creating build directory...\n")
                try? fileManager.createDirectory(atPath: self.buildDir, withIntermediateDirectories: true)

                // Run cmake
                self.appendOutput("Running cmake...\n")
                let cmakeResult = self.runBuildProcess(
                    executable: "/usr/bin/cmake",
                    arguments: [".."],
                    workingDir: self.buildDir
                )
                if cmakeResult != 0 {
                    self.appendOutput("\n❌ CMake failed\n")
                    DispatchQueue.main.async { completion(false) }
                    return
                }
                self.appendOutput("✓ CMake completed\n\n")
            }

            // Build the project
            self.appendOutput("Building \(self.projectName)...\n")
            let buildResult = self.runBuildProcess(
                executable: "/usr/bin/make",
                arguments: ["-j8", self.projectName],
                workingDir: self.buildDir
            )

            if buildResult != 0 {
                self.appendOutput("\n❌ Build failed\n")
                DispatchQueue.main.async { completion(false) }
                return
            }

            self.appendOutput("\n✓ Build successful\n")

            DispatchQueue.main.async {
                self.checkIfBuilt()
                completion(true)
            }
        }
    }

    private func runBuildProcess(executable: String, arguments: [String], workingDir: String) -> Int32 {
        let process = Process()
        process.executableURL = URL(fileURLWithPath: executable)
        process.arguments = arguments
        process.currentDirectoryURL = URL(fileURLWithPath: workingDir)

        let pipe = Pipe()
        process.standardOutput = pipe
        process.standardError = pipe

        // Read output in real-time
        pipe.fileHandleForReading.readabilityHandler = { [weak self] handle in
            let data = handle.availableData
            if let str = String(data: data, encoding: .utf8), !str.isEmpty {
                self?.appendOutput(str)
            }
        }

        do {
            buildProcess = process
            try process.run()
            process.waitUntilExit()
            pipe.fileHandleForReading.readabilityHandler = nil
            buildProcess = nil
            return process.terminationStatus
        } catch {
            appendOutput("Error: \(error)\n")
            return -1
        }
    }

    private func appendOutput(_ text: String) {
        DispatchQueue.main.async { [weak self] in
            guard let self = self else { return }
            self.buildOutput += text
            self.delegate?.buildOutputUpdated(self.buildOutput)
        }
    }

    func run() -> Bool {
        guard isBuilt else { return false }

        // Stop if already running
        if let process = runProcess, process.isRunning {
            process.terminate()
            runProcess = nil
            return false
        }

        let execPath = getExecutablePath()
        let process = Process()
        process.executableURL = URL(fileURLWithPath: execPath)
        process.currentDirectoryURL = URL(fileURLWithPath: buildDir + "/examples")

        do {
            try process.run()
            runProcess = process

            // Monitor termination
            DispatchQueue.global().async { [weak self] in
                process.waitUntilExit()
                DispatchQueue.main.async {
                    self?.runProcess = nil
                    self?.delegate?.runStateChanged(false)
                }
            }
            return true
        } catch {
            return false
        }
    }

    func stop() {
        runProcess?.terminate()
        runProcess = nil
    }

    var isRunning: Bool {
        return runProcess?.isRunning ?? false
    }
}

protocol ProjectManagerDelegate: AnyObject {
    func buildOutputUpdated(_ output: String)
    func runStateChanged(_ isRunning: Bool)
}

// MARK: - Main Window
class MainWindowController: NSWindowController, ProjectManagerDelegate {
    let manager = ProjectManager()

    var selectButton: NSButton!
    var projectLabel: NSTextField!
    var statusLabel: NSTextField!
    var buildButton: NSButton!
    var runButton: NSButton!
    var logTextView: NSTextView!
    var progressIndicator: NSProgressIndicator!

    convenience init() {
        let window = NSWindow(
            contentRect: NSRect(x: 0, y: 0, width: 600, height: 500),
            styleMask: [.titled, .closable, .miniaturizable, .resizable],
            backing: .buffered,
            defer: false
        )
        window.title = "oflike-metal Launcher"
        window.center()
        window.minSize = NSSize(width: 500, height: 400)

        self.init(window: window)
        manager.delegate = self
        setupUI()
    }

    func setupUI() {
        guard let window = window else { return }

        let contentView = NSView(frame: window.contentView!.bounds)
        contentView.autoresizingMask = [.width, .height]

        // === Top Section: Project Selection ===
        let topSection = NSView(frame: NSRect(x: 0, y: contentView.bounds.height - 120, width: contentView.bounds.width, height: 120))
        topSection.autoresizingMask = [.width, .minYMargin]

        // Title
        let titleLabel = NSTextField(labelWithString: "oflike-metal Example Launcher")
        titleLabel.font = NSFont.systemFont(ofSize: 20, weight: .semibold)
        titleLabel.frame = NSRect(x: 20, y: 75, width: 400, height: 30)
        topSection.addSubview(titleLabel)

        // Select button
        selectButton = NSButton(frame: NSRect(x: 20, y: 30, width: 150, height: 32))
        selectButton.title = "Select Example..."
        selectButton.bezelStyle = .rounded
        selectButton.target = self
        selectButton.action = #selector(selectProject)
        topSection.addSubview(selectButton)

        // Project name
        projectLabel = NSTextField(labelWithString: "No project selected")
        projectLabel.font = NSFont.systemFont(ofSize: 14)
        projectLabel.textColor = .secondaryLabelColor
        projectLabel.frame = NSRect(x: 180, y: 35, width: 300, height: 22)
        projectLabel.autoresizingMask = [.width]
        topSection.addSubview(projectLabel)

        // Status
        statusLabel = NSTextField(labelWithString: "")
        statusLabel.font = NSFont.systemFont(ofSize: 12)
        statusLabel.textColor = .tertiaryLabelColor
        statusLabel.frame = NSRect(x: 180, y: 12, width: 300, height: 18)
        statusLabel.autoresizingMask = [.width]
        topSection.addSubview(statusLabel)

        contentView.addSubview(topSection)

        // === Middle Section: Buttons ===
        let buttonSection = NSView(frame: NSRect(x: 0, y: contentView.bounds.height - 180, width: contentView.bounds.width, height: 50))
        buttonSection.autoresizingMask = [.width, .minYMargin]

        buildButton = NSButton(frame: NSRect(x: 20, y: 10, width: 100, height: 32))
        buildButton.title = "Build"
        buildButton.bezelStyle = .rounded
        buildButton.target = self
        buildButton.action = #selector(buildProject)
        buildButton.isEnabled = false
        buttonSection.addSubview(buildButton)

        runButton = NSButton(frame: NSRect(x: 130, y: 10, width: 100, height: 32))
        runButton.title = "Run"
        runButton.bezelStyle = .rounded
        runButton.target = self
        runButton.action = #selector(runProject)
        runButton.isEnabled = false
        buttonSection.addSubview(runButton)

        progressIndicator = NSProgressIndicator(frame: NSRect(x: 250, y: 15, width: 20, height: 20))
        progressIndicator.style = .spinning
        progressIndicator.isHidden = true
        buttonSection.addSubview(progressIndicator)

        let buildRunButton = NSButton(frame: NSRect(x: buttonSection.bounds.width - 140, y: 10, width: 120, height: 32))
        buildRunButton.title = "Build & Run"
        buildRunButton.bezelStyle = .rounded
        buildRunButton.keyEquivalent = "\r"  // Enter key
        buildRunButton.target = self
        buildRunButton.action = #selector(buildAndRun)
        buildRunButton.autoresizingMask = [.minXMargin]
        buttonSection.addSubview(buildRunButton)

        contentView.addSubview(buttonSection)

        // === Bottom Section: Log ===
        let logLabel = NSTextField(labelWithString: "Build Output")
        logLabel.font = NSFont.systemFont(ofSize: 12, weight: .medium)
        logLabel.textColor = .secondaryLabelColor
        logLabel.frame = NSRect(x: 20, y: contentView.bounds.height - 205, width: 100, height: 18)
        logLabel.autoresizingMask = [.minYMargin]
        contentView.addSubview(logLabel)

        let logScrollView = NSScrollView(frame: NSRect(
            x: 20,
            y: 20,
            width: contentView.bounds.width - 40,
            height: contentView.bounds.height - 230
        ))
        logScrollView.autoresizingMask = [.width, .height]
        logScrollView.hasVerticalScroller = true
        logScrollView.borderType = .bezelBorder

        logTextView = NSTextView(frame: logScrollView.bounds)
        logTextView.isEditable = false
        logTextView.font = NSFont.monospacedSystemFont(ofSize: 11, weight: .regular)
        logTextView.backgroundColor = NSColor(white: 0.1, alpha: 1.0)
        logTextView.textColor = NSColor(white: 0.9, alpha: 1.0)
        logTextView.autoresizingMask = [.width]
        logTextView.string = "Select an example folder to begin.\n\nExamples are located in:\n  examples/01_basics\n  examples/02_shapes\n  etc."
        logScrollView.documentView = logTextView
        contentView.addSubview(logScrollView)

        window.contentView = contentView
    }

    @objc func selectProject() {
        let panel = NSOpenPanel()
        panel.canChooseFiles = false
        panel.canChooseDirectories = true
        panel.allowsMultipleSelection = false
        panel.message = "Select an example folder (e.g., 01_basics)"
        panel.prompt = "Select"

        // Set initial directory to examples folder
        let execPath = CommandLine.arguments[0]
        let execURL = URL(fileURLWithPath: execPath).deletingLastPathComponent()
        let projectRoot = execURL
            .deletingLastPathComponent()
            .deletingLastPathComponent()
            .deletingLastPathComponent()
        let examplesDir = projectRoot.appendingPathComponent("examples")

        if FileManager.default.fileExists(atPath: examplesDir.path) {
            panel.directoryURL = examplesDir
        }

        panel.beginSheetModal(for: window!) { [weak self] response in
            guard response == .OK, let url = panel.url else { return }
            self?.loadProject(url.path)
        }
    }

    func loadProject(_ path: String) {
        manager.selectProject(path: path)

        // Update UI
        projectLabel.stringValue = manager.projectName
        projectLabel.textColor = .labelColor

        updateStatus()

        buildButton.isEnabled = true
        runButton.isEnabled = manager.isBuilt

        logTextView.string = "Project: \(manager.projectName)\nPath: \(path)\nBuild dir: \(manager.buildDir)\n\nClick 'Build' or 'Build & Run' to compile."
    }

    func updateStatus() {
        if manager.isBuilt {
            statusLabel.stringValue = "✓ Built and ready"
            statusLabel.textColor = .systemGreen
            runButton.isEnabled = true
        } else {
            statusLabel.stringValue = "Not built"
            statusLabel.textColor = .secondaryLabelColor
            runButton.isEnabled = false
        }

        if manager.isRunning {
            runButton.title = "Stop"
            statusLabel.stringValue = "● Running"
            statusLabel.textColor = .systemGreen
        } else {
            runButton.title = "Run"
        }
    }

    @objc func buildProject() {
        guard !manager.projectDir.isEmpty else { return }

        buildButton.isEnabled = false
        runButton.isEnabled = false
        progressIndicator.isHidden = false
        progressIndicator.startAnimation(nil)
        statusLabel.stringValue = "Building..."
        statusLabel.textColor = .systemOrange

        manager.build { [weak self] success in
            self?.progressIndicator.stopAnimation(nil)
            self?.progressIndicator.isHidden = true
            self?.buildButton.isEnabled = true
            self?.updateStatus()
        }
    }

    @objc func runProject() {
        guard manager.isBuilt else { return }

        if manager.isRunning {
            manager.stop()
        } else {
            if manager.run() {
                runStateChanged(true)
            }
        }
        updateStatus()
    }

    @objc func buildAndRun() {
        guard !manager.projectDir.isEmpty else {
            selectProject()
            return
        }

        buildButton.isEnabled = false
        runButton.isEnabled = false
        progressIndicator.isHidden = false
        progressIndicator.startAnimation(nil)
        statusLabel.stringValue = "Building..."
        statusLabel.textColor = .systemOrange

        manager.build { [weak self] success in
            guard let self = self else { return }
            self.progressIndicator.stopAnimation(nil)
            self.progressIndicator.isHidden = true
            self.buildButton.isEnabled = true
            self.updateStatus()

            if success {
                // Auto-run after successful build
                DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                    if self.manager.run() {
                        self.runStateChanged(true)
                        self.updateStatus()
                    }
                }
            }
        }
    }

    // MARK: - ProjectManagerDelegate

    func buildOutputUpdated(_ output: String) {
        logTextView.string = output
        logTextView.scrollToEndOfDocument(nil)
    }

    func runStateChanged(_ isRunning: Bool) {
        updateStatus()
    }
}

// MARK: - App Delegate
class AppDelegate: NSObject, NSApplicationDelegate {
    var windowController: MainWindowController!

    func applicationDidFinishLaunching(_ notification: Notification) {
        windowController = MainWindowController()
        windowController.showWindow(nil)
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        return true
    }

    func applicationWillTerminate(_ notification: Notification) {
        windowController.manager.stop()
    }
}

// MARK: - Main
let app = NSApplication.shared
let delegate = AppDelegate()
app.delegate = delegate
app.setActivationPolicy(.regular)

// Create menu bar
let mainMenu = NSMenu()
let appMenuItem = NSMenuItem()
mainMenu.addItem(appMenuItem)

let appMenu = NSMenu()
appMenu.addItem(withTitle: "Quit", action: #selector(NSApplication.terminate(_:)), keyEquivalent: "q")
appMenuItem.submenu = appMenu

app.mainMenu = mainMenu
app.activate(ignoringOtherApps: true)
app.run()
