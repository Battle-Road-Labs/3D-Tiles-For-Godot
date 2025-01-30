extends Node3D
class_name GlobeNormalFinder

func get_normal(global_pos: Vector3, globe_node: CesiumGlobe) -> Vector3:
	# Get the dimensions of the globe
	var surface_radius : float = globe_node.get_ellipsoid_dimensions().x
	# Place a point on the same part of the surface that we're in
	var origin : Vector3 = globe_node.get_global_center_position()
	# But projected towards the surface
	var edge_pos : Vector3 = self.project_position_on_edge(surface_radius, global_pos, origin)
	#The normal should be the inverse of the direction going to the center
	var normal : Vector3 = -(origin - edge_pos)
	return normal.normalized()

func project_position_on_edge(radius: float, global_pos: Vector3, origin: Vector3) -> Vector3:
	# Intersection onto a sphere: 
	# P = global_pos - origin
	var center_offset_pos : Vector3 = global_pos - origin;
	# |P| = sqrt(x'^2 + y'^2 + z'^2)
	var magnitude : float = center_offset_pos.length()
	#Q = (radius/|P|)*P
	var scaled_v : Vector3 = (radius / magnitude) * center_offset_pos;
	var projected_pos : Vector3 = scaled_v + origin
	return projected_pos
