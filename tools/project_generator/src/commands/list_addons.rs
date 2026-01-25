use crate::error::Result;
use crate::utils::*;

pub fn execute(available: bool, project: &str, verbose: bool) -> Result<()> {
    if available {
        println!("Core Addons:");
        for addon in core_addons() {
            println!("  - {}", addon);
        }

        println!();
        println!("Apple Native Addons:");
        for addon in native_addons() {
            println!("  - {}", addon);
        }
    } else {
        if verbose {
            println!("Listing addons in project: {}", project);
        }
        println!("Project addons:");
        println!("  (project addon listing not yet implemented)");
    }

    Ok(())
}
