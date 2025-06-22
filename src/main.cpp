#include <Arduino.h>
#include <WiFiS3.h>
#include "webpage.h"

#include "Arduino_LED_Matrix.h"
#include "LiquidCrystal.h"
#include "pitches.h"

#include <OPAMP.h>

#define MAX_Y 8
#define MAX_X 12
#define MAX_PATTERN_Y 2
#define MAX_PATTERN_X 2
#define ANZ_PATTERN 2

#define INTERRUPTPIN 2

void draw_grid();
void draw_status();
boolean checkVictory(int currentPlayer);
void playVictorySound();
void reset_status();
void handle_buttonPressed();
int ADC_to_KeyNR(int adc);
void buttonPressed(uint8_t buttnNr);
void setupLibCrystal();

unsigned long dbgBtnPressed = 0;
unsigned long tButtonLastPressed = 0;
boolean isWiFiErledigt = false;
int isButtonPressed = 16;

const char *ssid = "iPhone";	   //"AndMotoG";
const char *password = "gmxspot1"; //"aaaabbbb";

WiFiServer server(80);

int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

uint8_t grid[MAX_Y][MAX_X]{
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

boolean grid_TIC_TAC_TOE[][MAX_Y][MAX_X] = {
	{{0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
	 {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
	 {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
	 {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
	 {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
	 {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
	 {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
	 {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0}},
	{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}};

boolean player_pattern[][MAX_PATTERN_Y][MAX_PATTERN_Y] = {
	{// Player 1
	 {1, 1},
	 {1, 1}},
	{// Player 2
	 {1, 0},
	 {0, 1}}};

uint8_t status[3][3] = {
	{5, 5, 5},
	{5, 5, 5},
	{5, 5, 5}};

ArduinoLEDMatrix matrix;
int currentPattern = 0;
uint8_t currentPlayer = 0; // Player 1 == 0, Player 2 == 1 , free == 5

// Tasten 0 bis 16
const int anz_tasten = 17;
int tasten[anz_tasten] = {933, 780, 670, 565, 483, 439, 403, 361, 328, 307, 290, 269, 240, 239, 228, 100, 0};

// Timer
unsigned long timer = 0;
unsigned long deltaTime = 0;
unsigned long buttonTimer = 0;
unsigned long lastTimer = 0;

// Sound
const int buzzer = 10;

boolean gameOver = false;
int state = 0;

// notes in the melody:
int melody[] = {
	NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
	4, 8, 8, 4, 4, 4, 4, 4};

void handleClient(WiFiClient client)
{
	Serial.print("+");
	String request = "";
	unsigned long timeout = millis() + 1000;
	while (client.connected() && millis() < timeout)
	{
		if (client.available())
		{
			char c = client.read();
			request += c;

			if (request.endsWith("\r\n\r\n"))
			{
				break;
			}
		}
	}

	String response;
	if (request.indexOf("GET /gamestate") >= 0)
	{
		String json = "{\"game\":[";
		for (int i = 0; i < 3; i++)
		{
			json += "[";
			for (int j = 0; j < 3; j++)
			{
				json += String(status[i][j]);
				if (j < 2)
					json += ",";
			}
			json += "]";
			if (i < 2)
				json += ",";
		}
		json += "],\"status\":" + String(currentPlayer) + "}";

		response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n";
		response += json;
	}
	else if (request.indexOf("POST /setplayer") >= 0)
	{
		int contentLength = 0;
		int contentIndex = request.indexOf("Content-Length:");
		if (contentIndex >= 0)
		{
			int lineEnd = request.indexOf("\r\n", contentIndex);
			if (lineEnd >= 0)
			{
				String lengthStr = request.substring(contentIndex + 15, lineEnd);
				contentLength = lengthStr.toInt();
			}
		}

		String body = "";
		unsigned long bodyTimeout = millis() + 1000;
		while (body.length() < contentLength && millis() < bodyTimeout)
		{
			if (client.available())
			{
				char c = client.read();
				body += c;
			}
		}

		int y = body[1] - 48;
		int x = body[3] - 48;

		playerMove(x, y);
	}
	else
	{
		response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
		response += webpage;
	}

	client.print(response);
	delay(1);
	client.stop();
}

void setup()
{
	// put your setup code here, to run once:
	Serial.begin(115200);

	setupLibCrystal();

	matrix.begin();
	matrix.renderBitmap(grid, 8, 12);

	delay(1000);

	Serial.print("Connecting to WiFi");
	// linke obere diode wird blinken, solange wifi nicht verbunden.
	grid_TIC_TAC_TOE[1][0][0] = 1;
	WiFi.begin(ssid, password);

	pinMode(INTERRUPTPIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(INTERRUPTPIN), handle_buttonPressed, RISING);

	OPAMP.begin(OPAMP_SPEED_HIGHSPEED);
}

int dbgServerAvailable = 0;
void loop()
{
	deltaTime = millis() - lastTimer;
	lastTimer = millis();
	timer += deltaTime;

	if (dbgBtnPressed > 0)
	{
		Serial.print("dbgButtonPressed: ");
		Serial.println(dbgBtnPressed);
		dbgBtnPressed = 0;
	}
	if (isButtonPressed != 16)
	{
		Serial.print("Button pressed: ");
		Serial.println(isButtonPressed);
		buttonPressed(isButtonPressed);
		isButtonPressed = 16;
	}

	if ((WiFi.status() == WL_CONNECTED) && !isWiFiErledigt && (WiFi.localIP() != IPAddress(0, 0, 0, 0)))
	{
		// Serial.print(".");
		// delay(500);
		Serial.println("\nConnected!");
		Serial.print("IP Address: ");
		Serial.println(WiFi.localIP());
		lcd.setCursor(0, 0);
		// lcd.print("IP:");
		lcd.print(WiFi.localIP());

		server.begin();
		Serial.println("Server started.");
		grid_TIC_TAC_TOE[1][0][0] = 0;
		isWiFiErledigt = true;
	}
	else if ((WiFi.status() != WL_CONNECTED))
	{
		grid_TIC_TAC_TOE[1][0][0] = 1;
		isWiFiErledigt = false;
		dbgServerAvailable = 0;
		lcd.setCursor(0, 0);
		lcd.print("? SSID:");
		lcd.print(ssid);
	}

	if (isWiFiErledigt)
	{
		WiFiClient client = server.available();
		if (client)
		{
			grid_TIC_TAC_TOE[1][1][0] = 0;
		}
		else
		{
			grid_TIC_TAC_TOE[1][1][0] = 1;
		}

		int dbgServerAvailableNew = server.available();
		dbgServerAvailable = dbgServerAvailableNew;

		if (client)
		{
			handleClient(client);
		}
	}
	// draw LEDs
	if (timer >= 500 && state == 0)
	{
		timer -= 500;
		draw_grid();
		currentPattern = (currentPattern + 1) % ANZ_PATTERN;
	}
	matrix.renderBitmap(grid, 8, 12);
	if (gameOver && state == 1 && timer > 600)
	{
		playVictorySound();
		timer -= 600;
		state = 0;
	}
	draw_status();
}

// checkt which button was pressed as index value
int ADC_to_KeyNR(int adc)
{
	for (int i = 0; i <= anz_tasten; i++)
	{
		if (adc >= tasten[i])
			return i;
	}
	return -1;
}

// draw TIC-TAC-TOE grid layer
void draw_grid()
{
	for (int y = 0; y < MAX_Y; y++)
	{
		for (int x = 0; x < MAX_X; x++)
		{
			grid[y][x] = grid_TIC_TAC_TOE[currentPattern][y][x];
		}
	}
}

// draw TIC-TAC-TOE status
void draw_status()
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{

			if (status[i][j] == 1 || status[i][j] == 0)
			{

				for (int y = 0; y < MAX_PATTERN_Y; y++)
				{
					for (int x = 0; x < MAX_PATTERN_X; x++)
					{
						grid[i * 3 + y][2 + j * 3 + x] = player_pattern[status[i][j]][y][x];
					}
				}
			}
		}
	}
}

void handle_buttonPressed()
{
	unsigned long t = millis();
	dbgBtnPressed = t - tButtonLastPressed;
	if (dbgBtnPressed < 100)
		return;

	int adc = analogRead(A1);
	int taste = ADC_to_KeyNR(adc);
	tButtonLastPressed = t;
	isButtonPressed = taste;
}

void buttonPressed(uint8_t buttnNr)
{
	uint8_t btnIdx = buttnNr;

	if (btnIdx == 15)
	{
		reset_status();
		return;
	}

	if ((btnIdx == 3 || btnIdx == 7 || btnIdx >= 11 || gameOver))
	{
		return;
	}

	playerMove(btnIdx % 4, btnIdx / 4);
}

void playerMove(int x, int y)
{
	if (status[y][x] != 5)
	{
		return;
	}

	status[y][x] = currentPlayer;
	if (!checkVictory(currentPlayer))
	{
		currentPlayer = (currentPlayer == 0) ? 1 : 0;
		lcd.setCursor(0, 1);
		lcd.print("Spieler ");
		lcd.print(currentPlayer + 1);
		lcd.print(" dran  ");
	}
	else
	{
		draw_status();
		matrix.renderBitmap(grid, 8, 12);
		Serial.println((String) "Der Sieger ist Spieler: " + currentPlayer);

		lcd.setCursor(0, 1);
		lcd.print(currentPlayer + 1);
		lcd.print(" hat gewonnen!");
		gameOver = true;
		timer = 0;
		state = 1;
	}
}

void reset_status()
{
	for (int i = 0; i < 3; i++)
	{
		status[i][0] = 5;
		status[i][1] = 5;
		status[i][2] = 5;
	}
	currentPlayer = 0;
	gameOver = false;
	lcd.setCursor(0, 1);
	lcd.print("Spieler 1 dran  ");
}

boolean checkVictory(int currentPlayer)
{

	// check Victory horizontal
	for (int y = 0; y < 3; y++)
	{
		for (int x = 0; x < 3; x++)
		{
			if (status[y][x] != currentPlayer)
			{
				break;
			}
			else if (x == 2)
			{
				Serial.println((String) "horizonal X:" + x + " Y: " + y);
				return true;
			}
		}
	}

	// check Victory vertical
	for (int x = 0; x < 3; x++)
	{
		for (int y = 0; y < 3; y++)
		{
			if (status[y][x] != currentPlayer)
			{
				break;
			}
			else if (y == 2)
			{
				Serial.println((String) "vertical X:" + x + " Y: " + y);
				return true;
			}
		}
	}

	// check Victory diagonal left up to botten right
	for (int y = 0; y < 3; y++)
	{
		if (status[y][y] != currentPlayer)
		{
			break;
		}
		else if (y == 2)
		{
			Serial.println((String) "digonal 1 X:" + y + " Y: " + y);
			return true;
		}
	}

	// check Victory diagonal left bottem to right up
	for (int y = 0; y < 3; y++)
	{
		if (status[y][2 - y] != currentPlayer)
		{
			break;
		}
		else if ((2 - y) == 0)
		{
			Serial.println((String) "diagonal 2 X:" + (2 - y) + " Y: " + y);
			return true;
		}
	}

	return false;
}

void playVictorySound()
{
	for (int thisNote = 0; thisNote < 8; thisNote++)
	{

		// to calculate the note duration, take one second divided by the note type.
		// e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
		int noteDuration = 1000 / noteDurations[thisNote];
		tone(buzzer, melody[thisNote], noteDuration);

		// to distinguish the notes, set a minimum time between them.
		// the note's duration + 30% seems to work well:
		int pauseBetweenNotes = noteDuration * 1.30;
		delay(pauseBetweenNotes);
		// stop the tone playing:
		noTone(buzzer);
	}
}

void setupLibCrystal()
{

	// set up the LCD's number of columns and rows:
	lcd.begin(16, 2);

	// Benutzerdefinierte symbole:
	byte chr1[8] = {
		0b00100,
		0b11111,
		0b01010,
		0b01010,
		0b00100,
		0b11111,
		0b00100,
		0b01010};

	byte chr2[8] = {
		0b00000,
		0b00000,
		0b00011,
		0b01100,
		0b10000,
		0b00000,
		0b00000,
		0b00000};

	lcd.createChar(1, chr1);
	lcd.createChar(2, chr2);
	lcd.setCursor(0, 1);
	lcd.print("Spieler 1 dran  ");
}
