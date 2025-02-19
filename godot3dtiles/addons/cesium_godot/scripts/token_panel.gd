class_name TokenPanelData

enum TokenUsageType { New, Existing, Specific }

var existing_token_check: CheckBox
var specific_token_check: CheckBox
var config_picker: EditorResourcePicker
var new_token_check: CheckBox
var create_or_use_token_button: Button
var test_token_button: Button
var specific_token_name: TextEdit

var request_node: HTTPRequest

var current_token_usage: TokenUsageType = TokenUsageType.Specific

const default_config: CesiumGDConfig = preload("res://addons/cesium_godot/cesium_gd_config.tres")

func initialize_fields(token_panel: Popup) -> void:
	self.existing_token_check = token_panel.find_child("ExistingTokenCheck") as CheckBox
	self.specific_token_check = token_panel.find_child("SpecificTokenCheck") as CheckBox
	self.new_token_check = token_panel.find_child("NewTokenCheck") as CheckBox
	self.create_or_use_token_button = token_panel.find_child("CreateOrUseToken") as Button
	self.test_token_button = token_panel.find_child("TestToken") as Button
	# Then add the ResourcePicker
	var configuration_container = token_panel.find_child("ConfigurationContainer")
	self.config_picker = EditorResourcePicker.new()
	self.config_picker.custom_minimum_size = Vector2(200, 50)
	self.config_picker.edited_resource = self.default_config
	configuration_container.add_child(self.config_picker)
	
	self.specific_token_name = self.find_sibling(self.specific_token_check, "TokenName") as TextEdit
	self.specific_token_name.text = (self.config_picker.edited_resource as CesiumGDConfig).accessToken
	
	self.existing_token_check.toggled.connect(on_existing_token_check)
	self.specific_token_check.toggled.connect(on_specific_token_check)
	self.new_token_check.toggled.connect(on_new_token_check)
	self.token_panel.about_to_popup.connect(initialize_panel_buttons_from_state)
	self.create_or_use_token_button.pressed.connect(apply_or_create_token)
	self.test_token_button.pressed.connect(on_test_button_pressed)
	

	self.request_node = HTTPRequest.new()
	token_panel.add_child(self.request_node)
	self.request_node.request_completed.connect(_on_request_completed)
	


func _on_request_completed(result: int, responseCode: int, headers: PackedStringArray, body: PackedByteArray) -> void:
	pass

func initialize_panel_buttons_from_state() -> void:
	var usage = self.current_token_usage
	# Manually call the events
	if (usage == TokenPanelData.TokenUsageType.New):
		self.on_new_token_check(true)
	if (usage == TokenPanelData.TokenUsageType.Specific):
		self.on_specific_token_check(true)
	if (usage == TokenPanelData.TokenUsageType.Existing):
		self.on_existing_token_check(true)

func on_new_token_check(checked: bool) -> void:
	if (!checked):
		return
	self.specific_token_check.button_pressed = false
	self.existing_token_check.button_pressed = false
	self.current_token_usage = TokenPanelData.TokenUsageType.New
			

func on_specific_token_check(checked: bool) -> void:
	if (!checked):
		return
	self.new_token_check.button_pressed = false
	self.existing_token_check.button_pressed = false
	self.current_token_usage = TokenPanelData.TokenUsageType.Specific
	pass
	
func on_existing_token_check(checked: bool) -> void:
	if (!checked):
		return
	self.new_token_check.button_pressed = false
	self.specific_token_check.button_pressed = false
	self.current_token_usage = TokenPanelData.TokenUsageType.Existing
	pass


func on_test_button_pressed() -> void:
	pass


func test_token() -> void:
	# Get the config
	print("Token working")
	
	print("Token not working")
	

func apply_or_create_token() -> void:
	var usage := self.current_token_usage
	if (usage == TokenUsageType.Specific):
		var config := self.config_picker.edited_resource as CesiumGDConfig
		config.accessToken = self.specific_token_name.text

	OS.alert("Token changed in configuration, you can now close this window...", "Changes applied!")


# Finds a sibling node by name or predicate function.
# Returns `null` if no matching sibling is found.
func find_sibling(node: Node,
	target_name: String = "", 
	predicate: Callable = func(node: Node): return node.name == target_name) -> Node:
	var parent = node.get_parent()
	if not parent:
		return null

	for child in parent.get_children():
		if child != self and predicate.call(child):
			return child
	return null
