# robot project
the robot part runs on odroid-n2+ board with Debian (bullseye) with kernel 5.15.5 base one https://github.com/tobetter/linux/tree/odroid-5.15.y
with specific configuration.

- The motion_control directory is a C project used to communicate with FRDM_KV31 board through serial com at 500KHz (frame period = 50 Hz)
- The gamepad directory is a C project used to read gamepad information. The gamepad is able to drive the robot.
- The voice_control is a C project example to generate sound.
- The supervisor project is a C project in charge to manage all events revecied from gamepad, motion_controm, vision_control...
- The vison_control is a C++ projet using opencv-4.5.4 stack to detect face or body and track face.
- The voice_control is a C project able to play sounds using aplay or to speak using software speech synthesizer (espeak and mobrola)   
