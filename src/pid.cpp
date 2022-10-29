#include "vex.h"
using namespace vex;

///////////////////////////////////////////////
// PID 
///////////////////////////////////////////////

/*
  PID is a way to move some distance or turn to some angle.
  The distance or angle from a target is called error.
  To reduce error precisely, PID uses three variables to calculate speed.
  It adds up the three variables and uses the sum as the speed output.
    P: Proportional
      - If you’re not where you want to be, get there. Go faster if you're far and slower if you're close.
      - Where most of the speed comes from.
    
    I: Integral
      - If you haven’t been where you want to be for a long time, get there faster.
      - Makes sure speed is fast enough to overcome friction & weight of robot (steady-state error).
    D: Derivative
      - If you’re getting close to where you want to be, slow down.
      - Smooths out any unwanted oscillations and overshoot.
  PID is tuned by changing the values Kp, Ki, and Kd (found in the PID cycle functions).
  Each tuning constant controls how much its corresponding variable affects the speed output.
*/

double fwd_error = 0; // Distance from target forward distance
double fwd_lastError = 0; // Keep track of last error for the derivative (rate of change)
double fwd_integral = 0; // Integral accumulates the error, speeding up if target is not reached fast enough
double fwd_derivative = 0; // Derivative smooths out oscillations and counters sudden changes

double turn_error = 0; // Relative angle from the target rotation (degrees)
double turn_lastError = 0;
double turn_integral = 0;
double turn_derivative = 0;




// Get speed by doing one cycle of PID
double fwdPIDCycle(double targetDist, double maxSpeed)
{ 
  double Kp = 5; // Proportional makes the speed proportional to the error

  double Ki = 0.01; // Integral accumulates error to the speed over time
  // Integral is often used to to overcome friction at the end due to derivative

  double Kd = 1;//45.0; // Derivative slows down the speed if it is too fast

  // Don't let the integral term have too much control
  double integralPowerLimit = 40 / Ki; // 40 is the maximum power that integral can contribute
  double integralActiveZone = 10; // Only start accumulating integral
  double errorThreshold = 0.25; // Exit loop when error is less than this

  double speed = 0;

  fwd_error = targetDist; // Error is the distance from the target

  if (fabs(fwd_error) > errorThreshold) // Check if error is over the acceptable threshold
  {
    // Only accumulate error if error is within active zone
    if (fabs(fwd_error) < integralActiveZone)
    {
      fwd_integral = fwd_integral + fwd_error; // Accumulate the error to the integral
    }
    else
    {
      fwd_integral = 0;
    }
    fwd_integral = keepInRange(fwd_integral, -integralPowerLimit, integralPowerLimit); // Limit the integral
  
    fwd_derivative = fwd_error - fwd_lastError; // Derivative is the change in error since the last PID cycle

    speed = (Kp * fwd_error) + (Ki * fwd_integral) + (Kd * fwd_derivative); // Multiply each variable by its tuning constant
    speed =  keepInRange(speed, -maxSpeed, maxSpeed); // Restrict the absolute value of speed to the maximum allowed value

    // make sure speed is always enough to overcome steady-state error
    if (speed > 0 && speed < 2) {
      speed = 2;
    } if (speed < 0 && speed > -2) {
      speed = -2;
    }

    Brain.Screen.printAt(210, 140, "P: %.1f, I: %.1f, D: %.1f    ", (Kp * fwd_error), (Ki * fwd_integral), (Kd * fwd_derivative));
  //Brain.Screen.printAt(210, 120, "%f  ", fwd_error - fwd_lastError);
  }
  else
  {
    // If error threshold has been reached, set the speed to 0
    fwd_integral = 0;
    fwd_derivative = 0;
    speed = 0;
    //Brain.Screen.printAt(210, 140, "PID Fwd Completed             ");
  }
  fwd_lastError = fwd_error; // Keep track of the error since last cycle
  
  return speed; // Return the final speed value
}


// The target angle is relative to the starting angle of the robot in degrees
double turnPIDCycle(double targetDegree, double maxSpeed)
{
  double Kp = 0.56;
  double Ki = 0.00075;//0.01;
  double Kd = 0.5;//1;
  double integralPowerLimit =
      40 / Ki;                   // little less than half power
  double integralActiveZone = 15; // degrees to start accumulating to integral
  double errorThreshold = 0.5; // Exit loop when error is less than this

  double speed = 0;

  turn_error = targetDegree - getRotationDeg(); // The error is the relative angle to the target angle

  if (fabs(turn_error) > errorThreshold)
  {
    if (fabs(turn_error) < integralActiveZone) {
      turn_integral = turn_integral + turn_error;
    } else {
      turn_integral = 0;
    }
    turn_integral = keepInRange(turn_integral, -integralPowerLimit, integralPowerLimit);

    turn_derivative = turn_error - turn_lastError;

    speed = ((Kp * turn_error) + (Ki * turn_integral) + (Kd * turn_derivative));
    speed =  keepInRange(speed, -maxSpeed, maxSpeed);
    Brain.Screen.printAt(210, 160, "P: %.1f, I: %.1f, D: %.1f    ", (Kp * turn_error), (Ki * turn_integral), (Kd * turn_derivative));
  }
  else
  {
    turn_integral = 0;
    turn_derivative = 0;
    speed = 0;
    //Brain.Screen.printAt(210, 160, "PID Turn Completed             ");
  }
  turn_lastError = turn_error;

  return speed;
}


// Run a closed PID loop for linear distance in inches
void forwardPID(double targetInches, double maxSpeed, int timeoutMillis = -1) // default timeout is -1, meaning unlimited
{
  double speed = 1;

  timer Timer;
  Timer.reset(); // start timer for timeout

  resetTotalDistance();
  // While time isn't up or time isn't set (default value of -1)
  while (speed != 0 && (Timer.time() < timeoutMillis || timeoutMillis == -1))
  {
    double currentDist = targetInches - getTotalDistance(); // calculate the distance from target

    speed = fwdPIDCycle(currentDist, maxSpeed); // plug in distance and speed into PID
    setLeftBase(speed);
    setRightBase(speed);

    task::sleep(10);
  }
  stopBase();
}


// Run a closed PID loop for turning to a specific absolute degree
// Setting targetDeg to 0 degrees always returns to the starting angle
void turnPID(double targetDeg, double maxSpeed, int timeoutMillis=-1) // default timeout is -1, meaning unlimited
{
  double speed = 1;

  timer Timer;
  Timer.reset(); // start timer for timeout

  resetTotalDistance();
  // While time isn't up or time isn't set (default value of -1)
  while (speed != 0 && (Timer.time() < timeoutMillis || timeoutMillis == -1))
  {
    speed = turnPIDCycle(targetDeg, maxSpeed);
    setLeftBase(-speed);
    setRightBase(speed);

    task::sleep(10);
  }
  stopBase();
}
