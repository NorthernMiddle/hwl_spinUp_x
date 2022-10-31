/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       C:\Users\reece.turner                                     */
/*    Created:      Fri Oct 21 2022                                           */
/*    Description:  V5 project                                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

// ---- START VEXCODE CONFIGURED DEVICES ----
// Robot Configuration:
// [Name]               [Type]        [Port(s)]
// LeftFront            motor         1
// LeftBack             motor         2
// RightFront           motor         6
// RightBack            motor         7
// Indexer              motor         5
// Intake               motor         16
// Flywheel             motor         9
// Flywheel2            motor         10
// Controller1          controller
// ---- END VEXCODE CONFIGURED DEVICES ----

///////////////////////////////////////////////////
//
//  --| CONTROLS |--
//  L1 INTAKE IN
//  L2 INTAKE OUT
//  R1 INDEXER
//  R2 FLYWHEEL CONTROL
//  TANK DRIVE
//
//////////////////////////////////////////////////

#include "vex.h"
#include "drivePower.cpp"
using namespace vex;

// A global instance of competition
competition Competition;

// define your global instances of motors and other devices here

/*---------------------------------------------------------------------------*/
/*                          Pre-Autonomous Functions                         */
/*                                                                           */
/*  You may want to perform some actions before the competition starts.      */
/*  Do them in the following function.  You must return from this function   */
/*  or the autonomous and usercontrol tasks will not be started.  This       */
/*  function is only called once after the V5 has been powered on and        */
/*  not every time that the robot is disabled.                               */
/*---------------------------------------------------------------------------*/

void pre_auton(void) {
  // Initializing Robot Configuration. DO NOT REMOVE!
  vexcodeInit();

  // All activities that occur before the competition starts
  // Example: clearing encoders, setting servo positions, ...
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                              Autonomous Task                              */
/*                                                                           */
/*  This task is used to control your robot during the autonomous phase of   */
/*  a VEX Competition.                                                       */
/*                                                                           */
/*  You must modify the code to add your own robot specific commands here.   */
/*---------------------------------------------------------------------------*/

void autonomous(void) { 
  // ..........................................................................
  // Insert autonomous user code here.
  // ..........................................................................
}

/*---------------------------------------------------------------------------*/
/*                                                                           */
/*                              User Control Task                            */
/*                                                                           */
/*  This task is used to control your robot during the user control phase of */
/*  a VEX Competition.                                                       */
/*                                                                           */
/*  You must modify the code to add your own robot specific commands here.   */
/*---------------------------------------------------------------------------*/
int speeds[] = {0, 127, 115, 110};
int speed = 0;
bool indexer_pressed = false;
int target = 0;
int indexer_timer = 0;

void usercontrol(void) {

  bool last_r2 = false;
  // User control code here, inside the loop

  while (true) {
    // This is the main execution loop for the user control program.
    // Each time through the loop your program should update motor + servo
    // values based on feedback from the joysticks.

    // ........................................................................
    // Insert user code here. This is where you use the joystick values to
    // update your motors, etc.
    // ........................................................................

    // Tank Drive
    int l_joy = abs(Controller1.Axis3.value()) > 5 ? Controller1.Axis3.value() : 0;
    int r_joy = abs(Controller1.Axis2.value()) > 5 ? Controller1.Axis2.value() : 0;
    set_tank(l_joy, r_joy);

    // L1 Intake In
    if (Controller1.ButtonL1.pressing()) {
      set_intake(127); 
    // L2 Intake Out
    } else if (Controller1.ButtonL2.pressing()) {
      set_intake(-127);
    } else {
      set_intake(0);
    }
    // R2 Flywheel Control
    if (Controller1.ButtonR2.pressing() and !last_r2) {
      if (speed >= 3) {
        speed = 0;
      } else {
        speed++;
      } 
      set_flywheel(speeds[speed]);
    }
    printf("\n %d", speeds[speed]);
    last_r2 = Controller1.ButtonR2.pressing();

    // R1 Indexer
    if (Controller1.ButtonR1.pressing()) {
      indexer_pressed = true;
    }
    if (indexer_pressed) {
      if (indexer_timer <= 17) {
        target = 160;
        indexer_timer++;
      } else {
        indexer_timer = 0;
        target = 0;
        indexer_pressed = false;
      }
    }

    set_indexer(target, 200);
    wait(20, msec); // Sleep the task for a short amount of time to
                    // prevent wasted resources.
  }
}

////////////////////////////////////////////////////////////////////////////
//
//  --| MAIN |-- Main will set up the competition functions and callbacks.
//
////////////////////////////////////////////////////////////////////////////
int main(){

  // Set up callbacks for autonomous and driver control periods.
  Competition.autonomous(autonomous);
  Competition.drivercontrol(usercontrol);

  // Run the pre-autonomous function.
  pre_auton();

  // Prevent main from exiting with an infinite loop.
  while (true) {
    wait(100, msec);
  }
}
