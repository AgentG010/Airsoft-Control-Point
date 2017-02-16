/*
 * Arduino Game Enhancement Device for Airsoft
 * Airsoft Control Point for King of the Hill Gamemode from Team Fortress 2
 *
 * Two teams, RED and BLU, fight over the posession of a control point.
 * To capture the point, the capturing team has to be selected by using
 * a toggle pushbutton, and the capture pushbutton has to be held down for 10
 * seconds in order for the selected team to capture it. Every team has a
 * timer, and the timer counts down for the team that has it currently captured.
 *
 * Code by George Troulis
 * KOTH Gamemode Concept by Team Fortress 2
 *
 * TODO: Add overtime functionality
 *       Add dynamic time changing
 *       Use 1 pushbutton per team rather than a selector and a capturer
 */

#include <LiquidCrystal.h>

//////////////////////////
// I/O Declarations //////
//////////////////////////


// TODO: change these from selector/capturer to blucapture/redcapture
const int selector = 13; // Team selector pushbutton
const int button   = 12; // The pushbutton that captures the point

const int speaker = 11;

const int redLED   = 8;
const int greenLED = 9;

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

//////////////////////////
// Game State Variables //
//////////////////////////

//Team Timers (mmss format)
int redTimer = 1000;
int bluTimer = 1000;

//The team selected by the switch (start with Blu)
String selectedTeam = "Blu";

//The team that currently has the point; starts with none
String activeTeam = "none";

//the capturing progress of the point
int captureProgress = 0;
boolean capturing   = false;

//for timing purposes
int tick = 0;

//true once one of the timers has finished counting down
boolean won = false;

////////////////////////////////////////////////////
// Main Functions //////////////////////////////////
////////////////////////////////////////////////////

void setup()
{
    pinMode(selector,  INPUT);
    pinMode(button,    INPUT);
    pinMode(speaker,  OUTPUT);
    pinMode(redLED,   OUTPUT);
    pinMode(greenLED, OUTPUT);

    lcd.begin(20, 4);
}

void loop()
{
    updateDisplay();
    updateTeam();
    updateCapture();
    countDown();

    delay(40);
    updateTick();

    if (won) gameOver();
}

//////////////////////////
// Other Functionality ///
//////////////////////////

//Gets the output of the selector switch
void updateTeam()
{
    //If point is being captured, don't change the capturing team
    static int prevState;
    int state = digitalRead(selector);

    if (!capturing)
    {
        if (prevState == LOW && state == HIGH)
        {
            if (selectedTeam != "Blu")  selectedTeam = "Blu";
            else                        selectedTeam = "Red";
        }
    }
    prevState = state;
}

void countDown()
{
    if (tick == 0)
    {
        if (activeTeam == "Red")
        {
            digitalWrite(redLED, HIGH);
            digitalWrite(greenLED, LOW);
            countDownTime(&redTimer);
        }
        else if (activeTeam == "Blu")
        {
            digitalWrite(greenLED, HIGH);
            digitalWrite(redLED, LOW);
            countDownTime(&bluTimer);
        }
    }
}

void countDownTime(int* time)
{
    int _time = *time;

    if (_time > 0)
    {
        if (_time % 100 == 0)
            _time -= 41;
        else
            _time --;
    }
    else
        won = true;

    if (_time < 100 && !capturing)
        tone(speaker, 500, 300);

    *time = _time;
}

//formats a time from mmss to mm:ss, and adds leading 0's if necessary
String formatTime(int time)
{
    String result = "";
    int _min = time / 100; //min is taken, so _min
    int sec = time % 100;

    //add the minutes, and a leading 0 if necessary
    if (_min < 10)
        result += '0';
    result += _min;

    result += ':'; //separator ':'

    //add the seconds, and a leading 0 if necessary
    if (sec < 10)
        result += '0';
    result += sec;

    return result;
}

//makes sure the proper stuff is displayed
void updateDisplay()
{
    lcd.clear();
    lcd.print("Selected:");
    lcd.print(selectedTeam);

    lcd.setCursor(0, 2);
    lcd.print("Blu Time   Red Time");

    //capture progress
    lcd.setCursor(0, 1);
    for (int i = 0; i < captureProgress; i++)
        lcd.print('-');

    //Times of the teams
    lcd.setCursor(2, 3);
    lcd.print(formatTime(bluTimer));

    lcd.setCursor(13, 3);
    lcd.print(formatTime(redTimer));
}

// updates the capturing progress of the point
void updateCapture()
{
    static int prevState = digitalRead(button);
    int state = digitalRead(button);
    if (state == HIGH && activeTeam != selectedTeam)
    {
        //TODO: Analyse this statement for potential timing error
        if (state != prevState)
            tick = 0;//make sure to count from the beginning
        capturing = true;
        if (captureProgress < 10)
        {
            if (tick % 500 == 0) //increase counting every half second
            {
                captureProgress ++;
                tone(speaker, 300, 100);
                delay(100);
            }
        }
        else
        {
            //done capturing
            lcd.setCursor(0, 1);
            lcd.print("                    ");
            lcd.setCursor(0, 1);
            lcd.print("Captured!");
            activeTeam = selectedTeam;
            tone(speaker, 750, 200);
            delay(100);
            tone(speaker, 1000, 200);
            delay(400);
            captureProgress = 0;
        }
    }
    else
    {
        capturing = false;
        captureProgress = 0;
    }

    prevState = state;
}

void updateTick()
{
    if (tick < 1000)    tick += 50;
    else                tick = 0;
}

// handles end game
void gameOver()
{
    lcd.clear();

    lcd.setCursor(5, 1);
    lcd.print("Game Over!");

    lcd.setCursor(5, 2);
    String victoryTeam = (redTimer == 0 ? "Red" : "Blu");
    lcd.print(victoryTeam + " won");

    while(true)
    {
        tone(speaker, 500, 200);
        delay(200);
        tone(speaker, 600, 200);
        delay(200);
        tone(speaker, 700, 200);
        delay(200);
        tone(speaker, 800, 200);
        delay(200);
        tone(speaker, 900, 200);
        delay(200);
    }
}

