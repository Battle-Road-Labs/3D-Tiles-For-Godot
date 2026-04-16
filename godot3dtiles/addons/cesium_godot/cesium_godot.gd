@tool
extends EditorPlugin

class_name CesiumGodotEditorTool

const editorAddon := preload("res://addons/cesium_godot/panels/cesium_panel.tscn")

const token_panel_popup := preload("res://addons/cesium_godot/panels/token_panel.tscn")

const plus_icon := preload("res://addons/cesium_godot/resources/icons/plus.svg")

const CESIUM_GLOBE_NAME = "CesiumGeoreference"
const CESIUM_TILESET_NAME = "Cesium3DTileset"

var docked_scene : Control

var add_button : Button
var upload_button : Button
var learn_button : Button
var help_button : Button
var token_button : Button
var sign_out_button : Button
var connect_button : Button
var connected_indicator : HBoxContainer
var blank_tileset_button : Button
var dynamic_camera_button : Button
var orbit_camera_button : Button
var ion_button_holder : Control

var ion_asset_buttons : Array[Button] = []

var auth_controller_node : OAuthController = null
var cesium_builder_node : CesiumGDAssetBuilder = null
var request_node : HTTPRequest = null

# So, for some reason we cannot have a custom popup because some definitions get lost in instantiation
# We don't really know why this is, but we circunvent it by just storing the data on another class
var token_panel: Popup = null

var token_panel_data : TokenPanelData = null

func _enter_tree() -> void:
	self.set_process(false)
	self.docked_scene = editorAddon.instantiate()
	add_control_to_dock(EditorPlugin.DOCK_SLOT_RIGHT_UL, self.docked_scene)
	self.set_session_buttons_enabled(false)
	self.auth_controller_node = OAuthController.new()
	self.add_child(self.auth_controller_node)
	self.cesium_builder_node = CesiumGDAssetBuilder.new()
	self.add_child(self.cesium_builder_node)
	self.token_panel = self.token_panel_popup.instantiate()
	self.token_panel_data = TokenPanelData.new()
	self.add_child(self.token_panel)
	self.token_panel.hide()
	self.add_inspector_plugin(CesiumTooltips.new())
	print("Enabled Cesium plugin")
	self.request_node = HTTPRequest.new()
	self.add_child(self.request_node)
	self.init_buttons()


func _exit_tree() -> void:
	print("Disabled Cesium plugin")
	remove_control_from_docks(self.docked_scene)
	self.docked_scene.free()

func init_buttons() -> void:
	self.add_ion_buttons()
	self.add_button = self.docked_scene.find_child("AddButton") as Button
	self.upload_button = self.docked_scene.find_child("UploadButton") as Button
	self.token_button = self.docked_scene.find_child("TokenButton") as Button
	self.learn_button = self.docked_scene.find_child("LearnButton") as Button
	self.help_button = self.docked_scene.find_child("HelpButton") as Button
	self.sign_out_button = self.docked_scene.find_child("SignOutButton") as Button
	self.connect_button = self.docked_scene.find_child("ConnectButton") as Button
	self.connected_indicator = self.docked_scene.find_child("IONConnected") as HBoxContainer
	self.blank_tileset_button = self.docked_scene.find_child("BlankTilesetButton") as Button
	self.dynamic_camera_button = self.docked_scene.find_child("DynamicCameraButton") as Button
	self.orbit_camera_button = self.docked_scene.find_child("OrbitCameraButton") as Button
	self.token_panel_data.initialize_fields(self.token_panel)

	# Connect to their signals
	self.upload_button.pressed.connect(on_upload_pressed)
	self.learn_button.pressed.connect(on_learn_pressed)
	self.help_button.pressed.connect(on_help_pressed)
	self.connect_button.pressed.connect(on_connect_pressed)
	self.sign_out_button.pressed.connect(on_sign_out_pressed)
	self.blank_tileset_button.pressed.connect(add_tileset)
	self.dynamic_camera_button.pressed.connect(create_dynamic_camera)
	self.orbit_camera_button.pressed.connect(create_orbit_camera)
	self.token_button.pressed.connect(on_token_panel_pressed)

	self.docked_scene.find_child("VisitDepotButton").pressed.connect(on_visit_depot)

func on_visit_depot() -> void:
	OS.shell_open("https://ion.cesium.com/assetdepot")

func on_token_panel_pressed() -> void:
	self.token_panel.popup()

func set_session_buttons_enabled(enabled: bool) -> void:
	var utilityButtons := self.get_utility_buttons();
	utilityButtons.append_array(self.ion_asset_buttons)
	for btn in utilityButtons:
		if btn == null: continue
		(btn as Button).disabled = !enabled

func _process(delta: float) -> void:
	if self.auth_controller_node == null:
		return
	if self.auth_controller_node.is_signed_in and connect_button.visible:
		self.toggle_connected(true)
	self.set_session_buttons_enabled(self.auth_controller_node.is_signed_in)

# All of the buttons that become available once the user logs in
func get_utility_buttons() -> Array[Button]:
	return [self.add_button, self.upload_button, self.sign_out_button]

func on_upload_pressed() -> void:
	OS.shell_open("https://ion.cesium.com/addasset?")

func on_learn_pressed() -> void:
	CesiumGDPanel.open_learn_page()

func on_help_pressed() -> void:
	CesiumGDPanel.open_help_page()

func toggle_connected(state: bool) -> void:
	self.connected_indicator.visible = state
	self.connect_button.disabled = state
	self.connect_button.visible = !state

func on_connect_pressed() -> void:
	#Open the browser with a TCP server, or show the URL
	if self.auth_controller_node.is_connecting:
		self.auth_controller_node.cancel_connection()
		self.toggle_connected(false)
		return
	await self.auth_controller_node.get_auth_code()
	self.toggle_connected(true)

func on_sign_out_pressed():
	self.auth_controller_node.sign_out()
	self.toggle_connected(false)

func on_georef_checked(is_checked: bool) -> void:
	self.cesium_builder_node.use_georeferences = is_checked

func add_tileset():
	self.cesium_builder_node.instantiate_tileset(CesiumAssetBuilder.TILESET_TYPE.Blank, "", "Blank")

func create_dynamic_camera():
	self.cesium_builder_node.instantiate_dynamic_cam()

func create_orbit_camera():
	self.cesium_builder_node.instantiate_orbit_cam()

func fetch_ion_asset_list():
	const url := "https://api.cesium.com/v1/assets";
	var token : String = CesiumGDConfig.get_singleton(self).accessToken;
	if token.is_empty():
		return null
	var headers: PackedStringArray = ["Authorization: Bearer " + token]
	var error: int = self.request_node.request(url, headers, HTTPClient.Method.METHOD_GET)
	if (error != OK):
		push_error("Error getting the asset list from Cesium: " + error_string(error))
		return null
	var response = await self.request_node.request_completed
	var status = response[1]
	if status != 200:
		push_error("Cesium Ion asset list request failed with status: " + str(status))
		return null
	var bodyBytes := response[3] as PackedByteArray
	var body := JSON.parse_string(bodyBytes.get_string_from_utf8()) as Dictionary
	if body == null or not body.has("items"):
		push_error("Cesium Ion asset list response missing 'items' key")
		return null
	var by_type: Dictionary[String, Array] = {}
	for item in body.items:
		if item.type not in by_type:
			by_type[item.type] = Array()
		by_type[item.type].append(item)

	return by_type

func make_button_container(type: String, entries: Array):
	var foldable = BoxContainer.new()
	# foldable.title = type
	var margin = MarginContainer.new()
	margin.set_anchors_preset(Control.PRESET_FULL_RECT)
	margin.offset_top = 10
	margin.offset_bottom = 10
	margin.offset_left = 10
	margin.offset_right = 10
	var assets_list = VBoxContainer.new()
	for entry in entries:
		var hbox = HBoxContainer.new()
		var label = Label.new()
		label.text = entry.name
		label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		label.size_flags_vertical = Control.SIZE_FILL
		label.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
		hbox.add_child(label)
		var button = Button.new()
		button.icon = plus_icon
		button.icon_alignment = HORIZONTAL_ALIGNMENT_CENTER
		button.set_meta("ion_name", entry.name)
		button.pressed.connect(func():
			self.cesium_builder_node.instantiate_tileset(int(entry.id), type, entry.name)
		)
		button.mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND
		hbox.add_child(button)
		assets_list.add_child(hbox)
	margin.add_child(assets_list)
	foldable.add_child(margin)
	ion_button_holder.add_child(foldable)

func add_ion_buttons() -> void:
	self.ion_button_holder = self.docked_scene.find_child("IonAssetButtonHolder") as Control
	for child in self.ion_button_holder.get_children():
		self.ion_button_holder.remove_child(child)
	var available_assets = await fetch_ion_asset_list()
	self.set_process(true)
	if available_assets == null:
		push_error("Failed to find any avilable assets!")
		return
	for type in available_assets.keys():
		make_button_container(type, available_assets[type])

func is_http_request_busy(http_request: HTTPRequest) -> bool:
	return http_request.get_http_client_status() in [
		HTTPClient.STATUS_CONNECTING,
		HTTPClient.STATUS_REQUESTING,
		HTTPClient.STATUS_BODY
	]
