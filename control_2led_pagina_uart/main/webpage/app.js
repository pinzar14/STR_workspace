let miCheckbox = document.getElementById('show_temp');
let msg = document.getElementById('msg');
let indicador = document.getElementById('circulo');
let updateTemperature;
const limitTemp = 30;
const delayUpdateTemp = 5000;

function updateTemp() {
	let request = new XMLHttpRequest();

	request.addEventListener("readystatechange", () => {
		console.log(request, request.readyState);
		if (request.readyState === 4) {
			let value = request.responseText;

			if (value >= limitTemp) {
				indicador.style.background = "red";
			} else {
				indicador.style.background = "green";
			}

			msg.innerText = value;
		}
	});
	request.open('GET', "/showTemp");
	request.responseType = "text";
	request.send();
}

/**
 * Add gobals here
 */
var seconds = null;
var otaTimerVar = null;
var wifiConnectInterval = null;

/**
 * Initialize functions here.
 */
$(document).ready(function () {
	//getUpdateStatus();
	//startDHTSensorInterval();
	$("#connect_wifi").on("click", function () {
		checkCredentials();
	});

	$("#RGBdata_1").on("click", function () {
		console.log("CLICK_1");
		checkValuesRGB('1');
	});
	$("#RGBdata_2").on("click", function () {
		console.log("CLICK_2");
		checkValuesRGB('2');
	});
});

/**
 * Gets file name and size for display on the web page.
 */
function getFileInfo() {
	var x = document.getElementById("selected_file");
	var file = x.files[0];

	document.getElementById("file_info").innerHTML = "<h4>File: " + file.name + "<br>" + "Size: " + file.size + " bytes</h4>";
}

/**
 * Handles the firmware update.
 */
function updateFirmware() {
	// Form Data
	var formData = new FormData();
	var fileSelect = document.getElementById("selected_file");

	if (fileSelect.files && fileSelect.files.length == 1) {
		var file = fileSelect.files[0];
		formData.set("file", file, file.name);
		document.getElementById("ota_update_status").innerHTML = "Uploading " + file.name + ", Firmware Update in Progress...";

		// Http Request
		var request = new XMLHttpRequest();

		request.upload.addEventListener("progress", updateProgress);
		request.open('POST', "/OTAupdate");
		request.responseType = "blob";
		request.send(formData);
	}
	else {
		window.alert('Select A File First')
	}
}

/**
 * Progress on transfers from the server to the client (downloads).
 */
function updateProgress(oEvent) {
	if (oEvent.lengthComputable) {
		getUpdateStatus();
	}
	else {
		window.alert('total size is unknown')
	}
}

/**
 * Posts the firmware udpate status.
 */
function getUpdateStatus() {
	var xhr = new XMLHttpRequest();
	var requestURL = "/OTAstatus";
	xhr.open('POST', requestURL, false);
	xhr.send('ota_update_status');

	if (xhr.readyState == 4 && xhr.status == 200) {
		var response = JSON.parse(xhr.responseText);

		document.getElementById("latest_firmware").innerHTML = response.compile_date + " - " + response.compile_time

		// If flashing was complete it will return a 1, else -1
		// A return of 0 is just for information on the Latest Firmware request
		if (response.ota_update_status == 1) {
			// Set the countdown timer time
			seconds = 10;
			// Start the countdown timer
			otaRebootTimer();
		}
		else if (response.ota_update_status == -1) {
			document.getElementById("ota_update_status").innerHTML = "!!! Upload Error !!!";
		}
	}
}

/**
 * Displays the reboot countdown.
 */
function otaRebootTimer() {
	document.getElementById("ota_update_status").innerHTML = "OTA Firmware Update Complete. This page will close shortly, Rebooting in: " + seconds;

	if (--seconds == 0) {
		clearTimeout(otaTimerVar);
		window.location.reload();
	}
	else {
		otaTimerVar = setTimeout(otaRebootTimer, 1000);
	}
}

/**
 * Gets DHT22 sensor temperature and humidity values for display on the web page.
 */
function getDHTSensorValues() {
	$.getJSON('/dhtSensor.json', function (data) {
		$("#temperature_reading").text(data["temp"]);
		$("#humidity_reading").text(data["humidity"]);
	});
}

/**
 * Sets the interval for getting the updated DHT22 sensor values.
 */
function startDHTSensorInterval() {
	setInterval(getDHTSensorValues, 5000);
}

/**
 * Clears the connection status interval.
 */
function stopWifiConnectStatusInterval() {
	if (wifiConnectInterval != null) {
		clearInterval(wifiConnectInterval);
		wifiConnectInterval = null;
	}
}

/**
 * Gets the WiFi connection status.
 */
function getWifiConnectStatus() {
	var xhr = new XMLHttpRequest();
	var requestURL = "/wifiConnectStatus";
	xhr.open('POST', requestURL, false);
	xhr.send('wifi_connect_status');

	if (xhr.readyState == 4 && xhr.status == 200) {
		var response = JSON.parse(xhr.responseText);

		document.getElementById("wifi_connect_status").innerHTML = "Connecting...";

		if (response.wifi_connect_status == 2) {
			document.getElementById("wifi_connect_status").innerHTML = "<h4 class='rd'>Failed to Connect. Please check your AP credentials and compatibility</h4>";
			stopWifiConnectStatusInterval();
		}
		else if (response.wifi_connect_status == 3) {
			document.getElementById("wifi_connect_status").innerHTML = "<h4 class='gr'>Connection Success!</h4>";
			stopWifiConnectStatusInterval();
		}
	}
}

/**
 * Starts the interval for checking the connection status.
 */
function startWifiConnectStatusInterval() {
	wifiConnectInterval = setInterval(getWifiConnectStatus, 2800);
}

/**
 * Connect WiFi function called using the SSID and password entered into the text fields.
 */
function connectWifi() {
	// Get the SSID and password
	/*selectedSSID = $("#connect_ssid").val();
	pwd = $("#connect_pass").val();
	
	$.ajax({
		url: '/wifiConnect.json',
		dataType: 'json',
		method: 'POST',
		cache: false,
		headers: {'my-connect-ssid': selectedSSID, 'my-connect-pwd': pwd},
		data: {'timestamp': Date.now()}
	});
	*/
	selectedSSID = $("#connect_ssid").val();
	pwd = $("#connect_pass").val();

	// Create an object to hold the data to be sent in the request body
	var requestData = {
		'selectedSSID': selectedSSID,
		'pwd': pwd,
		'timestamp': Date.now()
	};

	// Serialize the data object to JSON
	var requestDataJSON = JSON.stringify(requestData);

	$.ajax({
		url: '/wifiConnect.json',
		dataType: 'json',
		method: 'POST',
		cache: false,
		data: requestDataJSON, // Send the JSON data in the request body
		contentType: 'application/json', // Set the content type to JSON
		success: function (response) {
			// Handle the success response from the server
			console.log(response);
		},
		error: function (xhr, status, error) {
			// Handle errors
			console.error(xhr.responseText);
		}
	});


	//startWifiConnectStatusInterval();
}

/**
 * Checks credentials on connect_wifi button click.
 */
function checkCredentials() {
	errorList = "";
	credsOk = true;

	selectedSSID = $("#connect_ssid").val();
	pwd = $("#connect_pass").val();

	if (selectedSSID == "") {
		errorList += "<h4 class='rd'>SSID cannot be empty!</h4>";
		credsOk = false;
	}
	if (pwd == "") {
		errorList += "<h4 class='rd'>Password cannot be empty!</h4>";
		credsOk = false;
	}

	if (credsOk == false) {
		$("#wifi_connect_credentials_errors").html(errorList);
	}
	else {
		$("#wifi_connect_credentials_errors").html("");
		connectWifi();
	}
}

/**
 * Shows the WiFi password if the box is checked.
 */
function showPassword() {
	var x = document.getElementById("connect_pass");
	if (x.type === "password") {
		x.type = "text";
	}
	else {
		x.type = "password";
	}
}


function setValuesRGB(row, value_R, value_G, value_B) {

	// Create an object to hold the data to be sent in the request body
	var requestData_RGB = {
		'row': row,
		'value_R': value_R,
		'value_G': value_G,
		'value_B': value_B,
		'timestamp': Date.now()
	};

	// Serialize the data object to JSON
	var requestDataRGBJSON = JSON.stringify(requestData_RGB);

	console.log(requestDataRGBJSON);

	$.ajax({
		url: '/setRGB.json',
		dataType: 'json',
		method: 'POST',
		cache: false,
		data: requestDataRGBJSON, // Send the JSON data in the request body
		contentType: 'application/json', // Set the content type to JSON
		success: function (response) {
			// Handle the success response from the server
			console.log(response);
		},
		error: function (xhr, status, error) {
			// Handle errors
			console.error(xhr.responseText);
		}
	});

}

function checkValuesRGB(row) {
	errorValue = "";
	valuesOK = true;
	if (row == 1) {
		console.log("1_OK");
		value_R = $("#LEDR_1").val();
		value_G = $("#LEDG_1").val();
		value_B = $("#LEDB_1").val();
	}
	if (row == 2) {
		console.log("2_OK");
		value_R = $("#LEDR_2").val();
		value_G = $("#LEDG_2").val();
		value_B = $("#LEDB_2").val();
	}

	if (value_R == "") {
		errorValue += "<h4 class='rd'>RED LED cannot be empty!</h4>";
		valuesOK = false;
	}
	if (value_G == "") {
		errorValue += "<h4 class='rd'>GREEN LED cannot be empty!</h4>";
		valuesOK = false;
	}
	if (value_B == "") {
		errorValue += "<h4 class='rd'>BLUE LED cannot be empty!</h4>";
		valuesOK = false;
	}

	if (value_R < 0 || value_R > 100 || value_G < 0 || value_G > 100 || value_B < 0 || value_B > 100) {
		errorValue += "<h4 class='rd'>Only values between 0-100</h4>";
		valuesOK = false;
	}

	if (valuesOK == false) {
		$("#rgb_Values_errors").html(errorValue);
	}
	else {
		$("#rgb_Values_errors").html("");
		setValuesRGB(row, value_R, value_G, value_B);
	}

}





























