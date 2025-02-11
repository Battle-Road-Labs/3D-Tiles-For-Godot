extends Node
class_name InfoLabelsUI

@export
var distance_label : Label

@export
var speed_label : Label

const KM : float = 1000

func update_distance(distance: float) -> void:
	self.distance_label.text = "Distance to surface: "
	if (distance >= KM):
		self.distance_label.text += str(snapped(distance / KM, 0.01)) + "km"
	else:
		self.distance_label.text += str(snapped(distance, 0.01)) + "m"

func debug_text(text: String):
	self.distance_label.text = text

func update_move_speed(speed: float) -> void:
	self.speed_label.text = "Movement speed: " + str(snappedf(speed, 0.01)) + "m/s"

func toggle_visibility() -> void:
	self.distance_label.visible = !self.distance_label.visible
	self.speed_label.visible = !self.speed_label.visible

func _process(delta: float) -> void:
	if (Input.is_action_just_pressed("ToggleLabels")):
		toggle_visibility()
