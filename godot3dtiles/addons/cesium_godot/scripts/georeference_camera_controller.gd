extends Camera3D

class_name GeoreferenceCameraController

@export
var globe_node : CesiumGDGeoreference

@export
var tileset : CesiumGDTileset
		
@export
var move_speed : float = 100

@export
var rotation_speed : float = 2

var desired_cam_pos : Vector3 = Vector3.ZERO

var loaded : bool

var is_moving : bool = false

@export
var info_labels_ui : InfoLabelsUI

var is_orbiting : bool = false

func _ready() -> void:
	self.loaded = false

func _process(delta: float) -> void:
	handle_input(delta)
	if (self.loaded):
		self.adjust_far_and_near()
		self.tileset.test_rendering(self.globe_node.get_tx_engine_to_ecef() * self.global_transform)

	if (self.info_labels_ui != null):
		self.info_labels_ui.update_move_speed(self.move_speed)
	if (self.is_orbiting):
		var mouse_vel : Vector2 = Input.get_last_mouse_velocity().normalized()
		self.rotate_around_globe(Vector3(mouse_vel.y, -mouse_vel.x, 0), self.rotation_speed, delta)

func handle_input(delta: float):
	control_input()
	movement_input(delta)
	orbiting_input()

func control_input():
	if (!self.loaded && Input.is_action_just_pressed("Jump")):
		self.loaded = true
	if (self.loaded && Input.is_action_just_pressed("Jump")):
		save()

func movement_input(delta: float):
	var direction = Vector3.ZERO
	if (Input.is_action_pressed("camera_down")):
		direction -= self.global_transform.basis.y
	if (Input.is_action_pressed("camera_up")):
		direction += self.global_transform.basis.y
	var forward_dir : Vector3 = -self.global_transform.basis.z
	if (Input.is_action_pressed("up")):
		direction += forward_dir
	if (Input.is_action_pressed("down")):
		direction -= forward_dir
	var right_dir = self.global_transform.basis.x
	if (Input.is_action_pressed("right")):
		direction += right_dir
	if (Input.is_action_pressed("left")):
		direction -= right_dir
	if (Input.is_action_pressed("IncreaseMoveSpeed")):
		self.move_speed = lerpf(self.move_speed, self.move_speed * 1.2, delta * 2) 
	if (Input.is_action_pressed("DecreaseMoveSpeed")):
		self.move_speed = lerpf(self.move_speed, self.move_speed * 0.8, delta * 2)
	
	camera_walk_ecef(direction.normalized())

func orbiting_input():
	if (Input.is_action_pressed("Click") && !self.is_orbiting):
		self.is_orbiting = true
	if (Input.is_action_just_released("Click") && self.is_orbiting):
		self.is_orbiting = false

func save():
	var save = PackedScene.new()
	var root : Node3D = get_tree().current_scene
	for c in root.get_children():
		c.set_owner(root)
		for a in c.get_children():
			a.set_owner(root)
			for b in a.get_children():
				b.set_owner(root)
	save.pack(root);

	ResourceSaver.save(save, "res://test_packed.tscn");

func update_camera(delta: float):
	if (!self.is_moving):
		return
	if (self.is_moving):
		var t := Engine.get_physics_interpolation_fraction()
		self.global_position = self.desired_cam_pos
		return

func camera_walk_ecef(direction: Vector3) -> void:
	if (direction == Vector3.ZERO):
		return
	
	direction *= -self.move_speed
	
	self.globe_node.ecefX -= direction.x
	self.globe_node.ecefY += direction.z
	self.globe_node.ecefZ += direction.y

func camera_walk_physical(direction: Vector3) -> void:
	if (desired_cam_pos == Vector3.ZERO):
		self.desired_cam_pos = self.global_position + direction.normalized() * self.move_speed
	else:
		self.desired_cam_pos += direction.normalized() * self.move_speed
	self.is_moving = direction != Vector3.ZERO
	update_camera(0)

func adjust_far_and_near() -> void:
	#So, here let's calculate the amount of z-far based on the distance
	#It should be about 1.5 radii
	if (!self.tileset.is_initial_loading_finished()):
		return
	self.far = 1e12

func rotate_around_globe(axis: Vector3, rotation_radians: float, delta: float) -> void:
	if (axis == Vector3.ZERO):
		return
	# Pivot is the camera
	# Position is the globe
	
	var pivot : Vector3 = self.global_position
	# Maybe invert this
	var target_pos : Vector3 = Vector3(self.globe_node.ecefX, self.globe_node.ecefY, self.globe_node.ecefZ)
	target_pos = self.globe_node.get_initial_tx_ecef_to_engine() * target_pos
	
	# Translate the object to the origin (pivot point)
	var translated_position : Vector3 = target_pos - pivot
	
	# Create a rotation matrix
	var rotation_amount : float = rotation_radians * delta
	var rotation_matrix := Transform3D(Basis(axis, rotation_amount), Vector3())
	
	# Apply the rotation
	var rotated_position := rotation_matrix * translated_position
	
	# Translate the object back to its original position relative to the pivot
	var final_position = rotated_position + pivot
	
	final_position = self.globe_node.get_initial_tx_engine_to_ecef() * final_position
	
	# Set the ecef coordinates	
	self.globe_node.ecefX = final_position.x
	self.globe_node.ecefY = final_position.y
	self.globe_node.ecefZ = final_position.z
	self.look_at(self.globe_node.global_position)
	# Just rotate the globe around the given axis now
	#self.globe_node.rotate(axis, rotation_amount)
