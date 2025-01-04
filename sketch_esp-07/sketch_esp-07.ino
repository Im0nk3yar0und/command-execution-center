#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <FS.h>  // For using SPIFFS (SPI Flash File System)


/*
 * *******************************************************
 * System Information Printing for ESP8266
 * *******************************************************
 * Uncomment the following line to enable printing of 
 * detailed system information about the ESP8266 module.
 * 
 * This includes useful details such as:
 * - CPU frequency
 * - Flash size
 * - Heap size
 * - Wi-Fi status (IP address, MAC address, etc.)
 * 
 * This is useful for debugging or understanding the system's
 * resources and current status.
 * *******************************************************
 */
 // #define PRINT_INFO




/*
 * *******************************************************
 * Hardcoded Login Credentials for ESP8266 Web Server
 * *******************************************************
 * This section of the code handles user authentication
 * for accessing protected routes in the web server. 
 * 
 * It uses hardcoded credentials (username and password)
 * for simplicity. When a user submits the login form,
 * the entered credentials are checked against the hardcoded
 * values. If the credentials match, the user is logged in.
 * Otherwise, access is denied.
 *
 * The login state is managed with the global variable
 * 'isLoggedIn'. This variable is used to track whether
 * the user is authenticated or not..
 * 
 * Credentials:
 * - Username: "admin"
 * - Password: "admin123"
 * 
 * This code provides basic login functionality suitable
 * for simple applications or demonstrations.
 * *******************************************************
 */
bool isLoggedIn = false;  // Global flag to track login status
const String validUsername = "admin";
const String validPassword = "admin";




/*
 * *******************************************************
 * Intranet Mode for ESP8266
 * *******************************************************
 * Define USE_INTRANET to connect the ESP8266 to your home 
 * intranet for easier debugging during development.
 * 
 * When this mode is enabled, the ESP8266 will connect to 
 * your home Wi-Fi network (intranet). This allows you to 
 * access the ESP8266 web server from your browser without 
 * needing to reconnect to the network each time, which 
 * simplifies testing and debugging.
  * *******************************************************
 */
// #define USE_INTRANET




/*
 * *******************************************************
 * Enable File Removal Functionality
 * *******************************************************
 * Define REMOVE_FILES to enable the file removal functionality 
 * in your project.
 * 
 * When this option is enabled, your ESP8266 will be able to 
 * remove files stored in its file system (SPIFFS). 
 * This can be particularly useful in situations where you need 
 * to delete or clear files programmatically, such as cleaning 
 * up temporary files, clearing configurations, or removing 
 * outdated data from the device.
 * *******************************************************
 */
// #define REMOVE_FILES



/*
 * *******************************************************
 * Intranet Wi-Fi Credentials
 * *******************************************************
 * Replace the values below with your home or intranet Wi-Fi credentials. 
 * The ESP8266 will use these credentials to connect to your local Wi-Fi 
 * network for debugging, accessing internal resources, or enabling 
 * communication with other devices on your network.
 * *******************************************************
 */
// #define LOCAL_SSID "MyWiFiNetwork"  // Generic example SSID
// #define LOCAL_PASS "password1234"   // Generic example password





/*
 * *******************************************************
 * Access Point (AP) Configuration for the ESP8266
 * *******************************************************
 * When the ESP8266 is in AP mode, it creates its own Wi-Fi network.
 * Devices can connect to this network to communicate with the ESP8266, 
 * especially useful in cases where there is no external Wi-Fi network 
 * available or for setting up a configuration mode.
 * 
 * The SSID (AP_SSID) is the name of the Wi-Fi network created by the ESP8266, 
 * and AP_PASS is the password required to connect to it. 
 * 
 * This setup allows the ESP8266 to act as an access point (e.g., for 
 * configuration or direct device communication).
 * *******************************************************
 */
String AP_SSID = "ESP8266-Access-Point";
String AP_PASS = "123456789";
bool ssidHide = false;






/*
  *******************************************************
  Communication Setup between Arduino and ESP using SoftwareSerial
  *******************************************************
  This part of the code sets up a serial communication link between an Arduino and an ESP device.
  We are using the SoftwareSerial library to create a virtual serial port on other pins of the ESP device, 
  since the hardware serial ports are occupied by the serial monitor.
  
  - `rxPin`: This is the pin on the ESP device that will receive data from the Arduino.
    The default is pin 4, but you can modify this to another available pin.
  
  - `txPin`: This is the pin on the ESP device that will transmit data to the Arduino.
    The default is pin 5, but it can also be changed as needed.
  
  To change the communication pins, simply modify the `rxPin` and `txPin` values below.
  *******************************************************
*/
int rxPin = 4;  // RX pin for receiving data from Arduino
int txPin = 5;  // TX pin for sending data to Arduino







/*
  *******************************************************
  IP Configuration for ESP8266
  *******************************************************
  This part of the code defines various IP settings for the ESP8266.
  
  - `Actual_IP`: Stores the IP address assigned to the ESP8266 when it connects to the home intranet (used in debug mode).
  
  - `PageIP`: The default IP address for the ESP8266 when it is acting as an Access Point (AP).
  
  - `gateway`: The gateway IP address for the ESP8266 when it's set up as an Access Point.
  
  - `subnet`: The subnet mask used for the ESP8266 when it's set up as an Access Point.

  *******************************************************
*/
IPAddress Actual_IP;
IPAddress PageIP(192, 168, 1, 1);        // Default IP address of the AP
IPAddress gateway(192, 168, 1, 1);       // Gateway for the AP
IPAddress subnet(255, 255, 255, 0);      // Subnet mask for the AP








/*
  *******************************************************
  Web Server and DNS Redirection Setup
  *******************************************************
  This part of the code sets up a web server on the ESP8266 and configures DNS redirection.
  
  - `server`: An instance of the ESP8266 WebServer that listens on port 80 (HTTP default port) to handle incoming HTTP requests.
  
  - `dnsServer`: An instance of the DNS server for handling custom DNS redirection. The ESP8266 can act as a DNS server, redirecting specific domain requests to its own IP address.
  
  - `homeIP`: The IP address that the ESP8266 will respond with when a certain domain (`evilcorp.io` in this case) is queried. This address can be the IP address of the ESP8266 in AP mode or any custom address.
  
  - `domain`: The domain name that will be redirected to the ESP8266's IP address (e.g., `evilcorp.io`). When a device queries this domain, the ESP8266 will intercept the request and respond with its own IP address.
  *******************************************************
*/

// Create an instance of the ESP8266 WebServer
ESP8266WebServer server(80); // HTTP server running on port 80

// DNS server instance
DNSServer dnsServer;

// IP address to return for a specific domain
IPAddress homeIP(192, 168, 1, 1);  // Redirected IP address for the domain

// The domain name to redirect to the ESP8266's IP address
const char* domain = "evilcorp.io"; 






/*
  *******************************************************
  SoftwareSerial Setup for ESP8266 and Arduino Communication
  *******************************************************
  This section of the code initializes a `SoftwareSerial` object that enables communication 
  between the ESP8266 and an Arduino using specific RX and TX pins. The `SoftwareSerial` 
  library allows communication on any digital pins, but in this case, the ESP8266 will 
  use pins 4 (RX) and 5 (TX).

  - `rxPin`: The pin on the ESP8266 that receives data from the Arduino.
  - `txPin`: The pin on the ESP8266 that sends data to the Arduino.

  RX = rxPin, TX = txPin

  *******************************************************
*/
SoftwareSerial mySerial(rxPin, txPin); 





/*
  The `setup()` function is called once when the device starts up or is reset.
  It is used for initializing the system, setting up configurations, and establishing connections.
*/

void setup() {
  // Start serial communication for debugging purposes (baud rate 115200)
  Serial.begin(115200);

  // Initialize SoftwareSerial communication with Arduino (baud rate 9600)
  mySerial.begin(9600);

  // Wait for serial port to connect (only needed for native USB ports)
  while (!Serial) {
    ; // Wait for serial port to connect
  }

  // A small delay before starting the setup process
  delay(500);

  // Display a banner and perform a system check
  banner();
  systemCheck();

  // Check if REMOVE_FILES is defined (used for removing specific files)
  #ifdef REMOVE_FILES
      removeFiles(); // Call the function to remove files if defined
  #endif

  // If USE_INTRANET is defined, connect to a specific Wi-Fi network (home intranet)
  #ifdef USE_INTRANET
    WiFi.begin(LOCAL_SSID, LOCAL_PASS);  // Connect to Wi-Fi using predefined credentials
    
    // Wait until the ESP8266 successfully connects to Wi-Fi
    Serial.print("Connecting to WiFi: ");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);  // Wait half a second before trying again
      Serial.print("."); // Print a dot for each connection attempt
    }

    // Once connected, print the assigned IP address
    Serial.print("\nIP address: ");
    Serial.println(WiFi.localIP());

    // Store the current IP address for further use
    Actual_IP = WiFi.localIP();
  
  #endif

  // If USE_INTRANET is not defined, configure the ESP as an access point
  #ifndef USE_INTRANET
    startAP();  // Start the ESP8266 in Access Point (AP) mode

    // Start the DNS server on port 53 for redirecting requests to the ESP's IP
    dnsServer.start(53, domain, homeIP); 
  #endif

  // Start the HTTP server to listen for incoming HTTP requests on port 80
  server.begin();

  // Print information indicating that the system is ready
  Serial.println("\n\nSystem ready for further operations.");
  Serial.println("**************************************");

  // Initialize the SPIFFS (SPI Flash File System) for file storage
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS mount failed");
    return;   // Stop further execution if SPIFFS fails to mount
  }






/*
  Define Routes for the HTTP Server:
  - The server.on() function is used to define different routes (URLs) and associate them with specific handlers (functions).
  - Each route corresponds to a specific page or action within the web application.

  The routes are as follows:
  1. "/"                  -> The login page (HTTP GET request).
  2. "/favicon-48x48.png" -> The website's icon (favicon).
  3. "/E-Corp.jpg"        -> A request for the wallpaper image.
  4. "/login"             -> A POST request for submitting the login form.
  
  Protected routes (requiring login):
  5. "/home"               -> The home page, accessible only after a successful login.
  6. "/about"              -> The about page, accessible only after login.
  7. "/wifi_settings"      -> The Wi-Fi settings page, accessible only after login.
  8. "/save_wifi_settings" -> A POST request for saving Wi-Fi settings, accessible only after login.
  9. "/send_command"       -> A POST request for running a command on the device, accessible only after login.
  10. "/logout"            -> The logout functionality, accessible only after login.

  If the user is not logged in, access to these protected routes will be blocked, and the user will be redirected accordingly.
*/

  // Route for the root URL (login page)
  server.on("/", HTTP_GET, handleLogin);

  // Route for the favicon (used to display the website's icon)
  server.on("/favicon-48x48.png", HTTP_GET, handleFavicon);
  
  // Route for handling the wallpaper image request
  server.on("/E-Corp.jpg", HTTP_GET, handleWallpaper);

  // Route for handling the login form submission (POST request)
  server.on("/login", HTTP_POST, handleLoginPost);

  // Handle non-existing routes with a 404 response
  server.onNotFound(handleNotFound);





  // Route for the home page (protected, needs login)
  server.on("/home", HTTP_GET, []() {
    if (!checkLogin()) return;          // Stop further processing if not logged in
    handleHome();                       // Serve the home page if logged in
  });


  // Route for the about page (protected, needs login)
  server.on("/about", HTTP_GET, []() {
    if (!checkLogin()) return;          // Stop further processing if not logged in
    handleAbout();                      // Serve the about page if logged in
  });


  // Route for the Wi-Fi settings page (protected, needs login)
  server.on("/wifi_settings", HTTP_GET, []() {
    if (!checkLogin()) return;         // Stop further processing if not logged in
    handleWiFi();                      // Serve the Wi-Fi settings page if logged in
  });


  // Route for saving Wi-Fi settings (protected, needs login)
  server.on("/save_wifi_settings", HTTP_POST, []() {
    if (!checkLogin()) return;         // Stop further processing if not logged in
    handleSaveSettings();              // Handle saving Wi-Fi settings if logged in
  });


  // Route for running a command (protected, needs login)
  server.on("/send_command", HTTP_POST, []() {
    if (!checkLogin()) return;         // Stop further processing if not logged in
    handleRunCommand();                // Handle running the command if logged in
  });


  // Route for logging out (protected, needs login)
  server.on("/logout", HTTP_GET, []() {
    if (!checkLogin()) return;         // Stop further processing if not logged in
    handleLogout();                    // Handle logout functionality if logged in
  });





  /*
    If PRINT_INFO is defined, display additional information:
    - This section will print system information, including CPU details, flash size, and Wi-Fi status, 
      if the `PRINT_INFO` macro is defined during compilation.
    - This is useful for debugging or understanding the current state of the system.
  */
  #ifdef PRINT_INFO
    printInformation();       // Print system information if defined
  #endif
}









/*
  The `loop()` function runs continuously in a cycle after `setup()` completes. It is used for handling 
  repeated tasks such as monitoring user input, network requests, or device state updates.

  The `loop()` function typically includes:
  1. Handling requests to the HTTP server.
  2. Checking for network activity or other event-driven tasks.
  3. Periodic or continuous tasks like DNS handling, server management, or sensor readings.

*/
void loop() {
  server.handleClient();              // Listen for client requests

  #ifndef USE_INTRANET                 // Use DNS server to process incoming DNS queries
      dnsServer.processNextRequest();  // Process DNS requests
  #endif
 
}





/*
 * Function to handle the 'Home' page request 
 * ----------------------------------------------------------------------------------
 * This function is called when the user requests the '/home' page on the web server.
 * It attempts to serve the 'home.html' page stored in the SPIFFS (SPI Flash File System) of the ESP8266. 
 * If the file is found, it streams the content to the client with the MIME type 'text/html'.
 * If the file is not found, it sends a 404 error response with a 'File not found' message.
*/
void handleHome() {
  // Open the home.html file from SPIFFS
  File file = SPIFFS.open("/home.html", "r");
  
  if (file) {

    // Stream the file to the client
    server.streamFile(file, "text/html");

    // Close the file after streaming
    file.close();                              
  } else {

    // If file not found, send 404 error
    server.send(404, "text/plain", "File not found");
  }
}




/*
 * Handles the login page request by opening the 'login.html' file from SPIFFS.
 * If the file is found, it is streamed to the client with the MIME type 'text/html'.
 * If the file is not found, a 404 error is returned with a plain text message.
 */
void handleLogin() {

  // Open the login.html file from SPIFFS
  File file = SPIFFS.open("/login.html", "r");
  
  if (file) {

    // Stream the file to the client
    server.streamFile(file, "text/html");

    // Close the file after streaming
    file.close();
  } else {
    
    // If file not found, send 404 error
    server.send(404, "text/plain", "File not found");
  }
}




/*
 * Handles the About page request by opening the 'about.html' file from SPIFFS.
 * If the file is found, it is streamed to the client with the MIME type 'text/html'.
 * If the file is not found, a 404 error is returned with a plain text message.
 */
 void handleAbout() {
  File file = SPIFFS.open("/about.html", "r");  // Open the about.html file from SPIFFS
  if (file) {

    // Stream the file to the client
    server.streamFile(file, "text/html");

    // Close the file after streaming
    file.close();
    
  } else {

    // If file not found, send 404 error
    server.send(404, "text/plain", "File not found");
  }
}





/*
 * Handles the Wi-Fi settings page request by opening the 'wifi_settings.html' file from SPIFFS.
 * If the file exists, it is streamed to the client with the appropriate HTML MIME type.
 * If the file is not found, a 404 error is sent with a plain text response.
 */
void handleWiFi() {
  // Open the wifi_settings.html file from SPIFFS
  File file = SPIFFS.open("/wifi_settings.html", "r");
  
  if (file) {
    // Stream the file to the client
    server.streamFile(file, "text/html");
    
    // Close the file after streaming
    file.close();  
  } else {

    // If file not found, send 404 error
    server.send(404, "text/plain", "File not found");  
  }
}




/*
 * Handles the favicon request by opening the 'favicon-48x48.png' file from SPIFFS.
 * If the file is found, it is streamed to the client with the PNG MIME type.
 * If the file is not found, a 404 error is sent with a plain text response.
 */
 void handleFavicon() {
  // Open the favicon image from SPIFFS
  File file = SPIFFS.open("/favicon-48x48.png", "r");
  
  if (file) {
    
    // Stream the file with the correct MIME type for PNG
    server.streamFile(file, "image/png");

    // Close the file after streaming
    file.close();
  } else {
    
    // If the file isn't found, send a 404 error
    server.send(404, "text/plain", "File not found");
  }
}





/*
 * Handles the wallpaper request by opening the 'E-Corp.jpg' file from SPIFFS.
 * If the file is found, it is streamed to the client with the JPEG MIME type.
 * If the file is not found, a 404 error is sent with a plain text response.
 */
void handleWallpaper() {
  // Open the favicon image from SPIFFS
  File file = SPIFFS.open("/E-Corp.jpg", "r");
  
  if (file) {
    
    // Stream the file with the correct MIME type for JPEG
    server.streamFile(file, "image/jpeg");

    // Close the file after streaming
    file.close();
  } else {
    
    // If the file isn't found, send a 404 error
    server.send(404, "text/plain", "File not found");
  }
}





/*
 * Function to list files stored in SPIFFS along with their sizes.
 * This is primarily used for debugging purposes to verify file availability and sizes.
 * The function takes a directory path as input and prints each file's name and size.
 */
void listFiles(const char *dir) {
  Serial.println("Listing files in SPIFFS:");

  // Open the specified directory in SPIFFS
  Dir d = SPIFFS.openDir(dir);

  // Iterate through all files in the directory
  while (d.next()) {
    String fileName = d.fileName();  // Retrieve the file name
    size_t fileSize = d.fileSize();  // Retrieve the file size

    // Print the file name and size to the serial monitor
    Serial.print("File: ");
    Serial.print(fileName);
    Serial.print(" | Size: ");
    Serial.println(fileSize);
  }
}




/*
 * Function to start the Access Point (AP) mode on the ESP8266.
 * This sets up a wireless network with a specified SSID and password, 
 * configures its network parameters, and displays its IP address.
 */
void startAP() {
    // Start the Access Point with the defined SSID and password
    WiFi.softAP(AP_SSID, AP_PASS);
    delay(100);  // Brief delay to ensure the AP starts properly

    // Configure the Access Point's network settings: IP, gateway, and subnet
    WiFi.softAPConfig(PageIP, gateway, subnet);
    delay(100);  // Brief delay to ensure network settings are applied

    // Notify that the Access Point is active
    Serial.println("\nAccess Point Started");

    // Retrieve and display the IP address assigned to the Access Point
    Actual_IP = WiFi.softAPIP();
    Serial.print("IP address: ");
    Serial.println(Actual_IP);
}





/*
 * Function to remove specific files from SPIFFS.
 * This function iterates through a predefined list of files, checks if each exists,
 * and removes it if found. It also provides a user-friendly status update via the serial monitor.
 */
void removeFiles() {

    Serial.println("*******************************");
    Serial.println("*                             *");
    Serial.println("*       Removinf files        *");
    Serial.println("*        Please wait...       *");
    Serial.println("*                             *");
    Serial.println("*******************************");
  
    Serial.print("Initializing");
    for (int i = 0; i < 10; i++) {
      delay(500);
      Serial.print(".");
    }
  
    Serial.print("\n");

  
    // List of files to remove
    const char* filesToRemove[] = {
        "/favicon-48x48.png",
        "/home.html",
        "/wifi_settings.html",
        "/about.html"
    };
    
    // Calculate the number of files in the list
    const int numFiles = sizeof(filesToRemove) / sizeof(filesToRemove[0]);

    // Iterate through the list of files and attempt to remove each
    for (int i = 0; i < numFiles; i++) {
        if (SPIFFS.exists(filesToRemove[i])) {
          
            // Remove the file and confirm success
            SPIFFS.remove(filesToRemove[i]);
            Serial.printf("File %s removed successfully.\n", filesToRemove[i]);
        } else {
          
            // Notify if the file does not exist
            Serial.printf("File %s does not exist.\n", filesToRemove[i]);
        }
    }

    // Notify that the file removal process is complete
    Serial.println("Removing files done!\n\n");
}







/*
 * Function to handle saving Wi-Fi settings received in JSON format.
 * This function extracts SSID, password, and optional SSID hiding preference 
 * from the incoming data, validates the inputs, and updates the ESP's Access Point (AP) accordingly.
 */
void handleSaveSettings() {
    // Step 1: Read and parse the incoming JSON data
    String json = server.arg("plain");  // Retrieve raw JSON data from the client
    DynamicJsonDocument doc(1024);     // Create a JSON document to store parsed data
    DeserializationError error = deserializeJson(doc, json);  // Attempt to parse JSON

    if (error) {
        // If JSON parsing fails, send an error response and exit
        Serial.println("Failed to parse JSON");
        server.send(400, "application/json", "{\"success\": false, \"error\": \"Failed to parse JSON\"}");
        return;
    }

    // Step 2: Extract data from the JSON object
    String newSSID = doc["ssid"].as<String>();
    String newPass = doc["password"].as<String>();
    ssidHide = doc["ssidHide"].as<bool>();

    // Validate the password length
    if (newPass.length() < 8) {
        server.send(400, "application/json", "{\"success\": false, \"error\": \"Password must be at least 8 characters\"}");
        Serial.println("Password too short!");
        return;
    }

    // Step 3: Update the Access Point settings
    Serial.println("Access Point updated successfully!");
    server.send(200, "application/json", "{\"success\": true}");

    delay(500);

    AP_SSID = newSSID;  // Update the SSID
    AP_PASS = newPass;  // Update the password

    // Log the new settings for debugging
    Serial.println(String("Updating AP with SSID: ") + AP_SSID);
    Serial.println(String("Password: ") + AP_PASS);
    Serial.println("Hide SSID: " + String(ssidHide ? "Enabled" : "Disabled"));

    // Step 4: Disconnect the current Access Point
    if (!WiFi.softAPdisconnect(true)) {
        Serial.println("Failed to disconnect the current Access Point!");
        server.send(500, "application/json", "{\"success\": false, \"error\": \"Failed to disconnect AP\"}");
        return;
    }

    // Optional: Wait for the AP to fully disconnect
    delay(1000);

    // Step 5: Start a new Access Point with the updated settings
    if (!WiFi.softAP(AP_SSID.c_str(), AP_PASS.c_str(), 6, ssidHide)) {
        Serial.println("Failed to start the new Access Point!");
        server.send(500, "application/json", "{\"success\": false, \"error\": \"Failed to start new AP\"}");
        return;
    }
}







/*
 * Function to print ESP8266 system and Wi-Fi information for debugging purposes.
 * This function provides an overview of system performance, flash memory usage, and current Wi-Fi connection details.
 */void printInformation() {

    Serial.println("\n\n");
    Serial.println("ESP Information");
    Serial.println("------------------------------------------------");

    // Print list of files and their sizes in SPIFFS
    listFiles("/");

    Serial.println("\n\n");
    Serial.println("ESP8266 System Information:");
  
    // CPU Frequency (in MHz)
    Serial.print("CPU Frequency: ");
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println(" MHz");

    // Flash Chip Speed (in Hz)
    Serial.print("Flash Chip Speed: ");
    Serial.print(ESP.getFlashChipSpeed());
    Serial.println(" Hz");

    // Uptime (time since last restart)
    Serial.print("Uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" seconds");

    Serial.println("\n\n");

    // Flash Memory Size (in bytes)
    Serial.print("Flash Memory Size: ");
    Serial.print(ESP.getFlashChipSize());
    Serial.println(" bytes");

    // Free Heap (available RAM)
    Serial.print("Free Heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");

    // Used sketch size (in bytes)
    Serial.print("Sketch Size (used): ");
    Serial.print(ESP.getSketchSize());
    Serial.println(" bytes");

    // Maximum sketch size (in bytes)
    Serial.print("Maximum Sketch Size: ");
    Serial.print(ESP.getFreeSketchSpace() + ESP.getSketchSize());
    Serial.println(" bytes");

    Serial.println("\n\n");

    // Print Wi-Fi information
    Serial.println("Wi-Fi Information:");

    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    Serial.print("RSSI (Signal Strength): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());

    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());

    Serial.println("------------------------------------------------");
}









/*
 * Function to display a boot banner and initialization progress on the serial monitor.
 * Provides a visual indication of system startup, useful for debugging and user feedback.
 */
void banner() {
  delay(500);
  Serial.println("\n\n");
  Serial.println("*******************************");
  Serial.println("*                             *");
  Serial.println("*   ESP-07 System Booting     *");
  Serial.println("*       Please wait...        *");
  Serial.println("*                             *");
  Serial.println("*******************************");

  Serial.print("Initializing");
  for (int i = 0; i < 10; i++) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("\n");
}








/*
 * Function to perform a simulated system check and print board details.
 * This provides valuable information about the ESP8266 board and its memory for debugging.
 */
void systemCheck() {
  // Simulate system check steps with a delay for effect
  Serial.println("\nChecking components...\n");
  delay(500);

  // Display board details header
  Serial.println("*******************************");
  Serial.println("*        Board Details:       *");
  Serial.println("*******************************");

  // Check if the board is ESP8266 and display relevant information
  #if defined(ESP8266)
    Serial.println("Board: ESP8266");

    // Chip ID
    Serial.println("Chip ID: " + String(ESP.getChipId()));

    // Flash size in KB
    Serial.println("Flash Size: " + String(ESP.getFlashChipSize() / 1024) + " KB");

    // SDK version
    Serial.println("SDK Version: " + String(ESP.getSdkVersion()));

    // SPIFFS file system information
    FSInfo fs_info;
    if (SPIFFS.info(fs_info)) {
      Serial.printf("Total space: %d bytes\n", fs_info.totalBytes);
      Serial.printf("Used space: %d bytes\n", fs_info.usedBytes);
    } else {
      Serial.println("Error: Unable to access SPIFFS information.");
    }

  #else
    // If the board is not ESP8266
    Serial.println("Board: Unknown");
  #endif

  Serial.println("\n");
}





/*
 * Function to handle the run command by processing the incoming JSON data
 * from the client and sending it to the Arduino via SoftwareSerial.
 *
 * This function:
 * 1. Reads the incoming JSON data from the client.
 * 2. Parses the JSON data to ensure all required fields ("st", "ip", "p") are present and not null.
 * 3. If any required fields are missing or null, it responds with a 400 error and lists the missing keys.
 * 4. If all required fields are valid, it extracts the values for shell type, IP address, and port, and sends them to the Arduino.
 * 5. It waits for a response from the Arduino (ACK/NACK) for up to 5 seconds.
 * 6. Based on the Arduino response, it sends a success or failure message back to the client.
 * 
 * The function also logs important data for debugging, including the received command and the Arduino's response.
 */

// Function to handle the run command
void handleRunCommand() {
  
  // Read the incoming JSON data from the client
  String receivedCommand = server.arg("plain");

  // Create a dynamic JSON document for deserialization
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, receivedCommand);

  // Check for JSON parsing errors
  if (error) {
      Serial.print("JSON parsing error: ");
      Serial.println(error.c_str());
      server.send(400, "application/json", "{\"success\": false, \"error\": \"Failed to parse JSON\"}");
      return;
  }

  // Validate the presence and non-nullity of required keys: "st", "ip", and "p"
  if (!doc.containsKey("st") || doc["st"].isNull() || 
      !doc.containsKey("ip") || doc["ip"].isNull() || 
      !doc.containsKey("p") || doc["p"].isNull()) {
      
      Serial.println("Missing or null keys in JSON.");
      Serial.println(receivedCommand);

      // Construct a JSON error response indicating the missing keys
      String errorResponse = "{\"success\": false, \"error\": \"Missing or null keys: ";
      if (!doc.containsKey("st") || doc["st"].isNull()) errorResponse += "st ";
      if (!doc.containsKey("ip") || doc["ip"].isNull()) errorResponse += "ip ";
      if (!doc.containsKey("p") || doc["p"].isNull()) errorResponse += "p ";
      errorResponse += "\"}";

      // Send the error response to the client
      server.send(400, "application/json", errorResponse);
      return;
  }

  // Extract the values from the JSON object
  String shellType = doc["st"].as<String>();
  String ipAddress = doc["ip"].as<String>();
  String port = doc["p"].as<String>();

  // Log the received data for debugging
  Serial.println("\n\n");
  Serial.println("Data Received");
  Serial.println("************************");
  Serial.println(String("Command:    ") + shellType);
  Serial.println(String("IP Address: ") + ipAddress);
  Serial.println(String("Port:       ") + port);
  Serial.println(String("Raw data:   ") + receivedCommand);

  // Send the JSON command to the Arduino via SoftwareSerial
  mySerial.print(receivedCommand);
  delay(1000); // Wait 1 second to ensure the Arduino processes the command

  // Wait for an ACK/NACK response from the Arduino
  unsigned long startMillis = millis();
  bool responseReceived = false;

  while (millis() - startMillis < 5000) { // Wait for up to 5 seconds
    if (mySerial.available()) {
      String ackResponse = mySerial.readString();
      Serial.println("Received response from Arduino: " + ackResponse);

      // Parse the JSON response from the Arduino
      DynamicJsonDocument responseDoc(500);
      DeserializationError ackError = deserializeJson(responseDoc, ackResponse);

      if (!ackError) {
        const char* status = responseDoc["status"];
        
        // Check if the response status is "ACK"
        if (strcmp(status, "ACK") == 0) {
          
          Serial.println("ACK: Arduino successfully received the command!");
          responseReceived = true;

          // Exit the loop after receiving the ACK
          break;
          
        } else if (strcmp(status, "NACK") == 0) {
          
          Serial.println("NACK: Arduino failed to process the command.");
          responseReceived = true;
          break;
        }
      } else {
        Serial.println("Failed to parse response from Arduino.");
      }
    }
    delay(100); // Small delay to prevent flooding the serial buffer
  }

  // Check if a response was received within the timeout period
  if (responseReceived) {
    
    // Send acknowledgment back to the client indicating success
    server.send(200, "application/json", "{\"success\": true, \"message\": \"Command sent successfully\"}");
    Serial.println("Success: Command sent and ACK received from Arduino.");
    
  } else if (!responseReceived) {
    
    // Inform the client that the command failed to send successfully
    server.send(200, "application/json", "{\"success\": false, \"message\": \"Failed to send the command\"}");
    Serial.println("Error: No response from Arduino within the timeout period.");
    
  } else {
    
    // Handle any unexpected errors (this case should not typically occur)
    server.send(500, "application/json", "{\"success\": false, \"message\": \"Unknown error occurred\"}");
    Serial.println("Error: An unknown error occurred during command handling.");
  }

}


/*
 * Function to check the login status of the user.
 * This function ensures that only authenticated users can access certain resources.
 *
 * If the user is not logged in, it redirects them to the login page ("/"),
 * by sending an HTTP 303 status code 
 *
 * If the user is logged in, it allows access to the requested resource and returns true.
 * 
 */
bool checkLogin() {
  if (!isLoggedIn) {

    // Redirect to login page
    server.sendHeader("Location", "/");

    // Send HTTP status code 303 
    server.send(303);
    return false;
  }
  return true;
}





/*
 * Function to handle logout and reset login status.
 * This function logs the user out by resetting the login status to false.
 * 
 * After logging the user out, it redirects the user to the login page ("/")
 * by sending an HTTP 303 status code
 * 
 */
void handleLogout() {

  // Reset login status
  isLoggedIn = false;

  // Redirect to login page
  server.sendHeader("Location", "/");

  // Send HTTP status code 303 
  server.send(303);
}




/*
 * Function to handle the root route and serve the login page (login.html).
 * This function is called when a user accesses the root URL of the web server.
 * 
 * It attempts to open the login page (login.html) from the SPIFFS file system.
 * If the file is successfully opened, it streams the content to the client with
 * the correct MIME type (text/html) so that the browser can render the login page.
 * 
 * If the file cannot be opened (e.g., if the file does not exist or there is an error),
 * it sends a 500 Internal Server Error with a message indicating the failure.
 */
void handleLoginPage() {
  File file = SPIFFS.open("/login.html", "r");
  if (!file) {
    server.send(500, "text/plain", "Failed to open login page.");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}





/*
 * Function to handle 404 (Not Found) error
 * This function is called when a user accesses the root URL of the web server.
 * 
 * It attempts to open the login page (404.html) from the SPIFFS file system.
 * If the file is successfully opened, it streams the content to the client with
 * the correct MIME type (text/html) so that the browser can render the login page.
 * 
 */
void handleNotFound() {
  // Open the 404.html file
  File file = SPIFFS.open("/404.html", "r");  
  
  if (!file) {
    server.send(404, "text/plain", "404 - Not Found (Unable to open 404.html)");
    return;
  }

  // Read the content of the file
  String content = file.readString();

  // Send the content as the response
  server.send(404, "text/html", content);

  // Close the file
  file.close();
  }





/*
 * This function is triggered when a user submits the login form via a POST request.
 *
 * It retrieves the submitted username and password from the request.
 * If the provided credentials match the valid username and password,
 * the login status is set to true, and the user is redirected to the home page
 * using an HTTP 303 status code
 * 
 * If the credentials are incorrect, it sends an HTTP 401 status code (Unauthorized)
 * and displays an error message in the response body.
 */
void handleLoginPost() {
  String username = server.arg("username");
  String password = server.arg("password");

  if (username == validUsername && password == validPassword) {

    // Set login status to true
    isLoggedIn = true;

    // Redirect to the home page after login
    server.sendHeader("Location", "/home");

    // Send HTTP status code 303
    server.send(303);
    
  } else {
    server.send(401, "text/html", "<html><body><h1>Invalid credentials</h1></body></html>");
  }
}


// end of code
