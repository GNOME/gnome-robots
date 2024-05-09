config_rs = configure_file(
    input: 'config.rs.in',
    output: 'config.rs',
    configuration: config_rs
)
run_command(
    'cp',
    meson.project_build_root() / 'src' / 'config.rs',
    meson.project_source_root() / 'src' / 'config.rs',
    check: true
)

cargo_options = [ '--manifest-path', meson.project_source_root() / 'Cargo.toml' ]
cargo_options += [ '--target-dir', meson.project_build_root() / 'src' ]

if profile == 'debug'
    rust_target = 'debug'
    message('Building in debug mode')
else
    cargo_options += [ '--release' ]
    rust_target = 'release'
    message('Building in release mode')
endif

cargo_env = [ 'CARGO_HOME=' + meson.project_build_root() / 'cargo-home' ]

custom_target(
    'cargo-build',
    build_by_default: true,
    build_always_stale: true,
    input: [config_rs],
    output: meson.project_name(),
    console: true,
    install: true,
    install_dir: bindir,
    command: [
        'env',
        cargo_env,
        cargo, 'build', cargo_options,
        '&&',
        'cp', 'src' / rust_target / meson.project_name(), '@OUTPUT@',
    ]
)
