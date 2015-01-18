#include <SPI.h>
#include <UIPEthernet.h>

#define cs   10
#define dc   9
#define rst  8
#define rele_signal 7
#define analogPin A0
#define buzzer_pin 2

byte mac[] = { 0x00, 0x1A, 0x79, 0x05, 0x74, 0xAA };
EthernetServer server(3300);
int co2 = 9999;
float v400ppm = 3.95;   //MUST BE SET ACCORDING TO CALIBRATION
float v40000ppm = 1.55; //MUST BE SET ACCORDING TO CALIBRATION````````````````````````
float deltavs = v400ppm - v40000ppm;
float A = deltavs / (log10(400) - log10(40000));
float B = log10(400);
int readingDelay = 500;

// The IP address will be dependent on your local network.
// If you have IP network info, uncomment the lines starting
// with IPAddress and enter relevant data for your network.
// If you don't know, you probably have dynamically allocated IP adresses, then
// you don't need to do anything, move along.
// IPAddress ip(192,168,1, 177);
// IPAddress gateway(192,168,1, 1);
// IPAddress subnet(255, 255, 255, 0);

void printServerStatus()
{
  Serial.print("Server address:");
  Serial.println(Ethernet.localIP());
}

void sendResponse(EthernetClient* client, String response)
{
  // Send response to client.
  client->println(response);

  // Debug print.
  Serial.println("sendResponse:");
  Serial.println(response);
}

// Read the request line,
String readRequest(EthernetClient* client)
{
  String request = "";

  // Loop while the client is connected.
  while (client->connected())
  {
    // Read available bytes.
    while (client->available())
    {
      // Read a byte.
      char c = client->read();

      // Print the value (for debugging).
      Serial.write(c);

      // Exit loop if end of line.
      if ('\n' == c)
      {
        return request;
      }

      // Add byte to request line.
      request += c;
    }
  }
  return request;
}

void executeRequest(EthernetClient* client, String* request)
{
  char command = readCommand(request);
  int n = readParam(request);
  if ('O' == command)
  {
    pinMode(n, OUTPUT);
  }
  else if ('I' == command)
  {
    pinMode(n, INPUT);
  }
  else if ('L' == command)
  {
    digitalWrite(n, LOW);
  }
  else if ('H' == command)
  {
    digitalWrite(n, HIGH);
  }
  else if ('R' == command)
  {
    sendResponse(client, String(digitalRead(n)));
  }
  else if ('A' == command)
  {
    sendResponse(client, String(analogRead(n)));
  }
}

// Read the command from the request string.
char readCommand(String* request)
{
  String commandString = request->substring(0, 1);
  return commandString.charAt(0);
}

// Read the parameter from the request string.
int readParam(String* request)
{
  // This handles a hex digit 0 to F (0 to 15).
  char buffer[2];
  buffer[0] = request->charAt(1);
  buffer[1] = 0;
  return (int) strtol(buffer, NULL, 16);
}

void setup()
{
  Serial.begin(9600);                              //UART setup, baudrate = 9600bps

  // Wait for serial port to connect. Needed for Leonardo only.
  while (!Serial) {
    ;
  }


  // Initialize the Ethernet shield.
  // If you entered fixed ipaddress info, gateway, subnet mask,
  // then uncommment the next line.
  // Ethernet.begin(mac, ip, gateway, subnet);

  // If it works to get a dynamic IP from a DHCP server, use this
  // code to test if you're getting a dynamic adress. If this
  // does not work, use the above method of specifying an IP-address.
  // dhcp test starts here
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("Failed to configure Ethernet using DHCP");
    // No point in carrying on, stop here forever.
    while (true) ;
  }
  // dhcp test end

  // Start the server.
  server.begin();

  // Print status.
  printServerStatus();

  pinMode(analogPin, INPUT);
  pinMode(rele_signal, OUTPUT);
  digitalWrite(rele_signal, LOW);
  pinMode(buzzer_pin, OUTPUT);
  digitalWrite(buzzer_pin, HIGH);
}

void loop()
{

//  int data = analogRead(analogPin); //digitise output from c02 sensor
//  float voltage = data * 5.0 / 1024;     //convert output to voltage
//
//  // Calculate co2 from log10 formula (see sensor datasheet)
//  float power = ((voltage - v400ppm) / A) + B;
//  float co2ppm = pow(10, power);
//  co2 = co2ppm;
//
//  if (co2 > 1000)
//  {
//    digitalWrite(buzzer_pin, LOW);
//    delay(25);
//    digitalWrite(buzzer_pin, HIGH);
//  }
//  else
//  {
//    digitalWrite(buzzer_pin, HIGH);
//  }

  // Listen for incoming client requests.
  EthernetClient client = server.available();
  if (!client)
  {
    return;
  }

  Serial.println("Client connected");

  String request = readRequest(&client);
  executeRequest(&client, &request);

  // Close the connection.
  client.stop();

  Serial.println("Client disonnected");
  // wait for a moment
  delay(readingDelay);
}

