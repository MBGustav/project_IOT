#include "robot_conf.h"
#include <string.h>

/* Robot current position in the grid */
int robot_pos_x = 0;
int robot_pos_y = 0;

/* Robot current heading (direction it is facing) */
heading_t robot_heading = NORTH;

/* Global map of the environment (grid of fields) */
field_t global_map[MAP_GRID_SIZE][MAP_GRID_SIZE];

/* Function prototypes */
void map_init();
void mark_field(int coord_x, int coord_y, field_t field);
field_t get_field(int coord_x, int coord_y);
void mark_field(int coord_x, int coord_y, field_t field);

/* Initialize the map: set every cell to "unknown" */
void map_init()
{
    for(int i = 0; i < MAP_GRID_SIZE; i++){
        for(int j = 0; j < MAP_GRID_SIZE; j++){
            mark_field(i,j, unknown);
        }
    }
}

/* Write a value into a map cell (with bounds checking) */
void mark_field(int coord_x, int coord_y, field_t field)
{
    // constraint: ignore if outside grid
    if (coord_x < 0 || coord_x >= MAP_GRID_SIZE) return;
    if (coord_y < 0 || coord_y >= MAP_GRID_SIZE) return;

    global_map[coord_x][coord_y] = field;
}

/* Read a value from a map cell (with bounds checking) */
field_t get_field(int coord_x, int coord_y)
{
    if (coord_x < 0 || coord_x >= MAP_GRID_SIZE) return -1;
    if (coord_y < 0 || coord_y >= MAP_GRID_SIZE) return -1;

    return global_map[coord_x][coord_y];
}

/* Compute the next (x,y) cell if we move one step in direction dir */
void get_next_cell(int x, int y, heading_t dir, int* next_x, int* next_y)
{
    *next_x = x;
    *next_y = y;

    switch (dir)
    {
        case NORTH:(*next_y)--; break;
        case SOUTH:(*next_y)++; break;
        case EAST: (*next_x)++; break;
        case WEST: (*next_x)--; break;
    }
}

/* Convert heading enum into readable text */
char *heading_to_string(heading_t heading)
{
    switch (heading)
    {
        case NORTH: return "NORTH";
        case SOUTH: return "SOUTH";
        case EAST:  return "EAST";
        case WEST:  return "WEST";
        default:    return "UNKNOWN";
    }
}

/* Rotate the robot so it faces "target" heading */
void rotate_to(heading_t target)
{
    /* Difference between target direction and current direction */
    int diff = target - robot_heading;

    if (diff == 0) return; // already facing the right way

    /* Determine whether right, left, or 180-degree turn is needed */
    if (diff == 1 || diff == -3) {
        printf("Rotating right from %d to %d\n", robot_heading, target);
        trolley_steering(CB_TURN_RIGHT_IN_PLACE, 90);
    }
    else if (diff == -1 || diff == 3) {
        printf("Rotating left from %d to %d\n", robot_heading, target);
        trolley_steering(CB_TURN_LEFT_IN_PLACE, 90);
    }
    else {
        printf("Rotating 180 degrees from %d to %d\n", robot_heading, target);
        trolley_steering(CB_TURN_LEFT_IN_PLACE, 180);
    }

    /* Update internal state */
    robot_heading = target;
}

/* Update robot_pos_x/robot_pos_y based on current heading after moving */
bool update_position(void)
{
    int next_x = robot_pos_x;
    int next_y = robot_pos_y;

    /* Compute the next cell from heading */
    switch (robot_heading)
    {
        case NORTH: next_y--; break;
        case SOUTH: next_y++; break;
        case EAST:  next_x++; break;
        case WEST:  next_x--; break;
    }

    /* Reject moves that would go outside the grid */
    if (!is_valid_cell(next_x, next_y))
    {
        printf("Blocked movement outside grid: (%d,%d)\n", next_x, next_y);
        return false;
    }

    /* Commit position change */
    robot_pos_x = next_x;
    robot_pos_y = next_y;

    printf("Robot (x,y, heading): (%d, %d, %s)\n",
           robot_pos_x, robot_pos_y, heading_to_string(robot_heading));

    /* Mark the new cell as explored */
    mark_field(robot_pos_x, robot_pos_y, explored);

    return true;
}

/* Move exactly one grid cell forward (physical move + internal update) */
void move_one_cell_forward(void)
{
    printf("Robot (x,y, heading): (%d, %d, %s)\n",
           robot_pos_x, robot_pos_y, heading_to_string(robot_heading));

    /* Ensure robot is oriented correctly (mostly redundant here) */
    rotate_to(robot_heading);

    /* Drive one cell distance */
    move_distance(CUTEBOT_FORWARD, CELL_CM);

    /* Update internal coordinates */
    if (!update_position())
    {
        printf("Movement blocked\n");
    }
}

/* Return true if (x,y) is inside the map grid */
bool is_valid_cell(int x, int y)
{
    if (x < 0 || x >= MAP_GRID_SIZE) return false;
    if (y < 0 || y >= MAP_GRID_SIZE) return false;

    return true;
}

/* Use sonar to check obstacle in front; mark next cell as occupied if needed */
bool check_obstacle_ahead(void)
{
    int next_x, next_y;

    /* Read sonar distance */
    int dist = (int)get_sonar_distance_cm();
    printf("Distance: %d cm\n", dist);

    /* Compute which cell is in front of the robot */
    get_next_cell(robot_pos_x, robot_pos_y, robot_heading, &next_x, &next_y);

    /* If too close, treat it as an obstacle in the next cell */
    if (dist <= SAFE_OBSTACLE_THRESHOLD)
    {
        if (is_valid_cell(next_x, next_y)) {
            mark_field(next_x, next_y, occuppied);
        } else {
            printf("Obstacle detected outside of map boundaries at cell (%d, %d).\n",
                   next_x, next_y);
        }
        return true;
    }

    return false;
}
    
    
void return_to_base()
{
    
    printf("Returning to base...\n");
    rotate_to(NORTH);
    // move up until we reach the top row (y=0)
    while(robot_pos_y > 0){
        printf("current pos: (%d, %d)\n", robot_pos_x, robot_pos_y);
        move_one_cell_forward();
        delay_ms(100);
    }
    // move left until we reach the leftmost column (x=0)
    move_one_cell_forward();
    

    rotate_to(WEST);
    // move left until we reach the leftmost column (x=0)
    while(robot_pos_x > 0){
        printf("current pos: (%d, %d)\n", robot_pos_x, robot_pos_y);
        move_one_cell_forward();
    }
    // final move to (0,0) if not already there
    move_one_cell_forward();
    

    printf("Arrived at base (0,0)\n");
    // make a little dance to celebrate! :D
    rotate_to(NORTH);
    rotate_to(SOUTH);
    rotate_to(EAST);
}
    
    
void explore_grid(void)
{
    int direction = 1;
    
    robot_pos_x = 0;
    robot_pos_y = 0;
    robot_heading = EAST;
    printf("Starting exploration...\n");
    mark_field(0, 0, explored);
    
    for(int y = 0; y < MAP_GRID_SIZE; y++)
    {
        // move in the current row
        for(int i = 0; i < MAP_GRID_SIZE; i++)
        {
            
            if(check_obstacle_ahead())
            {
            
                // this step ends the exploration, so we mark the cell as occupied and return to base 
                map_led_location(robot_pos_x,robot_pos_y);
                return_to_base();
                return;
            }

            move_one_cell_forward();
        }
        
        // if we are not on the last row, move down and switch direction
        if(y < MAP_GRID_SIZE - 1)
        {   
            printf("Changing row from %d to %d\n", y, y+1);
            if(direction == 1)
            {
                rotate_to(SOUTH);
                printf("Turning right to move down\n");
                if(check_obstacle_ahead())
                {
                    printf("Obstacle detected at cell (%d, %d)\n", robot_pos_x, robot_pos_y);
                    map_led_location(robot_pos_x,robot_pos_y-1);
                    return_to_base();
                    return;
                }
                move_one_cell_forward();
                rotate_to(WEST);
            } else { // direction == -1

                printf("Turning left to move down\n");
                rotate_to(SOUTH);
                if(check_obstacle_ahead())
                {
                    printf("Obstacle detected at cell (%d, %d)\n", robot_pos_x, robot_pos_y);
                    map_led_location(robot_pos_x-1,robot_pos_y);
                    return_to_base();
                    return;
                }
                
                move_one_cell_forward();
                rotate_to(EAST);
            }
            
            direction *= -1;
        }
    }
    printf("Exploration completed. Returning to base...\n");
    return_to_base();
}