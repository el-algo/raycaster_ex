/*******************************************************************************************
*
*   raylib game template
*
*   <Game title>
*   <Game description>
*
*   This game has been created using raylib (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2021 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "screens.h"    // NOTE: Declares global (extern) variables and screens functions

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Shared Variables Definition (global)
// NOTE: Those variables are shared between modules through screens.h
//----------------------------------------------------------------------------------
GameScreen currentScreen = LOGO;
Font font = { 0 };
Music music = { 0 };
Sound fxCoin = { 0 };

#include "raylib.h"

#include <math.h>       // Required for: sinf(), cosf()
#include <raymath.h>
#include <stdio.h>

#define mapHeight 14 * 10
#define mapWidth 18 * 10
#define max_room_num 100

float sint(float ang)
{
    return -sinf(2 * PI * ang);
}

float cost(float ang)
{
    return cosf(2 * PI * ang);
}

float atan2t(float x, float y)
{
    float temp_ang = (-atan2(y, x) / (2 * PI));
    float ang = (temp_ang < 0) ? temp_ang += 1.0 : temp_ang;
    return ang;
}

//Returns the sign of the number
int sgn(float num)
{
    return (num >= 0) ? 1 : -1;
}

float abst(float num)
{
    return (num >= 0) ? num : -num;
}


int map[mapHeight][mapWidth];
int floor_map[100] = { 0 };
int starting_room_element = 0;
int end_room_element = 0;

void make_rectangle(int x, int y, int width, int height, int type)
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            map[y + i][x + j] = type;
        }
    }
}


int kthBit(unsigned int n, unsigned int k)
{
    return (n >> (k - 1)) % 2;
}


void make_corridor(int x, int y, int blueprint)
{
    int right = kthBit(blueprint, 1);
    int down = kthBit(blueprint, 2);
    int left = kthBit(blueprint, 3);
    int up = kthBit(blueprint, 4);

    int dirs[] = { right, down, left, up };
    int x_pos = 0;
    int y_pos = 0;

    for (int i = 0; i < 4; i++)
    {
        if (dirs[i] == 1)
        {
            switch (i)
            {
            case 0:
                x_pos = 16;
                y_pos = 6;
                break;
            case 1:
                x_pos = 8;
                y_pos = 12;
                break;
            case 2:
                x_pos = 0;
                y_pos = 6;
                break;
            case 3:
                x_pos = 8;
                y_pos = 0;
                break;
            default:
                break;
            }
            make_rectangle(x + x_pos, y + y_pos, 2, 2, 0);
        }
    }
}


void make_room(int x, int y, int blueprint)
{
    // Fill cell with walls
    make_rectangle(x, y, 18, 14, 1);
    // Only leave the walls in the perimeter
    make_rectangle(x + 1, y + 1, 16, 12, 0);

    make_corridor(x, y, blueprint);
}


int map2floor_map(int x, int y)
{
    return (int)(x / 18) + 10 * (int)(y / 14);
}


void generate_map(int cols, int rows)
{
    for (int i = 0; i < 100; i++)
    {
        if (floor_map[i] != 0)
        {
            int index_units = (i % 10) * 18;
            int index_tens = (i / 10) * 14;

            map[index_tens][index_units] = floor_map[i];
        }
    }

    // Only loop over the corners of each room cell
    for (int i = 0; i < cols; i += 14)
    {
        for (int j = 0; j < rows; j += 18)
        {
            if (map[i][j] > 0)
            {
                make_room(j, i, map[i][j]);
            }
        }
    }
}


void empty_map()
{
    for (int i = 0; i < mapHeight; i++)
    {
        for (int j = 0; j < mapWidth; j++)
        {
            map[i][j] = 0;
        }
    }
}


int get_neighbors(int arr[], int index)
{
    int right = index + 1;
    int down = index + 10;
    int left = index - 1;
    int up = index - 10;
    int neighbors = 0;

    int index_tens = index / 10;
    // Right Margin
    if (right / 10 == index_tens)
    {
        if (arr[right] != 0) { neighbors++; }
    }
    // Down Margin
    if (down <= 99)
    {
        if (arr[down] != 0) { neighbors++; }
    }
    // Left Margin
    if (left / 10 == index_tens)
    {
        if (arr[left] != 0) { neighbors++; }
    }
    // Up Margin
    if (up >= 0)
    {
        if (arr[up] != 0) { neighbors++; }
    }

    return neighbors;
}


bool is_valid_neighbor_index(int index, int dir)
{
    int neighbor_index = index + dir;
    int index_units = index % 10;
    int index_tens = index / 10;

    //printf("    INDEX: %i\n", (index));
    //printf("    DIR: %i\n", (dir));
    //printf("    NEIGHBOR_INDEX: %i\n", (neighbor_index));

    // Out of array range
    if (neighbor_index < 0 || neighbor_index > 100)
    {
        return false;
    }

    // Left and right
    if (dir == 1 || dir == -1)
    {
        if (neighbor_index / 10 == index_tens)
        {
            return true;
        }
    }

    // Up and down
    if (dir == 10 || dir == -10)
    {
        if (neighbor_index % 10 == index_units)
        {
            return true;
        }
    }

    return false;
}


void print_floor(int arr[])
{
    for (int i = 0; i < 100; i++)
    {
        if (arr[i] == 0)
        {
            printf(". ");
        }
        else
        {
            printf("%i ", arr[i]);
        }
        if ((i + 1) % 10 == 0)
        {
            printf("\n");
        }
    }
    printf("\n");
}


int choose_random_direction()
{
    int dir = 0;
    switch (GetRandomValue(0, 3))
    {
    case 0: dir = 1;   break;
    case 1: dir = 10;  break;
    case 2: dir = -1;  break;
    case 3: dir = -10; break;
    default: dir = 0;  break;
    }
    return dir;
}


unsigned int binary_dir(int i)
{
    int bin = 0;
    switch (i)
    {
    case 1:   bin = 0b0001; break;
    case 10:  bin = 0b0010; break;
    case -1:  bin = 0b0100; break;
    case -10: bin = 0b1000; break;
    default:  bin = 0b0000; break;
    }
    return bin;
}


int opposite_dir(int og_dir)
{
    return og_dir * -1;
}


bool is_cell_empty(int arr)
{
    return (arr == 0) ? true : false;
}


// Random 50/50 chance
bool coin_flip()
{
    return (GetRandomValue(0, 1) == 0) ? true : false;
}


int first_non_empty_index(int arr[])
{
    int index = 0;
    for (int i = 0; i <= 100; i++)
    {
        if (!is_cell_empty(arr[i]))
        {
            index = i;
        }
    }
    return index;
}


int generate_floor(int arr[], int room_num)
{
    int current_cell = 55;
    int neighbor_cell = 0;
    int rooms_built = 1;
    int room_location[max_room_num] = {0};

    room_location[0] = current_cell;

    int room_queue = 0;
    bool built = false;

    while (rooms_built < room_num)
    {
        for (int i = 0; i < rooms_built; i++)
        {
            current_cell = room_location[i];
            //printf("%i\n", current_cell);
            int dir = choose_random_direction();
            neighbor_cell = current_cell + dir;
            bool cell_empty = is_cell_empty(arr[neighbor_cell]);
            
            room_queue++;
            if (room_queue == 16)
            {
                starting_room_element = current_cell;
                //printf("START ROOM: %i\n", starting_room_element);
            }
            
            if (!built && cell_empty && get_neighbors(arr, neighbor_cell) < 2 && is_valid_neighbor_index(current_cell, dir) && coin_flip())
            {
                //printf("OG ROOM: %i\n", (room_location[i]));
                //printf("DIR: %i\n", (dir));
                //printf("NEIGHBOR: %i\n", (neighbor_cell
                //printf("    DIR: %i -> ", dir);
                //printf("%i\n", binary_dir(dir));

                arr[current_cell] += binary_dir(dir);
                arr[neighbor_cell] = binary_dir(opposite_dir(dir));
                room_location[rooms_built] = neighbor_cell;
                built = true;
            }
        }

        // Do it after traversing the full array of rooms
        if (built)
        {
            built = false;
            rooms_built++;

            //print_floor(arr);
            //printf("BUILT: %i\n", rooms_built);
            //printf("QUEUE: %i\n", room_queue);
        }

    }
    //printf("POSSIBLE ROOMS: %i", room_queue);
    end_room_element = current_cell;
    //printf("END ROOM: %i", end_room_element);
    return 0;
}


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 400 * 3;
    const int screenHeight = 240 * 3;

    int halfScreenWidth = screenWidth / 2;
    int halfScreenHeight = screenHeight / 2;

    InitWindow(screenWidth, screenHeight, "Raycaster Test");

    Texture2D wall_texture = LoadTexture("resources/generic_wall_texture.png");        // Texture loading

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    //unsigned int bina = 0b0001;
    //printf("%u\n", bina + 0b1110);

    SetRandomSeed(16);

    generate_floor(floor_map, 8);
    generate_map(mapHeight, mapWidth);

    int scale_a = 4;

    // Player position
    float px = (starting_room_element % 10) * 18 + 9;
    float py = (starting_room_element / 10) * 14 + 7;

    float ex = (end_room_element % 10) * 18 + 1;
    float ey = (end_room_element / 10) * 14 + 1;

    float step_x = 0;
    float step_y = 0;

    // Player angle (in revolutions)
    float pa = 0.25;
    SetMousePosition(halfScreenWidth, halfScreenHeight);

    // Player speeds
    float rot_speed = 0.2;
    float move_speed = 0.08;

    // Ray variables
    float x = 0;
    float y = 0;
    float vx = 0;
    float vy = 0;
    float ox = 0;
    float oy = 0;
    float dx = 0;
    float dy = 0;
    float a = 0;
    int ix = 0;
    int iy = 0;

    // Wall height. This value makes the walls into cubes.
    int h = screenWidth / 4;
    float d = 0.0;

    float first_ray[2] = { 0, 0 };
    bool dev = !false;
    

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        HideCursor();

        step_x = 0;
        step_y = 0;

        //Change player angle based on mouse position
        float x_diff = halfScreenWidth - GetMousePosition().x;
        SetMousePosition(halfScreenWidth, halfScreenHeight);
        x_diff /= screenWidth;
        pa += x_diff / 5.0f;

        if (IsKeyPressed(KEY_R))
        {
            for (int i = 0; i < 100; i++)
            {
                floor_map[i] = 0;
            }
            empty_map();
            generate_floor(floor_map, 8);
            print_floor(floor_map);
            generate_map(mapHeight, mapWidth);

            px = (starting_room_element % 10) * 18 + 9;
            py = (starting_room_element / 10) * 14 + 7;

            ex = (end_room_element % 10) * 18 + 1;
            ey = (end_room_element / 10) * 14 + 1;

            dev = false;
        }

        if (IsKeyDown(KEY_Q))
        {
            pa += rot_speed * GetFrameTime();
        }

        if (IsKeyDown(KEY_E))
        {
            pa -= rot_speed * GetFrameTime();
        }


        // Control player position
        if (IsKeyDown(KEY_A))
        {
            step_x = cost(0.25 + pa) * move_speed;
            step_y = sint(0.25 + pa) * move_speed;
        }

        if (IsKeyDown(KEY_D))
        {
            step_x = cost(pa - 0.25) * move_speed;
            step_y = sint(pa - 0.25) * move_speed;
        }

        if (IsKeyDown(KEY_W))
        {
            step_y += sint(pa) * move_speed;
            step_x += cost(pa) * move_speed;
        }

        if (IsKeyDown(KEY_S))
        {
            step_x += -cost(pa) * move_speed;
            step_y += -sint(pa) * move_speed;
        }


        if (IsKeyPressed(KEY_TAB))
        {
            dev = !dev;
        }

        if (IsKeyPressed(KEY_R))
        {
            dev = !dev;
        }

        px += step_x;
        py += step_y;

        /*
        if(map_collision(floor(step_x + x), floor(step_y + y), map))
        {
            px += step_x;
            py += step_y;
        }
        else if(map_collision(floor(step_x + x), floor(y), map))
        {
            px += step_x;
        }
        else if(map_collision(floor(x), floor(step_y + y), map))
        {
            py += step_y;
        }
        */

        // Limit angle to a single revolution
        if (pa <= 0.0) pa += 1.0;
        if (pa > 1.0) pa -= 1.0;


        BeginDrawing();
        ClearBackground(BLACK);

        DrawRectangle(0, 0, screenWidth, halfScreenHeight, DARKGRAY);
        DrawRectangle(0, halfScreenHeight, screenWidth, halfScreenHeight, GRAY);

        // for each screen column   
        for (int i = 0; i <= screenWidth; i++)
        {
            // find starting tile            
            x = px;
            y = py;

            // find ray direction
            a = atan2t(1, ((float)i - halfScreenWidth) / halfScreenWidth);
            vx = cost(pa + a);
            vy = sint(pa + a);

            // find standard distance
            dx = abst(1 / vx);
            dy = abst(1 / vy);

            // find increment value
            ix = (vx > 0) ? 1 : -1;
            iy = (vy > 0) ? 1 : -1;

            // ray distance
            d = 0;

            // find initial offset
            if (vx > 0)
            {
                ox = (floor(x) - x + 1) / vx;
            }
            else
            {
                ox = abst((x - floor(x)) / vx);
            }
            if (vy > 0)
            {
                oy = (floor(y) - y + 1) / vy;
            }
            else
            {
                oy = abst((y - floor(y)) / vy);
            }

            bool collFound = false;
            bool hitIsHorizontal;
            while (!collFound)
            {

                // horizontal intersection
                if (ox < oy)
                {
                    x += ix;
                    d = ox;
                    ox += dx;

                    hitIsHorizontal = false;
                }
                else
                {
                    y += iy;
                    d = oy;
                    oy += dy;

                    hitIsHorizontal = true;
                }

                // If ray hits a wall or is out of bounds
                if (map[(int)floor(y)][(int)floor(x)] == 1 || x <= 0 || x > mapWidth || y <= 0 || y > mapHeight)
                {
                    if (i == 0) { first_ray[0] = x; first_ray[1] = y; }
                    // Correct fish-eye effect
                    //d *= cost(a);

                    Color color = { 255 - d * 4, 255 - d * 4, 255 - d * 4, 255 };
                    //Color color = { 0, 150, 0, 255 };

                    if (d >= 64.0)
                    {
                        color = (Color){ 0, 0, 0, 255 };
                    }

                    if (!hitIsHorizontal)   
                    {
                        color = (Color){ color.r / 2, color.g / 2, color.b / 2, 255 };

                    }

                    int lineHeight = h / d / cost(a);
                    int drawStart = floor(halfScreenHeight - lineHeight);
                    int drawEnd = floor(halfScreenHeight + lineHeight);
                    int offset = 0;

                    //DrawLine(i, drawStart, i, drawEnd, color);
                    if (hitIsHorizontal)
                    {
                        offset = fmod((px + vx * d), 1) * 64;
                    }
                    else
                    {
                        offset = fmod((py + vy * d), 1) * 64;
                    }

                    Vector2 fff = { 0.0f, 0.0f };
                    Rectangle vvv = { offset, 0, 1, (float)wall_texture.height};
                    Rectangle www = { i, drawStart, 1, 2 * lineHeight};
                    DrawTexturePro(wall_texture, vvv, www, fff, 0.0f, color);

                    collFound = true;
                    break;
                }
            }
        }


        if (dev)
        {

            for (int i = 0; i < mapHeight; i++)
            {
                for (int j = 0; j < mapWidth; j++)
                {
                    Color wallColor;
                    switch (map[i][j])
                    {
                    case -1: wallColor = PINK; break;
                    case 0: wallColor = (Color){ 0, 0, 0, 0 }; break;
                    default: wallColor = GREEN; break;
                        /*
                        case 1: wallColor = DARKBLUE; break;
                        case 2: wallColor = DARKGREEN; break;
                        case 3: wallColor = RED; break;
                        case 4: wallColor = BLUE; break;
                        case 5: wallColor = DARKBLUE; break;
                        */
                    }

                    DrawRectangle(j * scale_a, i * scale_a, scale_a, scale_a, wallColor);
                }

            }


            DrawText(TextFormat("%f", px), 0, screenHeight - 40, 20, BLACK);
            DrawText(TextFormat("%f", py), 0, screenHeight - 20, 20, BLACK);
            DrawText(TextFormat("ANG: %f", pa), 0, screenHeight - 60, 20, BLACK);
            
            DrawText(TextFormat("X: %f", x), halfScreenWidth, screenHeight - 60, 20, BLACK);
            DrawText(TextFormat("D: %f", d), halfScreenWidth + 200, screenHeight - 60, 20, BLACK);
            DrawText(TextFormat("VX: %f", vx), halfScreenWidth + 200, screenHeight - 40, 20, BLACK);
            DrawText(TextFormat("Y: %f", y), halfScreenWidth, screenHeight - 40, 20, BLACK);

            DrawRectangle(ex * scale_a, ey * scale_a, 16 * scale_a, 12 * scale_a, RED);
            DrawRectangle(floor(px) * scale_a, floor(py) * scale_a, scale_a, scale_a, BLACK);
            DrawCircle(px * scale_a, py * scale_a, scale_a / 2, RED);
            DrawLine(px * scale_a, py * scale_a, px * scale_a + cost(pa) * scale_a, py * scale_a + sint(pa) * scale_a, YELLOW);

            // Projects the last column of the view angle
            DrawLine(px * scale_a, py * scale_a, first_ray[0] * scale_a, first_ray[1] * scale_a, WHITE);
            DrawLine(px * scale_a, py * scale_a, x * scale_a, y * scale_a, WHITE);

            DrawFPS(0, GetScreenHeight() - 80);
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(wall_texture);        // Texture unloading

    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
