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

var curr_yaw: float

var curr_pitch: float

@export
var info_labels_ui : InfoLabelsUI

func _ready() -> void:
	self.loaded = false
	Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)

func _process(delta: float) -> void:
	self.surface_basis = self.calculate_surface_basis()
	handle_input(delta)
	self.update_camera_rotation()
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
	var up : Vector3 = self.globe_node.get_normal_at_surface_pos(engineToEcefTransform)
	var reference = -self.global_basis.z
	
	var debug_position : Vector3 = self.global_position + (reference * 10)
	var dotProduct := up.dot(reference)
	
	if (dotProduct > 0.99):
		reference = self.global_basis.x
	
	# Calculate right vector using cross product
	var right := up.cross(reference).normalized()

	# Calculate forward vector using cross product of right and up
	var forward := right.cross(up).normalized()
	var result := Basis(right, up, -forward)
	DebugDraw3D.draw_gizmo(Transform3D(result, debug_position))
	#DebugDraw3D.draw_gizmo(Transform3D(self.global_basis, debug_position))
	return result

func movement_input(delta: float):
	#if (Input.is_mouse_button_pressed(MOUSE_BUTTON_RIGHT)):
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
	if (Input.is_key_pressed(KEY_KP_6)):
		rotate_z(delta * 0.5)
	if (Input.is_key_pressed(KEY_KP_4)):
		rotate_z(-delta * 0.5)
	
	#direction = self.adjust_move_direction_to_surface(direction)
	
	var startPos := self.global_position - (self.global_basis.z * 10)
	DebugDraw3D.draw_arrow(startPos, startPos + direction, Color.ORANGE, 0.1)
	var ecefDir : Vector3 = self.globe_node.get_initial_tx_engine_to_ecef() * direction
	
	camera_walk_ecef(-ecefDir.normalized())

func adjust_move_direction_to_surface(raw_direction: Vector3) -> Vector3:
	var engineToEcefTransform := Vector3(self.globe_node.ecefX, self.globe_node.ecefY, self.globe_node.ecefZ)
	var upEngine : Vector3 = self.surface_basis.y
	var result : Vector3 = self.global_basis.x * raw_direction.x + self.global_basis.z * raw_direction.z + upEngine * raw_direction.y
	return result

func update_camera():
	if (!self.is_moving):
		return	
	self.global_position = self.desired_cam_pos

func camera_walk_ecef(direction: Vector3) -> void:
	if (direction == Vector3.ZERO):
		return
	
	direction *= -self.move_speed
	
	self.globe_node.ecefX += direction.x
	self.globe_node.ecefY += direction.y
	self.globe_node.ecefZ += direction.z

func adjust_far_and_near() -> void:
	#So, here let's calculate the amount of z-far based on the distance
	#It should be about 1.5 radii
	if (!self.tileset.is_initial_loading_finished()):
		return
	self.far = 35358652
	self.near = 9

func update_camera_rotation() -> void:
	# Store original basis axes before any rotations
	var y_axis = self.surface_basis.y.normalized()
	var x_axis = self.surface_basis.x.normalized()

	# Apply yaw first around original Y axis
	var moddedBasis: Basis = self.surface_basis.rotated(y_axis, -curr_yaw)
	# Apply pitch around original X axis (now rotated by yaw)
	# Using the updated X axis from the basis after yaw rotation
	moddedBasis = moddedBasis.rotated(moddedBasis.x, curr_pitch)
	moddedBasis.x = -moddedBasis.x

	self.basis = moddedBasis
	print("Pitch: " + str(self.curr_pitch))
	self.curr_yaw = 0

	

func rotate_camera(delta_pitch: float, delta_yaw: float) -> void:
    # Apply yaw rotation around local Y axis
	self.curr_yaw += delta_yaw
	self.curr_pitch = clampf(self.curr_pitch + delta_pitch, -0.95, 0.95)
