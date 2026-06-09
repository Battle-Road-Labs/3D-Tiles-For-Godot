# config.py — Godot engine module configuration for Cesium 3D Tiles
# This file is read by the Godot engine build system when cesium_godot/
# is placed under <godot-source>/modules/cesium_godot/


def can_build(env, platform):
    # Requires C++20 support
    # Supported on all desktop platforms and web
    return platform in ["windows", "linuxbsd", "macos", "web"]


def configure(env):
    pass


def get_doc_classes():
    return [
        "CesiumGeoreference",
        "Cesium3DTileset",
        "CesiumHTTPRequestNode",
        "CesiumDebugUtils",
        "CesiumGDPanel",
        "CesiumIonRasterOverlay",
        "CesiumGDConfig",
        "DocumentContainer",
        "CesiumGDAssetBuilder",
        "TokenTroubleshooting",
        "GeoreferencedMesh",
        "Cesium3DTile",
        "CesiumGDCreditSystem",
    ]


def get_doc_path():
    return "doc_classes"
