use thiserror::Error;

pub type Result<T> = std::result::Result<T, GeneratorError>;

#[derive(Error, Debug)]
pub enum GeneratorError {
    #[error("IO error: {0}")]
    Io(#[from] std::io::Error),

    #[error("Invalid project name: {0}")]
    InvalidProjectName(String),

    #[error("Invalid bundle ID: {0}")]
    InvalidBundleId(String),

    #[error("Addon not found: {0}")]
    AddonNotFound(String),

    #[error("Invalid addon mode: {0}")]
    InvalidAddonMode(String),

    #[error("Invalid template: {0}")]
    InvalidTemplate(String),

    #[error("Project already exists: {0}")]
    ProjectExists(String),

    #[allow(dead_code)]
    #[error("Project not found: {0}")]
    ProjectNotFound(String),

    #[error("Config error: {0}")]
    Config(String),

    #[allow(dead_code)]
    #[error("Build file error: {0}")]
    BuildFile(String),

    #[allow(dead_code)]
    #[error("Validation error: {0}")]
    Validation(String),

    #[error("{0}")]
    Other(String),
}

impl From<anyhow::Error> for GeneratorError {
    fn from(err: anyhow::Error) -> Self {
        GeneratorError::Other(err.to_string())
    }
}
