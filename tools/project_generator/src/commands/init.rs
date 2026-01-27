use crate::config::get_author_from_git;
use crate::error::Result;
use std::fs;
use std::path::PathBuf;

pub fn execute(global: bool, local: bool, verbose: bool) -> Result<()> {
    let config_content = format!(
        r#"# oflike-gen configuration
[defaults]
addon_mode = "reference"
author = "{}"
bundle_id_prefix = "com.example"

[paths]
# oflike_metal_root = "/usr/local/lib/oflike-metal"
# addons_dir = "~/addons"

[templates]
default_template = "basic"
"#,
        get_author_from_git().unwrap_or_else(|| "Unknown".to_string())
    );

    if global || !local {
        let home = std::env::var("HOME").map_err(|_| {
            crate::error::GeneratorError::Config("HOME environment variable not set".to_string())
        })?;
        let config_path = PathBuf::from(home).join(".oflike-gen.toml");

        if verbose {
            println!("Creating global config: {}", config_path.display());
        }

        fs::write(&config_path, &config_content)?;
        println!("✅ Global config created: {}", config_path.display());
    }

    if local {
        let config_path = PathBuf::from(".oflike-gen.toml");

        if verbose {
            println!("Creating local config: {}", config_path.display());
        }

        fs::write(&config_path, &config_content)?;
        println!("✅ Local config created: {}", config_path.display());
    }

    Ok(())
}
