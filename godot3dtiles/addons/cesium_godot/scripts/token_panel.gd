class_name TokenPanelData

enum TokenUsageType { New, Existing, Specific }

var existing_token_check: CheckBox
var specific_token_check: CheckBox
var config_picker: EditorResourcePicker
var new_token_check: CheckBox
var create_or_use_token_button: Button
var test_token_button: Button
var specific_token_name: TextEdit
var asset_list_group: BoxContainer
var asset_list_items: OptionButton

var token_troubleshooting: TokenTroubleshooting

var request_node: HTTPRequest


var current_token_usage: TokenUsageType = TokenUsageType.Specific

var default_config: CesiumGDConfig = null

func initialize_fields(token_panel: Popup) -> void:
	self.default_config = CesiumGDConfig.get_singleton(token_panel)
	self.token_troubleshooting = TokenTroubleshooting.new()
	token_panel.add_child(self.token_troubleshooting)
	self.token_troubleshooting.owner = token_panel
	self.existing_token_check = token_panel.find_child("ExistingTokenCheck") as CheckBox
	self.specific_token_check = token_panel.find_child("SpecificTokenCheck") as CheckBox
	self.new_token_check = token_panel.find_child("NewTokenCheck") as CheckBox
	self.create_or_use_token_button = token_panel.find_child("CreateOrUseToken") as Button
	self.test_token_button = token_panel.find_child("TestToken") as Button
	self.asset_list_group = token_panel.find_child("AssetListGroup") as BoxContainer
	self.asset_list_items = token_panel.find_child("AssetList") as OptionButton
	# Then add the ResourcePicker
	var configuration_container = token_panel.find_child("ConfigurationContainer")

	self.specific_token_name = self.find_sibling(self.specific_token_check, "TokenName") as TextEdit
	self.specific_token_name.text = self.default_config.accessToken
	
	self.existing_token_check.toggled.connect(on_existing_token_check)
	self.specific_token_check.toggled.connect(on_specific_token_check)
	self.new_token_check.toggled.connect(on_new_token_check)
	token_panel.about_to_popup.connect(initialize_panel_buttons_from_state)
	self.create_or_use_token_button.pressed.connect(apply_or_create_token)
	self.test_token_button.pressed.connect(on_test_button_pressed)	
	self.request_node = HTTPRequest.new()
	token_panel.add_child(self.request_node)
	self.request_node.request_completed.connect(_on_request_completed)
	self.token_troubleshooting.set_data(self)



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
	self.token_troubleshooting.is_valid_token(self.default_config.accessToken)
	

func apply_or_create_token() -> void:
	var usage := self.current_token_usage
	if (usage == TokenUsageType.Specific):
		self.default_config.accessToken = self.specific_token_name.text

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
