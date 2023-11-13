#ifndef DEFS_H
#define DEFS_H

// time in seconds
#define TIME_ELEVATOR_OPEN                      10
#define SCAN_REQUEST_TREE_SECS                  10
#define ELEVATOR_SPEED_MS                       500
#define HELP_COUNTER                            10
#define ELEVATOR_BLOCK_DOOR_TIME_SENSITIVITY    3 // seconds

// Dimensions and limits
#define CUBE_SIZE                               64
#define BUTTON_SCALE                            32
#define ELEVATOR_PEOPLE_LIMIT                   12
#define SAFE_FLOOR                              1

// Used purely as defaults for ElevatorController
#define NUM_FLOORS                              8
#define NUM_ELEVATORS                           4

#define AGGRESSIVE_LOGGING                      true

class Elevator;
class ElevatorController;

#endif
