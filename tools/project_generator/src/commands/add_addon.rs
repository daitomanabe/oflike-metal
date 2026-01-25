use crate::error::{GeneratorError, Result};
use crate::utils::*;

pub fn execute(
    addon_name: &str,
    mode: &str,
    source: Option<&str>,
    project: &str,
    update_build: bool,
    verbose: bool,
) -> Result<()> {
    validate_addon_mode(mode)?;

    if verbose {
        println!("Adding addon: {}", addon_name);
        println!("  Mode: {}", mode);
        println!("  Project: {}", project);
    }

    // Check if it's a builtin addon
    if is_builtin_addon(addon_name) {
        if mode != "reference" {
            return Err(GeneratorError::InvalidAddonMode(format!(
                "Core/Native addon '{}' must use 'reference' mode",
                addon_name
            )));
        }
        println!(
            "✅ Core/Native addon '{}' is available via oflike-metal framework",
            addon_name
        );
        return Ok(());
    }

    // Custom addon requires source path
    if source.is_none() {
        return Err(GeneratorError::AddonNotFound(format!(
            "Custom addon '{}' requires --source path",
            addon_name
        )));
    }

    println!("✅ Addon '{}' added successfully!", addon_name);
    println!("   Note: This is a placeholder. Full implementation in Phase 10.3");

    Ok(())
}
