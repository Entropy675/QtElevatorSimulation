#ifndef DEFS_H
#define DEFS_H

// time in seconds
#define TIME_ELEVATOR_OPEN			15
#define ELEVATOR_PEOPLE_LIMIT       12
#define SCAN_REQUEST_TREE_SECS      100

// DO NOT USE THESE anywhere other then as defaults for the elevator controller
#define NUM_FLOORS					15
#define NUM_ELEVATORS				6

#define DEBUG_GENVALUES             false
#define AGGRESSIVE_LOGGING          true


class Elevator;
class ElevatorController;

#endif
