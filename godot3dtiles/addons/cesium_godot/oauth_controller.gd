extends Node

class_name OAuthController

#TODO: Move this to a CPP file instead of GDScript, this is for prototyping only

const PORT := 54351
const BINDING := "127.0.0.1"
const CLIENT_ID = 891
const other_uri := ""
const TOKEN_SCOPE = "assets:list assets:read profile:read tokens:read tokens:write geocode"
const auth_server := "https://ion.cesium.com/oauth"
var redirect_uri := "http://%s:%s/cesium-for-godot/oauth2/callback" % [BINDING, PORT]
const token_request_uri = "https://api.cesium.com/oauth/token"

var redirect_server := TCPServer.new()

var is_connecting : bool = false

var signedIn: bool = false

var config : CesiumGDConfig

func _ready() -> void:
	self.config = ResourceLoader.load("res://addons/cesium_godot/cesium_gd_config.tres")
	self.signedIn = !self.config.accessToken.is_empty()
	self.set_process(false)
	pass

func _process(delta: float) -> void:
	if !redirect_server.is_connection_available():
		return
	var connection : StreamPeerTCP = redirect_server.take_connection()
	var request : String = connection.get_string(connection.get_available_bytes())
	if request:
		self.set_process(false)
		if request.contains("error"):
			var description : String = request.split("description=")[1].split(" ")[0]
			const spaceUriSeparator = "+"
			description = description.replace(spaceUriSeparator, " ");
			send_error_page(connection, description)
			return
		# Black magic string manipulation
		var receivedCode : String = request.split("code=")[1].split(" ")[0]
		var results : Array = await self.request_token(receivedCode)
		if results[0] != OK:
			# Send some error page
			send_error_page(connection, results[1])
			return
		# Send the you can close this window page
		send_success_page(connection)
		self.signedIn = true

func send_success_page(connection: StreamPeerTCP) -> void:
	var pageStr : String = FileAccess.get_file_as_string("res://addons/cesium_godot/success_page.html")
	self.send_page(connection, pageStr)
	self.redirect_server.stop()

func send_error_page(connection: StreamPeerTCP, message: String) -> void:
	# Replace the string for the template
	const errorMarker := "{{ERROR}}"
	var templateFile : FileAccess = FileAccess.open("res://addons/cesium_godot/error_page.html", FileAccess.READ)
	var templateText : String = templateFile.get_as_text()
	templateText = templateText.replace(errorMarker, message)
	send_page(connection, templateText)
	self.redirect_server.stop()

func send_page(connection: StreamPeerTCP, page: String) -> void:
	connection.put_data(("HTTP/1.1 %d\r\n" % HTTPClient.ResponseCode.RESPONSE_OK).to_ascii_buffer())
	connection.put_data(page.to_ascii_buffer())
	

func get_auth_code() -> void:
	set_process(true)
	self.signedIn = false
	self.is_connecting = true
	var err : Error = self.redirect_server.listen(self.PORT, self.BINDING)
	if err != OK:
		print("Error starting up server: %s " % error_string(err))
		return
	var query_params : Array[String] = [
		"client_id=%d" %  CLIENT_ID,
		"redirect_uri=%s" % self.redirect_uri,
		"response_type=code",
		"scope=%s" % TOKEN_SCOPE
	]
	var url : String = self.add_query_params(auth_server, query_params)
	OS.shell_open(url)

func request_token(receivedCode: String) -> Array:
	# Request the oauth token now
	var bodyParams : Array[String] = [
		"grant_type=authorization_code",
		"client_id=" + str(CLIENT_ID),
		"code=%s" % receivedCode,
		"redirect_uri=%s" % self.redirect_uri
	]
	var headers : Array[String] = [
		"Content-Type: application/x-www-form-urlencoded"
	]
	var reqNode := HTTPRequest.new()
	self.add_child(reqNode)
	var err : Error = reqNode.request(self.token_request_uri, headers, HTTPClient.METHOD_POST, "&".join(bodyParams))
	if err != OK:
		return [err, "Error making the Cesium Token Request, failed with underlying: %s" % error_string(err)]
	
	var response := await reqNode.request_completed as Array
	var resultCode : int = response[0]
	var responseCode : int = response[1]
	var body : String = (response[3] as PackedByteArray).get_string_from_utf8()
	var bodyDict := JSON.parse_string(body) as Dictionary
	if (responseCode != HTTPClient.ResponseCode.RESPONSE_OK):
		# Get the message
		var message : String = "Error from server %d" % responseCode
		if bodyDict != null:
			for key in bodyDict.keys():
				message += "; " + bodyDict[key]
		return [ERR_CONNECTION_ERROR, message]
	self.config.accessToken = bodyDict["access_token"]
	cancel_connection()
	return [OK, ""]

func cancel_connection() -> void:
	self.is_connecting = false
	self.redirect_server.stop()

func add_query_params(url: String, params: Array[String]) -> String:
	return url + "?" + "&".join(params)

func sign_out() -> void:
	self.signedIn = false
	self.config.accessToken = ""
