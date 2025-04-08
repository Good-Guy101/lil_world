#include <math.h>
#include <stdbool.h>

#include "raylib.h"
#include "raymath.h"

#define GUY_SPEED 			50
#define GUY_SEP_RAD			150.0f
#define GUY_SEP_STR			0.2f
#define FOLLOW_DIST			60
#define GUY_BND_STR			1.0f

#define SPRITE_WIDTH		16.0f
#define SPRITE_HEIGHT		24.0f
#define SPRITE_SCALE		2.0f

#define TAN_PI_DIV_8 0.414213562 

// ISOMETIC CORDINATE DIRECTIONS
/*
NW	->	Top				0
N 	-> 	Top-Right		1
NE 	->	Right			2
E	->	Bottom-Right	3
SE	->	Bottom 			4
S	->	Bottom-Left 	5
SW	->	Left 			6
W	->	Top-Left 		7
*/

typedef enum Direction {
	NW,
	N,
	NE,
	E,
	SE,
	S,
	SW,
	W
}Direction;

typedef enum Guy_State {
	IDLE,
	WALK,
}Guy_State;

typedef struct Guy{
	Vector2 position;
	Vector2 velocity;
    Rectangle frame_rec;
    Rectangle scaled_rec;
    int cur_frame;
    Texture texture;
    Color color;

    enum Guy_State state;
    enum Direction heading;
}Guy;

// Gets the cardinal direction start to end points to
Direction get_dir(Vector2 start, Vector2 end) 
{
	double dx = end.x - start.x;
    double dy = end.y - start.y;

    if (fabs(dx) > fabs(dy)) {
        // Predominantly horizontal movement
        if (fabs(dy / dx) <= TAN_PI_DIV_8) 
            return dx > 0 ? NE : SW;     

        if (dx > 0) 
            return dy > 0 ? E : N;
        else
            return dy > 0 ? S : W;
    }

    if (fabs(dy) > 0) {
        // Predominantly vertical movement
        if (fabs(dx / dy) <= TAN_PI_DIV_8) 
            return dy > 0 ? SE : NW;
        
        if (dy > 0) 
            return dx > 0 ? E : S;
        else 
            return dx > 0 ? N : W;
    }

    return SE; 
}

// Sorts guys according to y-positions, outputs to pointer array
void y_sort_guys(Guy* in_guys, Guy** out_guys, int num_guys)
{
	for(int i = 0; i < num_guys; i++){
		out_guys[i] = &in_guys[i];
	}

	for(int i = 1; i < num_guys; i++){
		for(int j = i; j > 0 && out_guys[j-1]->position.y > out_guys[j]->position.y; j--){
			Guy* swap = out_guys[j];
			out_guys[j] = out_guys[j-1];
			out_guys[j-1] = swap;
        }
    }
}


// Updates the animation frames acorrding to state
void guys_update_ani(Guy* guys, int num_guys)
{

	for(int i = 0; i < num_guys; i++){

		if(Vector2Length(guys[i].velocity) > 0.1)
			guys[i].heading = get_dir(guys[i].position, Vector2Add(guys[i].position, guys[i].velocity));
		

		// Update the dest rect
		guys[i].scaled_rec.x = guys[i].position.x;
		guys[i].scaled_rec.y = guys[i].position.y;
		
		if(guys[i].state == WALK){
			guys[i].cur_frame++;

			if (guys[i].cur_frame > 2) guys[i].cur_frame = 0;
			else if(guys[i].cur_frame == 1) guys[i].cur_frame = 2;

			guys[i].frame_rec.x = (float)guys[i].heading * SPRITE_WIDTH;
			guys[i].frame_rec.y = (float)guys[i].cur_frame * SPRITE_HEIGHT;

			
		} else if(guys[i].state == IDLE){
			guys[i].cur_frame = 0;
			guys[i].frame_rec.x = (float)guys[i].heading * SPRITE_WIDTH;
			guys[i].frame_rec.y = SPRITE_HEIGHT;
		}
	}

}

// Gets separation force for a guy in the current array of guys
Vector2 get_separation(Guy* guys, int num_guys, int cur_guy)
{
	Vector2 separation = {0};
    
    for(int i = 0; i < num_guys; i++) {
        if (i == cur_guy) continue;

        if (Vector2DistanceSqr(guys[cur_guy].position, guys[i].position) > GUY_SEP_RAD * GUY_SEP_RAD) 
    		continue;
        
        float distance = Vector2Distance(guys[cur_guy].position, guys[i].position);
        
        if (distance < GUY_SEP_RAD) {
            // Calculate vector away from the nearby guy
            Vector2 away_vector = Vector2Subtract(guys[cur_guy].position, guys[i].position);
            
            // Normalize and scale by inverse of distance
            float scale = (GUY_SEP_RAD - distance) / GUY_SEP_RAD;
            away_vector = Vector2Scale(Vector2Normalize(away_vector), scale * GUY_SEP_STR);
            
            separation = Vector2Add(separation, away_vector);
        }
    }
    
    return separation;
}

// Update guys to follow the target position
void guys_follow(Guy* guys, int num_guys, Vector2 target, float dt)
{
	for (int i = 1; i < num_guys; i++){
		Vector2 separation = get_separation(guys, num_guys, i);

		guys[i].heading = get_dir(guys[i].position, target);

		Vector2 target_dir = Vector2Subtract(target, guys[i].position);
		float target_dist = Vector2Length(target_dir);
		target_dir = Vector2Normalize(target_dir);
		
		if(target_dist < FOLLOW_DIST) {
			guys[i].state = IDLE;

			Vector2 movement = Vector2Scale(separation, GUY_SEP_STR);

			guys[i].velocity = Vector2Scale(separation, dt);
		} else {
			guys[i].state = WALK;

			guys[i].velocity = Vector2Add(separation, target_dir);
	
			if(Vector2Length(guys[i].velocity) > 0.01)
				guys[i].velocity = Vector2Scale(Vector2Normalize(guys[i].velocity), (float)GUY_SPEED);
			else
				guys[i].state = IDLE;


			guys[i].velocity = Vector2Scale(guys[i].velocity, dt);
		}
		
		// bounds checking
		// clean up later...
		if(!((float)fabsf(guys[i].position.x / 2) < (float)guys[i].position.y && 
			(float)guys[i].position.y < -(float)fabsf(guys[i].position.x / 2) + (float)((float)128 * 0.5f * (float)10))){
			guys[i].velocity = Vector2Subtract((Vector2){0, 320}, guys[i].position);
			guys[i].velocity = Vector2Normalize(guys[i].velocity);
			guys[i].velocity = Vector2Scale(guys[i].velocity, 2.0f);
		}

		

		guys[i].position = Vector2Add(guys[i].position, guys[i].velocity);
	}
}

// Allows input guy to be controlled by the user
void player_move(Guy* player, float dt)
{
	Vector2 velocity = {0.0f};

	if (IsKeyDown(KEY_RIGHT)) 	velocity.x += 1.0f;
    if (IsKeyDown(KEY_LEFT)) 	velocity.x -= 1.0f;
    if (IsKeyDown(KEY_DOWN)) 	velocity.y += 1.0f;
    if (IsKeyDown(KEY_UP)) 		velocity.y -= 1.0f;

    player->heading = get_dir(player->position, Vector2Add(player->position, velocity));

    velocity = Vector2Normalize(velocity);
    velocity = Vector2Scale(velocity, GUY_SPEED);
    velocity = Vector2Scale(velocity, dt);

    if(Vector2Length(velocity) > 0)
    	player->state = WALK;
    else
    	player->state = IDLE;

    player->position = Vector2Add(player->position, velocity);
}
