extends Node

class_name CesiumAssetBuilder

enum TILESET_TYPE { Blank, OsmBuildings, GooglePhotorealistic, BingMapsAerial, BingMapsWithLabels }


const CESIUM_GLOBE_NAME = "CesiumGlobe"
const CESIUM_GLOBE_GEOREF = "CesiumGeoreference"
const CESIUM_TILESET_NAME = "CesiumGDTileset"

const cesiumConfig : Resource = preload("res://addons/cesium_godot/cesium_gd_config.tres")

var use_georeferences : bool

func instantiate_tileset(tileset_type: TILESET_TYPE) -> void:	
	# Create a new Globe
	var root : Node3D = self._get_root_of_edit_scene()
	var globe : CesiumGlobe = self._find_or_create_globe()
	var tileset := CesiumGDTileset.new()
	globe.add_child(tileset)
	tileset.owner = globe.get_parent()
	#globe.add_child(tileset)
	
	tileset.cesiumConfig = self.cesiumConfig
	tileset.name = CESIUM_TILESET_NAME
	globe.global_rotation_degrees.x = -90
	tileset.rotation_degrees.x = 90

func instantiate_dynamic_cam() -> void:
	# Find an existing tileset
	var root : Node3D = self.get_tree().edited_scene_root
	var globe := self._find_or_create_globe()
	var camera : Camera3D = Camera3D.new()
	
	# We have to use set_script here instead of instantiating directly
	# Otherwise GD will try to run the input input functions in edit mode
	if (self.use_georeferences):
		print("Using georeferences")
		camera.set_script(GeoreferenceCameraController)
	else:
		camera.set_script(CesiumCameraController)
	
	print("Created cam!")
	camera.globe_node = globe
	camera.tileset = globe.find_child(CESIUM_TILESET_NAME) as CesiumGDTileset
	const 
	camera.near = 9
	camera.
	if (camera.tileset == null):
		# Blank tileset as default instantiation
		self.instantiate_tileset(TILESET_TYPE.Blank)
	print("Root object: " + root.name)
	root.add_child(camera)
	camera.owner = root

func _add_overlay() -> void:
	pass

func _find_or_create_globe() -> CesiumGlobe:
	var root : Node3D = self._get_root_of_edit_scene()
	# Try to find the globe
	var globe : CesiumGlobe = null
	for child in root.get_children():
		var found_globe := child as CesiumGlobe
		if found_globe != null: return found_globe
	print("Globe not found, creating one...")
	if (self.use_georeferences):
		globe = CesiumGDGeoreference.new()
		globe.name = CESIUM_GLOBE_GEOREF
	else:
		globe = CesiumGlobe.new()
		globe.name = CESIUM_GLOBE_NAME	
	
	# Add it to the scene
	root.add_child(globe)
	globe.owner = root
	return globe

func _get_root_of_edit_scene() -> Node3D:
	var root : Node3D = self.get_tree().edited_scene_root
	return root

