use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::path::PathBuf;

#[derive(Debug, Serialize, Deserialize)]
pub struct Config {
    #[serde(default)]
    pub defaults: Defaults,
    #[serde(default)]
    pub paths: Paths,
    #[serde(default)]
    pub templates: Templates,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Defaults {
    #[serde(default = "default_entry_mode")]
    pub entry_mode: String,
    #[serde(default = "default_addon_mode")]
    pub addon_mode: String,
    #[serde(default)]
    pub author: String,
    #[serde(default = "default_bundle_id_prefix")]
    pub bundle_id_prefix: String,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Paths {
    pub oflike_metal_root: Option<String>,
    pub addons_dir: Option<String>,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Templates {
    #[serde(default = "default_template")]
    pub default_template: String,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct ProjectConfig {
    pub project: ProjectInfo,
    pub entry: EntryConfig,
    #[serde(default)]
    pub addons: AddonsConfig,
    #[serde(default)]
    pub build: BuildConfig,
    #[serde(default)]
    pub paths: ProjectPaths,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct ProjectInfo {
    pub name: String,
    #[serde(default = "default_version")]
    pub version: String,
    #[serde(default)]
    pub author: String,
    pub bundle_id: String,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct EntryConfig {
    #[serde(default = "default_entry_mode")]
    pub mode: String,
}

#[derive(Debug, Serialize, Deserialize, Default)]
pub struct AddonsConfig {
    #[serde(default)]
    pub core: Vec<String>,
    #[serde(default)]
    pub custom: Vec<CustomAddon>,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct CustomAddon {
    pub name: String,
    pub mode: String,
    pub source: String,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct BuildConfig {
    #[serde(default = "default_true")]
    pub cmake: bool,
    #[serde(default = "default_true")]
    pub xcodegen: bool,
    #[serde(default = "default_min_macos")]
    pub min_macos: String,
    #[serde(default = "default_swift_version")]
    pub swift_version: String,
    #[serde(default = "default_cpp_standard")]
    pub cpp_standard: String,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct ProjectPaths {
    #[serde(default = "default_src")]
    pub src: String,
    #[serde(default = "default_data")]
    pub data: String,
    #[serde(default = "default_resources")]
    pub resources: String,
    #[serde(default = "default_addons")]
    pub addons: String,
}

impl Default for Defaults {
    fn default() -> Self {
        Self {
            entry_mode: default_entry_mode(),
            addon_mode: default_addon_mode(),
            author: String::new(),
            bundle_id_prefix: default_bundle_id_prefix(),
        }
    }
}

impl Default for Paths {
    fn default() -> Self {
        Self {
            oflike_metal_root: None,
            addons_dir: None,
        }
    }
}

impl Default for Templates {
    fn default() -> Self {
        Self {
            default_template: default_template(),
        }
    }
}

impl Default for BuildConfig {
    fn default() -> Self {
        Self {
            cmake: true,
            xcodegen: true,
            min_macos: default_min_macos(),
            swift_version: default_swift_version(),
            cpp_standard: default_cpp_standard(),
        }
    }
}

impl Default for ProjectPaths {
    fn default() -> Self {
        Self {
            src: default_src(),
            data: default_data(),
            resources: default_resources(),
            addons: default_addons(),
        }
    }
}

fn default_entry_mode() -> String {
    "swiftui".to_string()
}

fn default_addon_mode() -> String {
    "reference".to_string()
}

fn default_bundle_id_prefix() -> String {
    "com.example".to_string()
}

fn default_template() -> String {
    "basic".to_string()
}

fn default_version() -> String {
    "1.0.0".to_string()
}

fn default_min_macos() -> String {
    "13.0".to_string()
}

fn default_swift_version() -> String {
    "5.9".to_string()
}

fn default_cpp_standard() -> String {
    "c++20".to_string()
}

fn default_src() -> String {
    "src".to_string()
}

fn default_data() -> String {
    "data".to_string()
}

fn default_resources() -> String {
    "resources".to_string()
}

fn default_addons() -> String {
    "addons".to_string()
}

fn default_true() -> bool {
    true
}

pub fn load_global_config() -> Option<Config> {
    let home = std::env::var("HOME").ok()?;
    let config_path = PathBuf::from(home).join(".oflike-gen.toml");
    if !config_path.exists() {
        return None;
    }

    let content = std::fs::read_to_string(config_path).ok()?;
    toml::from_str(&content).ok()
}

pub fn get_author_from_git() -> Option<String> {
    std::process::Command::new("git")
        .args(&["config", "user.name"])
        .output()
        .ok()
        .and_then(|output| {
            if output.status.success() {
                String::from_utf8(output.stdout).ok()
            } else {
                None
            }
        })
        .map(|s| s.trim().to_string())
        .filter(|s| !s.is_empty())
}
