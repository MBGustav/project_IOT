#include "robot_conf.h"
#include <string.h>


int robot_pos_x = 0;
int robot_pos_y = 0;
heading_t robot_heading = NORTH;

field_t global_map[MAP_GRID_SIZE][MAP_GRID_SIZE];


void map_init();
void mark_field(int coord_x, int coord_y, field_t field );
field_t get_field(int coord_x, int coord_y);
void mark_field(int coord_x, int coord_y, field_t field );



void map_init()
{
    for(int i = 0; i < MAP_GRID_SIZE; i++){
        for(int j = 0; j < MAP_GRID_SIZE; j++){
            mark_field(i,j, unknown);
        }
    }
}


void mark_field(int coord_x, int coord_y, field_t field )
{
    //constraint
    if (coord_x < 0 || coord_x >= MAP_GRID_SIZE ) return;
    if (coord_y < 0 || coord_y >= MAP_GRID_SIZE ) return;
    
    global_map[coord_x][coord_y] = field;
}


field_t get_field(int coord_x, int coord_y)
{
    if (coord_x < 0 || coord_x >= MAP_GRID_SIZE ) return -1;
    if (coord_y < 0 || coord_y >= MAP_GRID_SIZE ) return -1;
    
    return global_map[coord_x][coord_y];
}


void get_next_cell(int x, int y, heading_t dir, int* next_x, int* next_y)
{
    *next_x = x;
    *next_y = y;
    
    switch(dir)
    {
        case NORTH:(*next_y)--; break;
        case SOUTH:(*next_y)++; break;
        case EAST: (*next_x)++; break;
        case WEST: (*next_x)--; break;
    }
}

char *heading_to_string(heading_t heading)
{
    switch(heading)
    {
        case NORTH: return "NORTH";
        case SOUTH: return "SOUTH";
        case EAST:  return "EAST";
        case WEST:  return "WEST";
        default:    return "UNKNOWN";
    }
}


// Rotate vehicle to good direction
void rotate_to(heading_t target)
{
    int diff = target - robot_heading;
    
    if(diff == 0) return;
    
    if(diff == 1 || diff == -3){
        printf("Rotating right from %d to %d\n", robot_heading, target);
        trolley_steering(CB_TURN_RIGHT_IN_PLACE, 90);

    }
    
    else if(diff == -1 || diff == 3){
        printf("Rotating left from %d to %d\n", robot_heading, target);
        trolley_steering(CB_TURN_LEFT_IN_PLACE, 90);
    }
    
    else
    {
        printf("Rotating 180 degrees from %d to %d\n", robot_heading, target);
        trolley_steering(CB_TURN_LEFT_IN_PLACE, 180);
    }
    
    robot_heading = target;
}

bool update_position(void)
{
    int next_x = robot_pos_x;
    int next_y = robot_pos_y;

    switch(robot_heading)
    {
        case NORTH: next_y--; break;
        case SOUTH: next_y++; break;
        case EAST:  next_x++; break;
        case WEST:  next_x--; break;
    }

    
    if(!is_valid_cell(next_x, next_y))
    {
        printf("Blocked movement outside grid: (%d,%d)\n", next_x, next_y);
        return false;
    }

    robot_pos_x = next_x;
    robot_pos_y = next_y;

    printf("Robot (x,y, heading): (%d, %d, %s)\n", robot_pos_x, robot_pos_y, heading_to_string(robot_heading));

    mark_field(robot_pos_x, robot_pos_y, explored);

    return true;
}


void move_one_cell_forward(void)
{
    printf("Robot (x,y, heading): (%d, %d, %s)\n", robot_pos_x, robot_pos_y, heading_to_string(robot_heading));
    rotate_to(robot_heading);

    move_distance(CUTEBOT_FORWARD, CELL_CM);

    if(!update_position())
    {
        printf("Movement blocked\n");
    }
}

bool is_valid_cell(int x, int y)
{
    if(x < 0 || x >= MAP_GRID_SIZE) return false;
    if(y < 0 || y >= MAP_GRID_SIZE) return false;
    
    return true;
}


bool check_obstacle_ahead(void)
{
    int next_x, next_y;
    
    int dist = (int)get_sonar_distance_cm();
    printf("Distance: %d cm\n", dist);
    get_next_cell(
        robot_pos_x,
        robot_pos_y,
        robot_heading,
        &next_x,
        &next_y
    );
    
    // Check if we consider this grid or the next one as occupied
    
    
    if(dist <= SAFE_OBSTACLE_THRESHOLD)
    {
        if(is_valid_cell(next_x, next_y))
        {
            mark_field(next_x, next_y, occuppied);
        }else {
            printf("Obstacle detected outside of map boundaries at cell (%d, %d).\n", next_x, next_y);
        }
        
        return true;
    }
    
      
    return false;
}
    
    
    void return_to_base()
    {
        printf("Returning to base...\n");
        rotate_to(NORTH);
        while(robot_pos_y > 0){
            printf("current pos: (%d, %d)\n", robot_pos_x, robot_pos_y);
            move_one_cell_forward();
            delay_ms(100);
        }
        move_one_cell_forward();
        

        rotate_to(WEST);
        while(robot_pos_x > 0){
            printf("current pos: (%d, %d)\n", robot_pos_x, robot_pos_y);
            move_one_cell_forward();
        }
        move_one_cell_forward();
            
        printf("Arrived at base (0,0)\n");
        rotate_to(WEST);
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
                // printf("Moving to cell (%d, %d)\n", robot_pos_x, robot_pos_y);
                if(check_obstacle_ahead())
                {
                    // printf("Obstacle detected at cell (%d, %d). \n\
                            // Stopping exploration.\n", robot_pos_x, robot_pos_y);
                            
                    map_led_location(robot_pos_x,robot_pos_y);
                    return_to_base();
                    return;
                }
                
                
                // printf("Cell (%d, %d) is clear.\n", robot_pos_x, robot_pos_y);
                move_one_cell_forward();
            }
            
            // muda linha
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
                }
                else
                {
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