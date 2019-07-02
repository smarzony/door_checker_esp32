void handleRootPath() {
	server.send(200, "text/html", getStatusPage());
}

String getStatusPage()
{
	String page = "<html lang=pl-PL><head><meta http-equiv='refresh' content='10'/>";
	page += "<title>Door: ";
	/*if (reedSwitchState)
		page += "unlocked";
	else
		page += "locked";*/
	page += "</title>";
	page += "<style> body { background-color: #fffff; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style>";
	page += "</head><body><h1>ESP32</h1>";
	//------------------------------------
	page += "<h3>Inputs</h3>";
	page += "Door lock status: ";
	page += "<b>";
	/*if (reedSwitchState)
		page += "Unlocked";
	else
		page += "Locked";*/
	page += "</b><br>";

	page += "Sensor battery percentage: <b>";
	//page += (String)battery_percentage;
	page += "%</b><br>";

	page += "Sensor battery voltage: <b>";
	/*page += (String)(voltage_value / 1000);
	page += ".";
	page += (String)(voltage_value % 1000);
	page += " V</b><br>";*/

	/*page += "Sensor battery adc: <b>";
	page += (String)adc_value;
	page += "</b><br>";*/
	//-------------------------------------
	page += "<h3>Memory</h3>";
	page += "Free mem: <b>";
	page += ESP.getFreeHeap();
	page += " B</b><br>";
	page += "</body></html>";
	return page;
}
