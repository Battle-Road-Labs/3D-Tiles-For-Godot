extends Camera3D

class_name CesiumCameraController

const RADII := 6378137.0

@export var cam_zoom_speed: float = 1.05
@export var zoom_limits: Vector2 = Vector2(1, 1e12)

@export
var speed : float = 0.01

var initial_rotation_speed : float

var initial_zoom_speed : float

@export
var tilesets : Array[CesiumGDTileset]

@export
var globe_node : CesiumGlobe

var desired_cam_pos : Vector3

var loaded : bool

var mouse_ecef_position : Vector3

var mouse_engine_position : Vector3

var is_zooming: bool

var position_blend: float

@export
var correction_speed: float

var distance_to_surface : float

const KM = 1000

var current_index = 0

var traverse_blend : float = 0

var prev_pos : Vector3

var timer_to_stay : float = 0

# Variables to store the rotation angles
var rotation_x: float = 0.0
var rotation_y: float = 0.0

@export
var mouse_sensitivity : float = 0.5


@export
var move_speed : float = 100

@export
var picth_sensitivity : float = 0.003

@export
var yaw_sensitivity : float = 0.003

@export
var debug_model : Node3D

@export
var info_labels_ui : InfoLabelsUI

var ray_hit_pos : Vector3

var last_valid_hit : Vector3

var zoom_acceleration : float = 0


var last_remapped_up : Vector3

var is_moving : bool

var speed_factor := 0.0

var globe_normal_finder : GlobeNormalFinder = null

var surface_basis : Basis

var last_input_dir : Vector3

var ray_caster : RayCast3D

var initialized : bool = false

var current_pitch : float = 0
var current_yaw : float = 0

var eus_basis : Basis

var shouldRaycast: bool = false

func _ready() -> void:
	
	self.position_blend = 0
	self.is_zooming = false
	self.is_moving = false
	self.initial_zoom_speed = self.cam_zoom_speed
	self.initial_rotation_speed = self.speed
	#self.global_position = Vector3(0, 0, RADII * 1.5)
	self.prev_pos = self.position
	const defaultPos := Vector3(-1292935, -4740026, 4056960)
	var engineTestPos := self.globe_node.get_tx_ecef_to_engine() * defaultPos
	self.global_position = engineTestPos
	for tileset in self.tilesets:
		if tileset.create_physics_meshes:
			continue
		self.shouldRaycast = true


func _post_init() -> void:
	self.globe_normal_finder = GlobeNormalFinder.new()
	self.add_child(self.globe_normal_finder)
	self.ray_caster = RayCast3D.new()
	self.get_tree().root.add_child(self.ray_caster)
	self.ray_caster.enabled = true
	self.initialized = true
	print("Origin type: " + str(self.globe_node.origin_type))


func _process(delta: float) -> void:
	
	if (!self.initialized):
		self._post_init()
	
	if (self.info_labels_ui != null):
		#var display_distance := self.distance_to_surface - RADII
		self.info_labels_ui.update_move_speed(self.move_speed)
		self.info_labels_ui.update_distance(self.distance_to_surface)

	# Get the transform to convert to ECEF, and a surface normal
	var engineToEcefTransform : Transform3D = self.globe_node.get_tx_engine_to_ecef() * self.global_transform
	var cesiumNormal : Vector3 = self.globe_node.get_normal_at_surface_pos(engineToEcefTransform.origin)
	self.surface_basis = self._create_local_directions(cesiumNormal)
	
	var ecef_mouse : Vector3 = self.globe_node.get_mouse_pos_ecef()
	if !is_nan(ecef_mouse.x):
		self.mouse_ecef_position = ecef_mouse
	self.mouse_engine_position = self.get_mouse_engine_surface_pos()
	
	adjust_far_and_near(delta)
	if (self.loaded):
		for tileset in self.tilesets:
			tileset.update_tileset(engineToEcefTransform)
	self.ray_hit_pos = self.globe_node.ray_to_surface(self.global_position, self.global_transform.basis.z)
	
	if(self.shouldRaycast):
		self.distance_to_surface = self._get_surface_distance_raycast()
	else:	
		self.distance_to_surface = self.ray_hit_pos.distance_to(self.global_position)
	handle_input(delta)
	#self._update_orientation()

func _physics_process(delta: float) -> void:
	self._handle_collisions(delta)
	self.update_camera_pos(delta)

func _handle_collisions(_delta: float) -> void:
	var spaceState : PhysicsDirectSpaceState3D = self.get_world_3d().direct_space_state
	# Distance of 100 to debug bc I can't find any objects nearby
	var target : Vector3 = self.global_position + self.last_input_dir * self.distance_to_surface
	var params := PhysicsRayQueryParameters3D.create(self.global_position, target)
	params.hit_back_faces = true
	params.hit_from_inside = false
	params.collide_with_bodies = true
	params.collide_with_areas = true
	var collision = spaceState.intersect_ray(params)
	
	if (collision):
		var distance : float = self.global_position.distance_to(collision.position)
		if (distance > 10):
			return
		#TODO: Add reflection vector calculation
		self.desired_cam_pos = self.global_position
		print(distance)
	

func _update_orientation() -> void:
	# Get surface normal from the globe
	var normal: Vector3 = self.globe_node.get_normal_at_surface_pos(self.global_position)
	self.transform.basis = _create_local_directions(normal)
				

func update_camera_pos(delta: float) -> void:
	if (!self.is_moving && !self.is_zooming):
		return
	if (self.is_moving):
		self.global_position = self.desired_cam_pos
		#self.get_parent_node_3d().global_position = self.desired_cam_pos
		return
	if (self.position_blend > 1 || (!self.is_zooming && !self.is_moving)):
		self.is_zooming = false
		self.position_blend = self._get_surface_distance_raycast()
		return
	if (self.is_zooming):
		#self.get_parent_node_3d().global_position = lerp(self.global_position, self.desired_cam_pos, self.position_blend)
		self.global_position = lerp(self.global_position, self.desired_cam_pos, self.position_blend)  
	self.position_blend += delta

# TODO: Move to globe node
func _get_surface_distance_raycast() -> float:
	self.ray_caster.position = self.global_position
	# The ray will have to be cast towards the surface normal's inverse
	var normal : Vector3 = self.globe_normal_finder.get_normal(self.global_position, self.globe_node)
	self.ray_caster.target_position = -normal * RADII
	var point : Vector3 = self.ray_caster.get_collision_point()
	return self.global_position.distance_to(point)

func adjust_far_and_near(delta: float) -> void:
	#So, here let's calculate the amount of z-far based on the distance
	#It should be about 1.5 radii
	self.far = self.global_position.length() * 1.8

func calculate_rotation_speed(delta: float) -> float:
	#Interpolate (linearly, idk) to set a rotation between 1 (far) and 0 (close)
	var diffStep : float = self.distance_to_surface / (RADII * 2)
	var finalSpeed : float = clampf(lerpf(0, self.initial_rotation_speed, diffStep), 0, self.initial_rotation_speed) * delta * diffStep
	if (self.distance_to_surface < 3000 * KM):
		finalSpeed *= 0.1
	return finalSpeed

func calculate_speed_by_distance(current_depth: float, target_depth: float) -> float:
	var distance : float = absf(current_depth - target_depth)
	#Always do 20% of the distance as the amount
	var step : float = distance * 0.2
	return step

func handle_input(delta: float) -> void:
	if (Input.is_key_pressed(KEY_SPACE) && !self.loaded):
		self.initial_zoom_speed = self.cam_zoom_speed
		self.loaded = true
	if (Input.is_key_pressed(KEY_KP_ADD) || Input.is_key_pressed(KEY_PLUS)):
		self.move_speed = lerpf(self.move_speed, self.move_speed * 1.2, delta * 2)
	if (Input.is_key_pressed(KEY_KP_SUBTRACT) || Input.is_key_pressed(KEY_MINUS)):
		self.move_speed = lerpf(self.move_speed, self.move_speed * 0.8, delta * 2)
	if (Input.is_mouse_button_pressed(MOUSE_BUTTON_RIGHT)):
		var mouse_velocity : Vector2 = Input.get_last_mouse_velocity()
		var delta_yaw : float = mouse_velocity.x * delta * self.yaw_sensitivity
		var delta_pitch : float = mouse_velocity.y * delta * self.picth_sensitivity
		self.rotate_camera(delta_pitch, delta_yaw)

	var direction = Vector3.ZERO
	var movingBasis : Basis = self.global_transform.basis

	if (Input.is_key_pressed(KEY_Q)):
		direction -= movingBasis.y
	if (Input.is_key_pressed(KEY_E)):
		direction += movingBasis.y

	if (Input.is_key_pressed(KEY_W)):
		direction -= movingBasis.z
	if (Input.is_key_pressed(KEY_S)):
		direction += movingBasis.z

	if (Input.is_key_pressed(KEY_D)):
		direction += movingBasis.x
	if (Input.is_key_pressed(KEY_A)):
		direction -= movingBasis.x

	camera_walk(direction)

func camera_walk(direction: Vector3) -> void:
	if (desired_cam_pos == Vector3.ZERO):
		self.desired_cam_pos = self.global_position + direction.normalized() * self.move_speed
	else:
		direction = direction.normalized()

		self.desired_cam_pos += direction * self.move_speed
	self.is_moving = direction != Vector3.ZERO
	if (self.is_moving):
		self.last_input_dir = direction.normalized()


func _create_local_directions(up: Vector3) -> Basis:
	# We need to adjust the up direction by x degrees
	#up = rotate_vector_around_x_axis(up, deg_to_rad(self.current_pitch))
	
	var reference = -self.global_basis.z

	if (up.dot(reference) > 0.99):
		reference = self.global_basis.x
	
	# Calculate right vector using cross product
	var right := up.cross(reference).normalized()

	# Calculate forward vector using cross product of right and up
	var forward := right.cross(up).normalized()
	return Basis(right, up, -forward)


func rotate_vector_around_x_axis(vector: Vector3, angle: float) -> Vector3:
	var cos_angle = cos(angle)
	var sin_angle = sin(angle)

	var rotated_y = vector.y * cos_angle - vector.z * sin_angle
	var rotated_z = vector.y * sin_angle + vector.z * cos_angle

	return Vector3(vector.x, rotated_y, rotated_z)


func rotate_camera(delta_pitch: float, delta_yaw: float) -> void:
	self.current_pitch += delta_pitch
	self.current_yaw += delta_yaw
	var yaw_rot = Basis(-global_basis.y, deg_to_rad(delta_yaw))
	var pitch_rot = Basis(self.surface_basis.x, deg_to_rad(delta_pitch))
	self.global_basis = yaw_rot * pitch_rot * self.global_basis

		
func to_ecef_basis(p_basis: Basis) -> Basis:
	var result := Basis() 
	p_basis.x = self.globe_node.get_tx_engine_to_ecef() * p_basis.x;
	p_basis.y = self.globe_node.get_tx_engine_to_ecef() * p_basis.y;
	p_basis.z = self.globe_node.get_tx_engine_to_ecef() * p_basis.z;
	return result

func to_engine_basis(p_basis: Basis) -> Basis:
	var result := Basis() 
	p_basis.x = self.globe_node.get_tx_ecef_to_engine() * p_basis.x;
	p_basis.y = self.globe_node.get_tx_ecef_to_engine() * p_basis.y;
	p_basis.z = self.globe_node.get_tx_ecef_to_engine() * p_basis.z;
	return result


func eus_to_engine_transform(enginePos: Vector3, p_eus_basis: Basis) -> Transform3D:
	# Convert EUS basis to engine coordinate system
	var engine_basis := Basis(
		p_eus_basis.x,  # East becomes X
		p_eus_basis.y,  # Up becomes Y
		-p_eus_basis.z  # South becomes -Z
	).orthonormalized()

	return Transform3D(engine_basis, enginePos)

func update_eus_basis() -> void:
	# Get ECEF normal from Cesium
	var up := self.surface_basis.y

	# Calculate East direction (tangent to latitude circle)
	var east := Vector3.UP.cross(up).normalized()
	if east.length_squared() < 0.1:  # Handle poles
		print("At a pole!")

	# Calculate South direction
	var south := up.cross(east).normalized()

	eus_basis = Basis(east, up, south).orthonormalized()	

func correct_roll(earth_normal: Vector3):
	var forward = self.basis.z
	
	# Project Earth's normal onto camera's view plane
	var projected_normal = earth_normal - earth_normal.project(forward)
	if projected_normal.length_squared() < 1e-6: return  # Edge case handling
	projected_normal = projected_normal.normalized()

	# Find needed roll angle to align camera's up with projected normal
	var current_up = self.basis.y
	var roll_angle = current_up.signed_angle_to(projected_normal, forward)

	# Apply roll correction
	self.transform = self.transform.rotated(forward, roll_angle)	


func apply_zoom(amount: float) -> void:
	self.is_zooming = true
	var cam_pos_ecef : Vector3 = self.globe_node.get_tx_engine_to_ecef() * self.global_position
	var cam_delta : Vector3 = cam_pos_ecef - self.mouse_ecef_position
	var cam_factor : float = pow(self.cam_zoom_speed, amount) + self.correction_speed
	var desired_cam_delta : Vector3 = cam_delta * cam_factor
	
	#If the desired_cam_delta's magnitude is less than a certain amount, apply the zoom
	
	var desired_cam_pos_ecef = self.mouse_ecef_position + desired_cam_delta
	self.desired_cam_pos = globe_node.get_tx_ecef_to_engine() * desired_cam_pos_ecef

func get_mouse_engine_surface_pos() -> Vector3:
	var mouse_engine_pos : Vector3 = self.globe_node.get_tx_ecef_to_engine() * self.mouse_ecef_position
	var inverse_direction := (self.global_position - mouse_engine_pos).normalized()
	const aribitrary_add_length = 1000
	return mouse_engine_pos + inverse_direction * aribitrary_add_length

func get_dir_to_mouse() -> Vector3:
	return (self.mouse_engine_position - self.global_position).normalized()
