
# Cesium Godot 3D Tiles Plugin

A Godot 4 extension that integrates Cesium 3D Tiles capabilities into Godot Engine through GDExtension.

![Godot Cesium Plugin](readme_resources/demo1.jpg)

## Prerequisites

- **Windows 10/11** (64-bit)
- **Python 3.8+** (with pip)
- **SCons** (`pip install scons`)
- **Visual Studio 2022** with C++ desktop development workload (If you plan on contributing or developing)
- **Git**
- **Godot 4.1+** (with GDExtension support)

## Installation (as GDExtension)

### 1. Clone the Repository
```bash
git clone https://github.com/Battle-Road/3D-Tiles-For-Godot.git
cd 3D-Tiles-For-Godot
```

If you would like to build the binaries and you don't plan on modifying the underlying C++ code with any editor use the following command
`scons platform=windows arch=x64 compileTarget=extension`

Otherwise, to get a `compile_commands.json` file to use with an IDE or Text Editor, run
`scons platform=windows compiledb=true arch=x64 compileTarget=extension debug_symbols=true`

#### Build Flags Explained:

* `platform=windows`: Build for Windows (currently only supported platform)

* `compiledb=true`: Generate IDE-friendly compilation database (optional)

* `arch=x64`: 64-bit architecture, will be used as build architecture for all subsequent libraries 

* `compileTarget=extension`: Build as GDExtension

* `debug_symbols=yes`: Include debug information (use no for release)

**The build binaries will be at `godot3dtiles/bin/Godot3DTiles.windows.template_debug.x86_64.dll`**


## Setting Up a Project
### Create New Godot Project

* Use Godot 4.1 or newer

* Choose "Forward+" renderer in project settings

### Copy Plugin Files

* From the repository:

  * Copy `godot3dtiles/bin/` folder to your project root

  * Copy `godot3dtiles/addons/` folder to your project root

Your project structure should look like:

```
my_project/
├── addons/
│   └── cesium_godot/
│       ├── panels/
│       ├── scripts/
│       └── visuals/
├── bin/
└── project.godot
```

Alternatively, you can choose to create a new project using the `godot3dtiles/` folder as the project's root

### Enable the plugin

* Open your project in Godot

* Go to Project → Project Settings → Plugins

* Enable "cesium_godot"

## Getting Started

### Using the Cesium Panel

![PanelDemo1](readme_resources/panel_default.png)

The Cesium Panel provides essential tools for managing geospatial content and configuring plugin integration with Cesium services.

**Connect to Cesium ION**


* Select Connect to Cesium ION to connect the plugin to your Cesium account.

* Requires valid Cesium ION credentials

* Grants access to your private assets and token-protected resources

* Maintains session security through OAuth2 authorization flow.

**Access Token Management**

Upon successful authentication:

* Access tokens are stored by default in `addons/cesium_godot/cesium_gd_config.tres`
* The `cesium_gd_config.tres` resource file controls the following parameters
  * `accessToken`: Empty by default and will be filled in after connecting with ION
  * `serverUrl`: Points to `https://ion.cesium.com` by default, could be changed for self-hosting purposes
  * `apiUrl`: Points to `https://api.cesium.com/` by default, could be changed for self-hosting purposes
  * `applicationId`: The Cesium ION identifier for this application, by default, 3D Tiles For Godot uses id `891`

#### Note: This plugin is currently Windows-only. Linux/MacOS support is planned for future releases.

