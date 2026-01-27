use clap::{Parser, Subcommand};
use std::process;

mod commands;
mod config;
mod error;
mod utils;

use error::Result;

/// oflike-metal project generator
#[derive(Parser)]
#[command(name = "oflike-gen")]
#[command(version = "1.1.0")]
#[command(about = "Project generator for oflike-metal", long_about = None)]
struct Cli {
    /// Enable verbose output
    #[arg(long, global = true)]
    verbose: bool,

    /// Custom config file path
    #[arg(long, global = true, value_name = "PATH")]
    config: Option<String>,

    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand)]
enum Commands {
    /// Generate a new oflike-metal project
    New {
        /// Project name (PascalCase or kebab-case)
        project_name: String,

        /// Initial addons (comma-separated)
        #[arg(long, value_name = "LIST")]
        addons: Option<String>,

        /// Addon integration strategy
        #[arg(long, value_name = "MODE", default_value = "reference")]
        addon_mode: String,

        /// Project creation directory (default: <oflike-root>/apps)
        #[arg(long, value_name = "DIR")]
        path: Option<String>,

        /// Project template
        #[arg(long, value_name = "NAME", default_value = "basic")]
        template: String,

        /// macOS bundle identifier
        #[arg(long, value_name = "ID")]
        bundle_id: Option<String>,

        /// Project author name
        #[arg(long, value_name = "NAME")]
        author: Option<String>,

        /// Skip git initialization
        #[arg(long)]
        no_git: bool,

        /// Skip README.md generation
        #[arg(long)]
        no_readme: bool,
    },

    /// Add an addon to an existing project
    AddAddon {
        /// Addon name
        addon_name: String,

        /// Integration strategy
        #[arg(long, value_name = "MODE", default_value = "reference")]
        mode: String,

        /// Addon source path (for custom addons)
        #[arg(long, value_name = "PATH")]
        source: Option<String>,

        /// Target project directory
        #[arg(long, value_name = "PATH", default_value = ".")]
        project: String,

        /// Update build files (CMakeLists.txt, project.yml)
        #[arg(long, default_value = "true")]
        update_build: bool,
    },

    /// Remove an addon from a project
    RemoveAddon {
        /// Addon name to remove
        addon_name: String,

        /// Target project directory
        #[arg(long, value_name = "PATH", default_value = ".")]
        project: String,

        /// Keep addon files (only remove from build)
        #[arg(long)]
        keep_files: bool,
    },

    /// List available or project addons
    ListAddons {
        /// List all available addons (Core + Native)
        #[arg(long)]
        available: bool,

        /// List addons in project
        #[arg(long, value_name = "PATH", default_value = ".")]
        project: String,
    },

    /// Initialize oflike-gen configuration
    Init {
        /// Create global config (~/.oflike-gen.toml)
        #[arg(long)]
        global: bool,

        /// Create local config (./.oflike-gen.toml)
        #[arg(long)]
        local: bool,
    },

    /// Validate project structure and configuration
    Validate {
        /// Project directory to validate
        #[arg(long, value_name = "PATH", default_value = ".")]
        project: String,

        /// Attempt to fix issues automatically
        #[arg(long)]
        fix: bool,
    },
}

fn main() {
    let cli = Cli::parse();

    if let Err(e) = run(cli) {
        eprintln!("Error: {}", e);
        process::exit(1);
    }
}

fn run(cli: Cli) -> Result<()> {
    match cli.command {
        Commands::New {
            project_name,
            addons,
            addon_mode,
            path,
            template,
            bundle_id,
            author,
            no_git,
            no_readme,
        } => {
            commands::new::execute(
                &project_name,
                addons.as_deref(),
                &addon_mode,
                path.as_deref(),
                &template,
                bundle_id.as_deref(),
                author.as_deref(),
                no_git,
                no_readme,
                cli.verbose,
            )
        }

        Commands::AddAddon {
            addon_name,
            mode,
            source,
            project,
            update_build,
        } => commands::add_addon::execute(
            &addon_name,
            &mode,
            source.as_deref(),
            &project,
            update_build,
            cli.verbose,
        ),

        Commands::RemoveAddon {
            addon_name,
            project,
            keep_files,
        } => commands::remove_addon::execute(&addon_name, &project, keep_files, cli.verbose),

        Commands::ListAddons { available, project } => {
            commands::list_addons::execute(available, &project, cli.verbose)
        }

        Commands::Init { global, local } => commands::init::execute(global, local, cli.verbose),

        Commands::Validate { project, fix } => {
            commands::validate::execute(&project, fix, cli.verbose)
        }
    }
}
