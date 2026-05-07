#ifndef ROBOT_CONF_H
#define ROBOT_CONF_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "i2c_cmd.h" //i2c cmds


#define MAP_GRID_SIZE (5) // GRID_SIZE x GRID_SIZE
#define  CELL_CM (10)
#define SAFE_OBSTACLE_THRESHOLD (10)


#define CUTEBOT_FORWARD   (0x01)
#define CUTEBOT_BACKWARD  (0x00)



// squared map to consider.
typedef enum heading_t{
    NORTH =0,
    EAST  =1,
    SOUTH =2,
    WEST  =3
} heading_t;


// Robot pose (defined in robot_conf.c)
extern int robot_pos_x;
extern int robot_pos_y;
extern heading_t robot_heading;


typedef enum field_t{
  unknown,
  explored, 
  occuppied
}field_t;

// Global map (defined in robot_conf.c)
extern field_t global_map[MAP_GRID_SIZE][MAP_GRID_SIZE];



void map_init();
bool update_position(void);
void mark_field(int coord_x, int coord_y, field_t field );
field_t get_field(int coord_x, int coord_y);
void get_next_cell(int x, int y, heading_t dir, int* next_x, int* next_y);
void rotate_to(heading_t target);
bool check_obstacle_ahead(void);
void move_one_cell_forward(void);
void explore_grid(void);
bool is_valid_cell(int x, int y);
void move_to_next_cell_forward(void);
// Rotate vehicle to good direction
void move_one_cell_forward();
#endif // ROBOT_CONF_H