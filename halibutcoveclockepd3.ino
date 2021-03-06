/*  Tide_calculator.ino 
 Copyright (c) 2015 Luke Miller
This code calculates the current tide height for the 
pre-programmed site. It requires a real time clock
(DS1307 or DS3231 chips) to generate a time for the calculation.
The site is set by the name of the included library (see line 44 below)

Written under version 1.6.4 of the Arduino IDE.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or 
(at your option) any later version.

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTIBILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/

 The harmonic constituents used here were originally derived from 
 the Center for Operational Oceanic Products and Services (CO-OPS),
 National Ocean Service (NOS), National Oceanic and Atmospheric 
 Administration, U.S.A.
 The data were originally processed by David Flater for use with XTide,
 available at http://www.flaterco.com/xtide/files.html
As with XTide, the predictions generated by this program should 
NOT be used for navigation, and no accuracy or warranty is given
or implied for these tide predictions. The chances are pretty good
that the tide predictions generated here are completely wrong.
It is highly recommended that you verify the output of these predictions
against the relevant NOAA tide predictions online.
 

This software is a modification of the above software.  It utilizes  a couple of functions to calculate the
next high or low tide and then choose one of ten photos to display on the epd display.  It also has the ability 
to output the current time from the Real time clock and store a number of days that display unique photos for the
occasion.  The program is designed to boot on powering and then send a high signal to tpl5111 to shut down the 
enable pin.   
*/

#include <epd.h>
const int wake_up = 2;
const int reset = 3;
bool bing = 1;
#include <Wire.h> // Required for RTClib
#include <SPI.h> // Required for RTClib to compile properly
#include <RTClib.h> // From https://github.com/millerlp/RTClib
RTC_DS3231 RTC; // Uncomment when using this chip
float tidalDifference=0;
 float pastResult;
int dS = 0;
 const int rows = 5;
 const int columns = 2;
 //easterEgg is a array that contains the special days for all day photos--month,day photos is the 
 // array cast start at photo 11 on the card and correspond to rows + 1
  int easterEgg[rows][columns] = {{11,29},{7,15},{7,1},{5,22},{11,21}};
 char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
 DateTime future;
 int slope;
 int i=0;
 int zag = 0;
 bool gate=1;
 // Tide calculation library setup.
// Change the library name here to predict for a different site.
//Please go to his website to download other series for your particular tidal site
#include "TidelibSeldoviaCookInletAlaska.h"
// Other sites available at http://github.com/millerlp/Tide_calculator
TideCalc myTideCalc; // Create TideCalc object called myTideCalc
int currMinute; // Keep track of current minute value in main loop


float results; // results holds the output from the tide calc. Units = ft.






//function to calculate tidal differenc between current time and high or low tide and choose photo
void servoTime(DateTime now, DateTime future,int highLow,int dS) {
  int timer = 0;
 
  /*
  if(highLow){
     Serial.println("I found HIGH tide");
  }
  else Serial.println("I found LOW tide");
 Serial.print(future.year(), DEC);
  Serial.print("/");
  Serial.print(future.month(), DEC); 
  Serial.print("/");
  Serial.print(future.day(), DEC); 
  Serial.print("  ");
   Serial.print(future.hour(), DEC); 
  Serial.print(":");
  if (future.minute() < 10) {
    Serial.print("0");
    Serial.print(future.minute());
   }
  else if (future.minute() >= 10) {
    Serial.print(future.minute());
  }
  //Serial.print(":");
  if (future.second() < 10) {
    Serial.print("0");
    Serial.println(future.second());
  }
  else if (future.second() >= 10) {
    Serial.println(future.second());
  }
 
*/
  int hourDifference = ((future.unixtime() - now.unixtime())/360);
 //Serial.print("difference");
 //Serial.print((future.unixtime() - now.unixtime())/360);
   hourDifference = map( hourDifference, 0,60,1,6);
  //hourDifference = hourDifference + dS;
  hourDifference = constrain(hourDifference, 1, 5);
 
   //Serial.print("hourDifference");
   //Serial.println(hourDifference);

  if(highLow) timer = hourDifference + 5;
  else timer = hourDifference;
  //Serial.print("timer");
  //Serial.println(timer);
   draw_bitmap_demo(timer);
}

//function to display the choosen photo
 void draw_bitmap_demo(int i){
  char flag = 0;
  epd_clear();
  
  //for(int  i = 1; i <11; i++) {
    epd_clear();
    delay(100);
  String indexStr = String(i);
  String str = 'i' + indexStr + ".BMP ";
  char character[str.length()] ;
  str.toCharArray(character, str.length());
  epd_disp_bitmap(character, 0, 0);
  epd_udpate();
  delay(5000);
  epd_enter_stopmode();
  //high signal to terminate power to nano
  while (1)
  {
    if(flag)
    {
      flag = 0;
      digitalWrite(6, HIGH);
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
    }
    else
    {
      flag = 1;
      digitalWrite(6, LOW);
      digitalWrite(LED_BUILTIN, HIGH);
    }
    delay(500);
  }
  
  //}
  
}

void setup(void)
{
  
  Wire.begin(); 
  RTC.begin();
  //RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));//unmask this if you havnt set up your RTC
  // For debugging output to serial monitor
  epd_init(wake_up, reset);
  epd_wakeup(wake_up);
  epd_set_memory(MEM_TF);
  //Serial.begin(57600);
   pinMode(6, OUTPUT);
   digitalWrite(6, LOW);
 DateTime now = RTC.now(); // Get current time from clock
 
 /*
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
  Serial.println("Calculating tides for: ");
  Serial.print(myTideCalc.returnStationID());
  Serial.print(" ");
  Serial.println(myTideCalc.returnStationIDnumber());
*/
   for (int x = 0; x < rows; x++){
     if (easterEgg[x][0] == now.month()){
      if(easterEgg[x][1] == now.day()){
        x = x + 11;
        //Serial.print("here is x");
        //Serial.println(x);
        draw_bitmap_demo(x);
      }
     }
  }
 
  if((now.month()>3&&now.month()<11)||(now.month()==3&&now.day()>11)||(now.month()==11&&now.day()<6))dS=1;
   now = (now.unixtime() - dS*3600);
    pastResult=myTideCalc.currentTide(now);
  while(bing){ //This loop asks when the next high or low tide is by checking 15 min intervals from current time
    i++;
   
    DateTime future(now.unixtime() + (i*15*60L));
    results=myTideCalc.currentTide(future);
    tidalDifference=results-pastResult;
    if (gate){
      if(tidalDifference<0)slope=0;//if slope is positive--rising tide--slope neg falling tide
      else slope=1;
      gate=0;
   }
   if(tidalDifference>0&&slope==0){
      servoTime( now,future,slope,dS);
      gate=1;
      bing = 0;
       
   }
    else if(tidalDifference<0&&slope==1){
    servoTime( now,future,slope,dS);
    gate=1;
    bing = 0;
  
   }
    pastResult=results;
    //Serial.print("results");
    //Serial.print(results);
    //Serial.print(future.year());
    
  }
  /*
  Serial.print(i);
  Serial.println("slope");
  Serial.print(gate);
  Serial.println("gate");
  Serial.print(future.year());
  //Serial.print("/");
 // Serial.print(future.month()); 
 // Serial.print("/");
 // Serial.print(future.day()); 
  //Serial.print("  ");
  // Serial.print(future.hour()); 
  //Serial.print(":");
 
  //delay(5000);
  int hourFuture=future.hour();
  int servoWrite=0;
  if(slope){
     Serial.println("I found HIGH tide");
  }
  else Serial.println("I found LOW tide");
 
  Serial.print(future.year());
  //Serial.print("/");
  //Serial.print(future.month()); 
  //Serial.print("/");
  //Serial.print(future.day()); 
  //Serial.print("  ");
   Serial.print(future.hour()); 
  Serial.print(":");
  if (future.minute() < 10) {
    Serial.print("0");
    Serial.print(future.minute());
   }
  else if (future.minute() >= 10) {
    Serial.print(future.minute());
  }
  Serial.print(":");
  if (future.second() < 10) {
    Serial.print("0");
    Serial.println(future.second());
  }
  else if (future.second() >= 10) {
    Serial.println(future.second());
  }
  

  
  /*
  user led init
  */
/*
  epd_init(wake_up, reset);
  epd_wakeup(wake_up);
  epd_set_memory(MEM_TF);

  */
}

void loop(){

}

