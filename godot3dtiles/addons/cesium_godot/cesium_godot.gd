@tool
extends EditorPlugin

const editorAddon := preload("res://addons/cesium_godot/panels/cesium_panel.tscn")

const token_panel_popup := preload("res://addons/cesium_godot/panels/token_panel.tscn")

const CESIUM_GLOBE_NAME = "CesiumGlobe"
const CESIUM_TILESET_NAME = "CesiumGDTileset"

var docked_scene : Control

var add_button : Button
var upload_button : Button
var learn_button : Button
var help_button : Button
var token_button : Button
var sign_out_button : Button
var connect_button : Button
var blank_tileset_button : Button
var geo_ref_checkbox : CheckButton
var dynamic_camera_button : Button

var auth_controller_node : OAuthController = null
var cesium_builder_node : CesiumAssetBuilder = null

# So, for some reason we cannot have a custom popup because some definitions get lost in instantiation
# We don't really know why this is, but we circunvent it by just storing the data on another class
var token_panel: Popup = null

var token_panel_data : TokenPanelData = null

func _enter_tree() -> void:
	self.docked_scene = editorAddon.instantiate()
	add_control_to_dock(EditorPlugin.DOCK_SLOT_RIGHT_UL, self.docked_scene)
	self.set_utility_buttons_enabled(false)
	self.auth_controller_node = OAuthController.new()
	self.add_child(self.auth_controller_node)
	self.cesium_builder_node = CesiumAssetBuilder.new()
	self.add_child(self.cesium_builder_node)
	self.token_panel = self.token_panel_popup.instantiate()
	self.token_panel_data = TokenPanelData.new()
	self.add_child(self.token_panel)
	self.token_panel.hide()
	print("Enabled Cesium plugin")
	self.init_buttons()


func _exit_tree() -> void:
	print("Disabled Cesium plugin")
	remove_control_from_docks(self.docked_scene)
	self.docked_scene.free()

func init_buttons() -> void:
	self.add_button = self.docked_scene.find_child("AddButton") as Button
	self.upload_button = self.docked_scene.find_child("UploadButton") as Button
	self.token_button = self.docked_scene.find_child("TokenButton") as Button
	self.learn_button = self.docked_scene.find_child("LearnButton") as Button
	self.help_button = self.docked_scene.find_child("HelpButton") as Button
	self.sign_out_button = self.docked_scene.find_child("SignOutButton") as Button
	self.connect_button = self.docked_scene.find_child("ConnectButton") as Button
	self.blank_tileset_button = self.docked_scene.find_child("BlankTilesetButton") as Button
	self.dynamic_camera_button = self.docked_scene.find_child("DynamicCameraButton") as Button
	self.geo_ref_checkbox = self.docked_scene.find_child("GeoRefCheckButton") as CheckButton
	self.token_panel_data.initialize_fields(self.token_panel)
	# Connect to their signals
	self.upload_button.pressed.connect(on_upload_pressed)
	self.learn_button.pressed.connect(on_learn_pressed)
	self.help_button.pressed.connect(on_help_pressed)
	self.connect_button.pressed.connect(on_connect_pressed)
	self.sign_out_button.pressed.connect(on_sign_out_pressed)
	self.blank_tileset_button.pressed.connect(add_tileset)
	self.dynamic_camera_button.pressed.connect(create_dynamic_camera)
	self.geo_ref_checkbox.toggled.connect(on_georef_checked)
	self.token_button.pressed.connect(on_token_panel_pressed)

func on_token_panel_pressed() -> void:
	self.token_panel.popup()

func set_utility_buttons_enabled(enabled: bool) -> void:
	var utilityButtons := self.get_utility_buttons();
	for btn in utilityButtons:
		if btn == null: continue
		(btn as Button).disabled = !enabled

func _process(delta: float) -> void:
	if self.auth_controller_node == null:
		return
	if (self.auth_controller_node.signedIn):
		connect_button.text = "Connected!"
		connect_button.disabled = true
	self.set_utility_buttons_enabled(self.auth_controller_node.signedIn)

# All of the buttons that become available once the user logs in
func get_utility_buttons() -> Array[Button]:
	return [self.add_button, self.upload_button, self.sign_out_button]

func on_upload_pressed() -> void:
	const url := "https://ion.cesium.com/addasset?"	
	OS.shell_open(url)

func on_learn_pressed() -> void:
	CesiumGDPanel.open_learn_page()

func on_help_pressed() -> void:
	CesiumGDPanel.open_help_page()

func on_connect_pressed() -> void:
	#Open the browser with a TCP server, or show the URL
	if self.auth_controller_node.is_connecting:
		self.auth_controller_node.cancel_connection()
		self.connect_button.text = "Connect to Cesium ION"
		return
	self.auth_controller_node.get_auth_code()
	self.connect_button.text = "Cancel Connecting"

func on_sign_out_pressed():
	self.auth_controller_node.sign_out()
	self.connect_button.disabled = false
	self.connect_button.text = "Connect to Cesium ION"

func on_georef_checked(is_checked: bool) -> void:
	self.cesium_builder_node.use_georeferences = is_checked

func add_tileset():
	self.cesium_builder_node.instantiate_tileset(CesiumAssetBuilder.TILESET_TYPE.Blank)

func create_dynamic_camera():
	print("Create dynamic camera!")
	self.cesium_builder_node.instantiate_dynamic_cam()
