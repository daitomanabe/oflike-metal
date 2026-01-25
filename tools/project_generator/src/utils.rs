use crate::error::{GeneratorError, Result};

/// Validate project name (PascalCase or kebab-case)
pub fn validate_project_name(name: &str) -> Result<()> {
    if name.is_empty() {
        return Err(GeneratorError::InvalidProjectName(
            "Project name cannot be empty".to_string(),
        ));
    }

    // Check for invalid characters
    if !name
        .chars()
        .all(|c| c.is_alphanumeric() || c == '-' || c == '_')
    {
        return Err(GeneratorError::InvalidProjectName(format!(
            "Invalid characters in project name: {}",
            name
        )));
    }

    Ok(())
}

/// Validate bundle ID (reverse DNS format)
pub fn validate_bundle_id(bundle_id: &str) -> Result<()> {
    if bundle_id.is_empty() {
        return Err(GeneratorError::InvalidBundleId(
            "Bundle ID cannot be empty".to_string(),
        ));
    }

    let parts: Vec<&str> = bundle_id.split('.').collect();
    if parts.len() < 2 {
        return Err(GeneratorError::InvalidBundleId(
            "Bundle ID must have at least 2 parts (e.g., com.example)".to_string(),
        ));
    }

    for part in parts {
        if part.is_empty() || !part.chars().all(|c| c.is_alphanumeric()) {
            return Err(GeneratorError::InvalidBundleId(format!(
                "Invalid bundle ID format: {}",
                bundle_id
            )));
        }
    }

    Ok(())
}

/// Validate entry mode
pub fn validate_entry_mode(mode: &str) -> Result<()> {
    match mode {
        "swiftui" | "ofmain" => Ok(()),
        _ => Err(GeneratorError::InvalidEntryMode(format!(
            "Invalid entry mode: {}. Must be 'swiftui' or 'ofmain'",
            mode
        ))),
    }
}

/// Validate addon mode
pub fn validate_addon_mode(mode: &str) -> Result<()> {
    match mode {
        "reference" | "copy" | "symlink" => Ok(()),
        _ => Err(GeneratorError::InvalidAddonMode(format!(
            "Invalid addon mode: {}. Must be 'reference', 'copy', or 'symlink'",
            mode
        ))),
    }
}

/// Validate template name
pub fn validate_template(template: &str) -> Result<()> {
    match template {
        "basic" | "swiftui" | "metal" | "3d" => Ok(()),
        _ => Err(GeneratorError::InvalidTemplate(format!(
            "Invalid template: {}. Must be 'basic', 'swiftui', 'metal', or '3d'",
            template
        ))),
    }
}

/// Convert project name to PascalCase
pub fn to_pascal_case(name: &str) -> String {
    name.split('-')
        .map(|part| {
            let mut chars = part.chars();
            match chars.next() {
                None => String::new(),
                Some(first) => first.to_uppercase().chain(chars).collect(),
            }
        })
        .collect()
}

/// Generate bundle ID from project name
pub fn generate_bundle_id(project_name: &str, prefix: &str) -> String {
    let normalized = project_name.to_lowercase().replace('_', "");
    format!("{}.{}", prefix, normalized)
}

/// List of Core Addons
pub fn core_addons() -> Vec<&'static str> {
    vec!["ofxOsc", "ofxGui", "ofxXmlSettings", "ofxSvg", "ofxNetwork", "ofxOpenCv"]
}

/// List of Apple Native Addons
pub fn native_addons() -> Vec<&'static str> {
    vec![
        "ofxSharp",
        "ofxNeuralEngine",
        "ofxMetalCompute",
        "ofxMPS",
        "ofxVideoToolbox",
        "ofxSpatialAudio",
        "ofxMetalFX",
    ]
}

/// Check if addon is Core or Native
pub fn is_builtin_addon(addon_name: &str) -> bool {
    core_addons().contains(&addon_name) || native_addons().contains(&addon_name)
}
