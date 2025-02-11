extends Camera3D

class_name GeoreferenceCameraController

@export
var globe_node : CesiumGlobe

@export
var tileset : CesiumGDTileset
		
@export
var move_speed : float = 100

@export
var rotation_speed : float = 2

var desired_cam_pos : Vector3 = Vector3.ZERO

var loaded : bool

var is_moving : bool = false

var surface_basis: Basis

@export
var info_labels_ui : InfoLabelsUI

func _ready() -> void:
	self.loaded = false

func _process(delta: float) -> void:
	self.surface_basis = self.calculate_surface_basis()
	handle_input(delta)
	self.adjust_far_and_near()
	if (self.loaded):
		self.tileset.update_tileset(self.globe_node.get_tx_engine_to_ecef() * self.global_transform)

	if (self.info_labels_ui != null):
		self.info_labels_ui.update_move_speed(self.move_speed)
	
func handle_input(delta: float):
	control_input()
	movement_input(delta)

func control_input():
	if (Input.is_key_pressed(KEY_SPACE) && !self.loaded):
		self.loaded = true

	
func calculate_surface_basis() -> Basis:
	var engineToEcefTransform := Vector3(self.globe_node.ecefX, self.globe_node.ecefY, self.globe_node.ecefZ)
	var cesiumNormal : Vector3 = self.globe_node.get_normal_at_surface_pos(engineToEcefTransform)
	var up : Vector3 = self.globe_node.get_initial_tx_ecef_to_engine() * cesiumNormal
	var reference = -self.global_basis.z
	
	var debug_position : Vector3 = self.global_position + (reference * 10)
	DebugDraw3D.draw_arrow(debug_position, debug_position + up, Color.YELLOW, 1)
	var dotProduct := up.dot(reference)
	print("Dot with fwd: " + str(dotProduct))
	if (dotProduct > 0.99):
		reference = self.global_basis.y
	
	# Calculate right vector using cross product
	var right := up.cross(reference).normalized()
	DebugDraw3D.draw_arrow(debug_position, debug_position + right, Color.RED, 1)	

	# Calculate forward vector using cross product of right and up
	var forward := right.cross(up).normalized()

	DebugDraw3D.draw_arrow(debug_position, debug_position - forward, Color.BLUE, 1)
	
	return Basis(right, up, -forward)
	return Basis()
	
func movement_input(delta: float):

	if (Input.is_mouse_button_pressed(MOUSE_BUTTON_RIGHT)):
		var mouse_velocity : Vector2 = Input.get_last_mouse_velocity()
		var delta_yaw : float = mouse_velocity.x * delta * self.rotation_speed
		var delta_pitch : float = mouse_velocity.y * delta * self.rotation_speed
		self.rotate_camera(delta_pitch, delta_yaw)
	
	var direction := Vector3.ZERO
	var movingBasis : Basis = self.global_transform.basis

	if (Input.is_key_pressed(KEY_KP_ADD) || Input.is_key_pressed(KEY_PLUS)):
		self.move_speed = lerpf(self.move_speed, self.move_speed * 1.2, delta * 2)
	if (Input.is_key_pressed(KEY_KP_SUBTRACT) || Input.is_key_pressed(KEY_MINUS)):
		self.move_speed = lerpf(self.move_speed, self.move_speed * 0.8, delta * 2)

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
	#direction = self.adjust_move_direction_to_surface(direction)
	camera_walk_ecef(direction.normalized())

func adjust_move_direction_to_surface(raw_direction: Vector3) -> Vector3:
	var engineToEcefTransform := Vector3(self.globe_node.ecefX, self.globe_node.ecefY, self.globe_node.ecefZ)
	var cesiumNormal : Vector3 = self.globe_node.get_normal_at_surface_pos(engineToEcefTransform)
	var upEngine : Vector3 = self.globe_node.get_initial_tx_ecef_to_engine() * cesiumNormal
	
	var result : Vector3 = self.global_basis.x * raw_direction.x + self.global_basis.z * raw_direction.z - upEngine * raw_direction.y
	return result

func update_camera():
	if (!self.is_moving):
		return	
	self.global_position = self.desired_cam_pos

func camera_walk_ecef(direction: Vector3) -> void:
	if (direction == Vector3.ZERO):
		return
	
	direction *= -self.move_speed
	
	self.globe_node.ecefX -= direction.x
	self.globe_node.ecefY += direction.z
	self.globe_node.ecefZ += direction.y

func adjust_far_and_near() -> void:
	#So, here let's calculate the amount of z-far based on the distance
	#It should be about 1.5 radii
	if (!self.tileset.is_initial_loading_finished()):
		return
	self.far = 35358652
	self.near = 9

func rotate_camera(delta_pitch: float, delta_yaw: float) -> void:
	# Adjust roll to horizon
	# And rotate normally
	self.rotate_y(deg_to_rad(-delta_yaw))
	self.get_parent_node_3d().rotate_x(deg_to_rad(-delta_pitch))	
