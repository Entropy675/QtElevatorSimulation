#ifndef DEFS_H
#define DEFS_H

// time in seconds
#define TIME_ELEVATOR_OPEN			15
#define ELEVATOR_PEOPLE_LIMIT       12
#define SCAN_REQUEST_TREE_SECS      5

// DO NOT USE THESE anywhere other then as defaults for the elevator controller
#define NUM_FLOORS					8
#define NUM_ELEVATORS				4

#define DEBUG_GENVALUES             false
#define AGGRESSIVE_LOGGING          false


class Elevator;
class ElevatorController;

#endif
