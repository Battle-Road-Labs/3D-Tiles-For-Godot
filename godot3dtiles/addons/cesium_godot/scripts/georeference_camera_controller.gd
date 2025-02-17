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

var moving_direction: Vector3
var last_hit_distance: float


const RADII := 6378137.0

@export
var info_labels_ui : InfoLabelsUI

func _ready() -> void:
	self.loaded = false
	Input.set_mouse_mode(Input.MOUSE_MODE_CAPTURED)


func _physics_process(delta: float) -> void:
	self.adjust_speed()

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
	if (direction != Vector3.ZERO):
		self.moving_direction = direction.normalized()
	var ecefDir : Vector3 = self.globe_node.get_initial_tx_engine_to_ecef() * direction
	
	camera_walk_ecef(-ecefDir.normalized())

func adjust_move_direction_to_surface(raw_direction: Vector3) -> Vector3:
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
	self.curr_yaw = 0

	

func rotate_camera(delta_pitch: float, delta_yaw: float) -> void:
	# Apply yaw rotation (unchanged)
	self.curr_yaw += delta_yaw

	# Get the current forward direction of the camera
	var camera_forward: Vector3 = -self.global_basis.z.normalized()
	# Get the reference "surface" forward direction (e.g., world up or target direction)
	var surface_forward: Vector3 = self.surface_basis.z.normalized()

	# Calculate the signed angle between vectors (in degrees)
	var cross = camera_forward.cross(surface_forward)
	var dot = camera_forward.dot(surface_forward)
	var unsigned_angle = rad_to_deg(acos(clamp(dot, -1.0, 1.0)))

	# Determine sign using the cross product's direction relative to the camera's right vector
	var camera_right = self.global_basis.x.normalized()
	var sign = cross.dot(camera_right)  # Positive = above surface, Negative = below
	var signed_angle = unsigned_angle * sign(sign)

	# Clamp the pitch based on the signed angle
	var desired_pitch = self.curr_pitch + delta_pitch

	# We have a negative delta and the signed angle is already at its min
	if (signed_angle > -110 && signed_angle < 0  && delta_pitch > 0):
		return
	if (signed_angle < 110 && signed_angle > 0 && delta_pitch < 0):
		return
	self.curr_pitch = desired_pitch
		
	
	

func _get_surface_distance_raycast() -> float:
	# Calculate the moving direction for the second raycast
	var space_state = get_world_3d().direct_space_state

	# --- First Raycast: Using -surface_basis.y ---
	var ray_query = PhysicsRayQueryParameters3D.new()
	ray_query.from = global_position
	ray_query.to = global_position + (-surface_basis.y * RADII * 2)
	ray_query.hit_from_inside = true
	ray_query.hit_back_faces = true
	ray_query.exclude = [self]
	ray_query.collision_mask = 1  # Adjust this mask as needed

	var result: Dictionary = space_state.intersect_ray(ray_query)

	# --- Second Raycast: Using the moving direction ---
	ray_query.to = global_position + (moving_direction * RADII * 2)
	var secondResult: Dictionary = space_state.intersect_ray(ray_query)

	# Debug draw the two rays
	DebugDraw3D.draw_ray(global_position - global_basis.z * 10, -surface_basis.y, RADII * 2, Color.CYAN)
	DebugDraw3D.draw_ray(global_position - global_basis.z * 10, moving_direction, RADII * 2, Color.CYAN)

	# Get the collision distances from the raycasts (default to RADII)
	var distanceToFloor: float = RADII
	if result:
		distanceToFloor = global_position.distance_to(result.position)
		last_hit_distance = distanceToFloor

	var distanceToMove: float = RADII
	if secondResult:
		distanceToMove = global_position.distance_to(secondResult.position)
		last_hit_distance = distanceToMove

	# Determine the smallest distance from the raycasts
	var closest_distance: float = distanceToFloor
	if distanceToMove < closest_distance:
		closest_distance = distanceToMove

	# --- Additional Sphere Cast ---
	# Create a sphere with a radius of 5
	var sphere_radius: float = 5.0
	var sphere_shape = SphereShape3D.new()
	sphere_shape.radius = sphere_radius

	var shape_query = PhysicsShapeQueryParameters3D.new()
	shape_query.shape = sphere_shape
	shape_query.transform = Transform3D(Basis(), global_position + moving_direction)
	shape_query.exclude = [self]
	shape_query.collision_mask = 1  # Adjust based on your layers

	DebugDraw3D.draw_sphere(global_position + moving_direction - global_basis.z * last_hit_distance, 5, Color.YELLOW)
	var shape_results = space_state.intersect_shape(shape_query, 32)

	# Iterate over the sphere cast results and update the closest distance if needed
	if shape_results.size() > 0:
		for res in shape_results:
			var collider := instance_from_id(res.collider_id) as Node3D
			var shape_distance = global_position.distance_to(collider.position)
			if shape_distance < closest_distance:
				closest_distance = shape_distance
				last_hit_distance = shape_distance

	return closest_distance

func adjust_speed() -> float:
	# The speed has to go through the curve
	_get_surface_distance_raycast()
	#Always move by x% of the total distance
	self.move_speed = clampf(self.last_hit_distance * 0.005, 1, RADII * 3)
	return 0
