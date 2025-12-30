class_name AbstractCesiumCamera extends Camera3D

@export
var globe_node : CesiumGeoreference

@export
var tilesets : Array[Cesium3DTileset]

@onready
var post_process_mesh = preload("res://addons/cesium_godot/visuals/post-process.tscn")

@export
var render_atmosphere : bool

var atmosphere_manager: AtmosphereManager

var last_hit_distance: float

const RADII := 6378137.0
const ACCEPTABLE_NEAR_PLANE := 9

func find_directional_light(node: Node) -> DirectionalLight3D:
	if node is DirectionalLight3D:
		return node

	for child in node.get_children():
		var light = find_directional_light(child)
		if light:
			return light

	return null

func _ready() -> void:
	# Auto-discover tilesets if not explicitly set
	if self.tilesets.is_empty() and self.globe_node:
		self.tilesets = _find_all_tilesets(self.globe_node)

	# This is a workaround for the fact that you can't detect double precision builds in GDScript
	# but you probably aren't using TrueOrigin in a single precision build
	if self.globe_node.origin_type == CesiumGeoreference.CartographicOrigin:
		self.far = 35358652.0
		self.near = ACCEPTABLE_NEAR_PLANE
	else:
		self.far = 149597870700.0 + 3*1392700000.0
		# Get the ecef
		var ecef_pos := Vector3(self.globe_node.ecefX, self.globe_node.ecefY, self.globe_node.ecefZ)
		self.global_position = self.globe_node.get_tx_ecef_to_engine() * ecef_pos

	if self.render_atmosphere:
		self._load_atmosphere()



func _load_atmosphere() -> void:
	self.atmosphere_manager = AtmosphereManager.new()
	self.atmosphere_manager.display_atmosphere = true
	self.atmosphere_manager.globe = self.globe_node
	var atmosphereNode = self.post_process_mesh.instantiate()
	self.atmosphere_manager.mesh_atmosphere = atmosphereNode
	self.atmosphere_manager.camera = self
	self.atmosphere_manager.sun = self.find_directional_light(self.get_tree().current_scene)
	if self.atmosphere_manager.sun == null:
		push_warning("Atmosphere couldn't find a directional light to use as the sun!")
		return
	self.add_child(atmosphereNode)
	self.add_child(self.atmosphere_manager)


func _process(_delta: float) -> void:
	_update_tilesets()


func _update_tilesets() -> void:
	var camera_xform := self.globe_node.get_tx_engine_to_ecef() * self.global_transform
	for tileset in self.tilesets:
		if (tileset == null): continue
		tileset.update_tileset(camera_xform)


func _find_all_tilesets(node: Node) -> Array[Cesium3DTileset]:
	var result: Array[Cesium3DTileset] = []
	if node is Cesium3DTileset:
		result.append(node)
	for child in node.get_children():
		result.append_array(_find_all_tilesets(child))
	return result
