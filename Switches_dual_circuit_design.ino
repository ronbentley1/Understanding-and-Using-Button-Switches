//
//
//  This example and code is in the public domain and may be used without restriction and
//  without warranty.

/*
  READING SIMPLE SWITCHES RELIABLY, WITH OR WITHOUT A SWITCH PULL DOWN RESISTER
  '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
  In this example sketch we look at configuring a simple button switch such that when pressed
  it will toggle a LED on and off.  The sketch automatically allows for one of two switch
  circuits to be configured, either with a pull down switch resister or without.  This is configured
  and controlled with a simple macro parameter - see points of note below.

  Additionally, this sketch offers two methods to read a simple button switch, each offered as a
  specific function.  Only one function method should be configured (..._method_1/..._method_2),
  the choice left to preference of the user. The differences between each method is highlighted and
  explained below:
    Method 1 -  Function 'bool read_switch_method_1()' - this version of switch reading examines the switch
                input once each time the function is called.
                This allows the code section from which it is called to continue without waiting
                for the switch press cycle to complete once switching is initiated.
                Only drawback, of course, is that the design of the calling code must ensure that the
                switch is regularly tested to catch a change in switch status.

    Method 2 -  Function 'bool read_switch_method_2()' - this version of switch reading will wait for a
                switch cycle to complete, once it is initiated, before control is returned to the calling code.
                Once the switch is pressed, the code will fully consume the debounce period AFTER switch
                release until control is returned back to the calling code.
                That is, the calling code will be held up once the switch is pressed and until
                the debounce period has elapsed.

  Points of note:
    1.  The digital pin chosen as the input is initialised according to the 'circuit_type' macro
        parameter, either INPUT_PULLUP or INPUT using a call to pinMode(button_switch,circuit_type)
        in the setup() function.
        As the conditions for detecting switch on/off are different for each 'circuit_type' (they are
        reversed) two variables are used ('switch_low' and 'switch_high') to provide a reference
        indicating low and high (on/off) conditions.  However, this is transparent to the functioning
        of the code.  The only requirement is to define the 'circuit_type', everything else is
        automatically taken care of.

    2.  The design for each reading method incorporates code to debounce spurious inputs when the switch
        is pressed which, if not accounted for, would produce unexpected/spurious results.

    3.  The wiring designs for each type of switch circuit are:

        With switch pull down resister configured and 'circuit_type' of INPUT

              10K ohm     button      (onboard)
              resister    switch         LED
          ____|<><><>|_______ \___        O
         |              |         |       |
         0v         pin = 2      +5v    pin =
                                    LED_BUILTIN
         ___________________________________
         |            ARDUINO              |

        With NO switch pull down resister configured and 'circuit_type' of INPUT_PULLUP

             button     (onboard)
             switch        LED
            ___ \___        O
           |        |       |
         pin 2     0v     pin =
                       LED_BUILTIN
         _______________________
         |       ARDUINO       |

    5.  The on board LED (MEGA 2560) is utilised for testing to keep circuit design to a minimum.
        If not using the onboard LED then configure one in the traditional way using a 230ohm resister (for
        a red LED) and a red LED on a breadboard, or otherwise.
*/
// define type of switch circuit and associated LOW and HIGH variables:
//    - circuit C1, INPUT with switch resister           -> switch_high = HIGH, switch_low = LOW
//    - circuit C2, INPUT_PULLUP with no switch resister -> switch_high = LOW,  switch_low = HIGH

#define circuit_C1    INPUT
#define circuit_C2    INPUT_PULLUP

#define circuit_type  circuit_C1   // circuit type configured, see circuit design specs

int     switch_high, switch_low;

#define button_switch        2     // digital pin connected to button switch
#define debounce            50     // number of milliseconds to wait for switch to settle once pressed
#define switched           true    // signifies switch has been pressed

#define LED        LED_BUILTIN     // digital pin connected to LED, for testing of switch code only
bool    led_status      =  LOW;    // start with LED off, for testing of switch code only

void setup() {
  // define the switch circuit type 
  pinMode(button_switch, circuit_type); // circuit_type == INPUT or INPUT_PULLUP
  // establish meanings for switch on/off depending on circuit_type
  if (circuit_type == INPUT_PULLUP) {
    // switch is NOT configured with a pull down switch resister
    switch_high = LOW;  // switch pin goes LOW when switch pressed, ie on
    switch_low  = HIGH; // switch pin goes HIGH when switch released, ie off
  } else {
    // circuit_type == INPUT, so switch IS configured with a pull down switch resister
    switch_high = HIGH; // switch pin goes HIGH when switch pressed, ie on
    switch_low  = LOW;  // switch pin goes LOW when switch released, ie off
  }
  // set LED pin for output, for testing purposes only
  pinMode(LED, OUTPUT);
}
//
//  Button switch reading, method 1
//  This version of switch reading examines switch once each time the function is called.
//  This allows the code section from which it is called to continue without waiting
//  for the switch press cycle to complete once switching is initiated.
//  Only drawback, of course, is that the design of the calling code must ensure that the
//  switch is regularly tested to catch a change in switch status.
//
bool read_switch_method_1() {
  int             switch_pin_reading;
  // static variables because we need to retain old values between function calls
  static bool     switch_pending = false;
  static long int elapse_timer;
  switch_pin_reading = digitalRead(button_switch);
  if (switch_pin_reading == switch_high) {
    // switch is pressed, so start/restart debounce process
    switch_pending = true;
    elapse_timer = millis();    // start elapse timing
    return !switched;           // now waiting for debounce to conclude
  }
  if (switch_pending && switch_pin_reading == switch_low) {
    // switch was pressed, now released, so check if debounce time elapsed
    if (millis() - elapse_timer > debounce) {
      // dounce time elapsed, so switch press cycle complete
      switch_pending = false;
      return switched;
    }
  }
  return !switched;
}

//
//  Button switch reading, method 2.
//  This version of switch reading will wait for a switch cycle to complete once it
//  initiated, before control is returned to the calling code.
//  That is, the calling code will be held up once the switch is pressed and until
//  the debounce period has elapsed.
//
bool read_switch_method_2() {
  int      switch_pin_reading;
  bool     switch_status;
  long int elapse_timer;
  switch_status = !switched;                            // assume switch not pressed
  //  read the given switch_pin, if pressed will be HIGH, if not pressed will be LOW
  switch_pin_reading = digitalRead(button_switch);
  do {
    if (switch_pin_reading  == switch_high) {
      // switch has been pressed, now debounce
      switch_status = switched;                         // flag that switch was pressed
      elapse_timer  = millis();
      do {} while (millis() - elapse_timer < debounce); // wait for debounce period to lapse
      switch_pin_reading = digitalRead(button_switch);  // see if switch is still being pressed
    }
  } while (switch_pin_reading == switch_high);          // keep debouncing until switch no longer pressed
  return switch_status;                                 // result is either 'switched' or '!switched'
}

void loop() {
  do {
    if (read_switch_method_1() == switched) {
      led_status = HIGH - led_status;                   // flip between HIGH and LOW
      digitalWrite(LED, led_status);
    }
  } while (true);
}
