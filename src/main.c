#include <math.h>

#include "resource_dir.h"	
#include "raylib.h"
#include "raymath.h"

#include "Guy.c"
//#include "Environment.c"

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

#define WIN_WIDTH			1280
#define WIN_HEIGHT			800

//Tile Sprite
#define TILE_WIDTH			256
#define TILE_HEIGHT			128
#define TILE_FULL_HEIGHT 	512
#define TILE_SCALE			0.5

#define FLOOR_TILE_SIZE		
#define FLOOR_WIDTH 		10
#define FLOOR_HEIGHT 		10

#define CAM_SPEED 			10

#define NUM_GUYS			40

#define TARGET_FPS			60


//======================================================================




//======================================================================
// Converts 2D coords to isometric coords 
Vector2 twoDtoIso(Vector2 in_pos)
{
    Vector2 iso_point;
    iso_point.x =  in_pos.x - in_pos.y;
    iso_point.y = (in_pos.x + in_pos.y) / 2.0f;
    return iso_point;
}

// Draws isometric tiles
void draw_tile_floor(Texture tile_texture)
{

	for(int i = 0; i < FLOOR_WIDTH; i++){
		for(int j = 0; j < FLOOR_HEIGHT; j++){
			Vector2 tile_pos = (Vector2){
				(float)i * (TILE_WIDTH * TILE_SCALE) / 2,
				(float)j * (TILE_HEIGHT * TILE_SCALE)
			};
			tile_pos = twoDtoIso(tile_pos);

			// Construct the source rect
			Rectangle frame_rec = (Rectangle){ 
				0.0f, 0.0f,
				TILE_WIDTH,  TILE_FULL_HEIGHT 
			};

			// Construct the dest rect for scaling
			Rectangle scaled_rec = (Rectangle){ 
				tile_pos.x, tile_pos.y,
				TILE_WIDTH * TILE_SCALE,  TILE_FULL_HEIGHT * TILE_SCALE
			};

			// Draw final texture rect
			DrawTexturePro(
					tile_texture,
					frame_rec,
					scaled_rec,
					(Vector2){
						TILE_WIDTH * TILE_SCALE / 2.0f,
						(TILE_FULL_HEIGHT - TILE_HEIGHT) * TILE_SCALE
					},
					0.0f,
					WHITE
			);
		}
	}
}

//======================================================================


int main()
{


	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	InitWindow(WIN_WIDTH, WIN_HEIGHT, "lil_world");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");
	Texture tile_texture = LoadTexture("tile.png");
	Texture guy_texture = LoadTexture("spritesheet.png");
	Color colors[] = {YELLOW, ORANGE, RED, GREEN, BLUE};

	// Camera stuff
	Camera2D camera = 	{ 0 };
    camera.target	= 	(Vector2){ 0.0, 0.0 };
    camera.offset 	= 	(Vector2){ WIN_WIDTH/2.0f, WIN_HEIGHT/2.0f };
    camera.rotation	= 	0.0f;
    camera.zoom		= 	1.0f;

    // Make guys
    Guy guys[NUM_GUYS];
    for(int i = 0; i < NUM_GUYS; i++){
    	guys[i].position 	= 	(Vector2){
    		GetRandomValue(-100, 100),
    		GetRandomValue(0, 100)
    	};
    	guys[i].velocity	=	(Vector2){0.0f};
    	guys[i].frame_rec 	= 	(Rectangle){ 0.0f, 0.0f, SPRITE_WIDTH,  SPRITE_HEIGHT };
    	guys[i].scaled_rec 	= 	(Rectangle){ 0.0f, 0.0f, SPRITE_WIDTH * SPRITE_SCALE,  SPRITE_HEIGHT * SPRITE_SCALE};
    	guys[i].cur_frame 	= 	0;
    	guys[i].texture 	= 	guy_texture;
    	guys[i].state 		= 	WALK;
    	guys[i].heading 	=	SE;
    	guys[i].color 		=	colors[GetRandomValue(0, 5)];
    }

    // Animation frames information
    int framesCounter = 0;
    int framesSpeed = 15;
    SetTargetFPS(TARGET_FPS); 

    //======================================================================

	// game loop
	while (!WindowShouldClose()){

		float dt = GetFrameTime();

		framesCounter++;

        if(framesCounter >= (TARGET_FPS/framesSpeed)){
            framesCounter = 0;

            guys_update_ani(&guys, NUM_GUYS);
        }

        // Manual camera controls
        /*
		if(IsKeyDown(KEY_RIGHT)) 	camera.target.x += CAM_SPEED;
        if(IsKeyDown(KEY_LEFT)) 	camera.target.x -= CAM_SPEED;
        if(IsKeyDown(KEY_DOWN)) 	camera.target.y += CAM_SPEED;
        if(IsKeyDown(KEY_UP)) 		camera.target.y -= CAM_SPEED;
        */
        camera.target = guys[0].position;

        camera.zoom += GetMouseWheelMove() * 0.1f;
        if (camera.zoom < 0.1f) camera.zoom = 0.1f;

        //==================================================================
		BeginDrawing();

		ClearBackground(BLUE);

		BeginMode2D(camera);

			// Draw Tiles
			draw_tile_floor(tile_texture);

			player_move(&guys[0], dt);

			Vector2 global_mous_pos = GetScreenToWorld2D(GetMousePosition(), camera);

			guys_follow(&guys, NUM_GUYS, global_mous_pos, dt);
			//guys_follow(&guys, NUM_GUYS, guys[0].position, dt);

			printf("%f , %f\n", guys[0].position.x, guys[0].position.y);

			for(int i = 0; i < NUM_GUYS; i++){
				Guy* y_sorted[NUM_GUYS];
				
				y_sort_guys(guys, y_sorted, NUM_GUYS);

				DrawTexturePro(
					y_sorted[i]->texture,
					y_sorted[i]->frame_rec,
					y_sorted[i]->scaled_rec,
					(Vector2){SPRITE_WIDTH* SPRITE_SCALE / 2.0f,
					SPRITE_HEIGHT * SPRITE_SCALE/ 2.0f},
					0.0f,
					y_sorted[i]->color
				);
			}
			

		EndMode2D();
		

		EndDrawing();
		//==================================================================
	}

	//======================================================================


	// cleanup
	// unload our texture so it can be cleaned up
	UnloadTexture(tile_texture);
	UnloadTexture(guy_texture);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
