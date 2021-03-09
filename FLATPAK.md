# Develop with Flatpak

## Build

```
flatpak-builder --repo=repo --force-clean _flatpak_build/ build-aux/org.gnome.Robots.json
```

## Run

```
flatpak build --filesystem=host _flatpak_build gnome-robots
```

## Bundle

```
flatpak build-bundle ./repo robots.flatpak org.gnome.Robots
```
