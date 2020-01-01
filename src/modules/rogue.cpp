/* 
 * TODO
 * 
 * Long Term:
 *  Items
 *  Enemies
 *    https://www.youtube.com/watch?v=icZj67PTFhc ~14:30
 *  Attacking
 *  SC Key option menu
 * 
 */

#include "../modules.hpp"

#include <cstdio>
#include <vector>
#include <list>

namespace Modules {

/**
 * Constants
 */

constexpr unsigned MAX_NEIGHBORS = 4; // Don't touch
constexpr int GRAPH_SIZE = 3;
constexpr int MAP_SIZE = 60;    // ~10x MAP_GRID is good

// the number of tiles square of each room
constexpr int ROOM_WIDTH = MAP_SIZE / GRAPH_SIZE - 1;
constexpr int ROOM_TOLERANCE = ROOM_WIDTH / 2;
constexpr int ROOM_CONNECT_TRIES = 4;
#define ROOM_PATH_MODIFIER 2 / 3 /* INTENDS TO HAVE OPERATOR PRECEDENCE MAKE LHS RVALUE GREATER THAN RHS */
constexpr float ROOM_GONE_CHANCE = 0.05;

constexpr int TILE_SCALING = 60; // tile size modifier on SDL window
constexpr int TILE_WIDTH = TILE_SCALING / 7;

constexpr int ENTITY_MAX = 40; // maximum number of entities
constexpr int FLOORS_MAX = 20; // maximum number of floors

/**
 * Definitions
 */

enum MapTile {
    WALL = '#',
    FLOOR = '.',
    EMPTY = ' ',
};

enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT,
};

enum EntityId {
    ID_INVALID = -1,
    ID_PLAYER,
    ID_STAIR_DOWN,
    ID_STAIR_UP,
};

struct Node {
    bool obstacle = false;
    bool visited = false;
    float global_goal;
    float local_goal;
    int x, y;
    std::vector<Node *> neighbors;
    Node *parent;

    static bool cmp(Node *a, Node *b) {
        return a->global_goal < b->global_goal;
    }
};

// graph node
struct Room {
    bool is_connected = false;
    bool is_explored = false;
    int index = 0;
    int neighbors[MAX_NEIGHBORS] = { 0 };

    void insert_neighbor(int neighbor) {
        neighbors[index++] = neighbor;
        is_connected = true;
    }
    // check if the given neighbor is already connected to
    bool check_neighbor(int neighbor) {
        for (int i = 0; i < index; ++i) {
            if (neighbors[i] == neighbor)
                return true;
        }
        return false;
    }
    void print() {
        for (int i = 0; i < index; ++i) {
            switch (neighbors[i]) {
                case UP:    printf("Up ");    break;
                case RIGHT: printf("Right "); break;
                case DOWN:  printf("Down ");  break;
                case LEFT:  printf("Left ");  break;
            }
        }
    }
};

struct Floor {
    Room Graph[GRAPH_SIZE][GRAPH_SIZE]; // graph nodes to generate a map from
    int Map[MAP_SIZE][MAP_SIZE]; // floor plan of every tile on that floor
    int Start_i, Start_j, End_i, End_j; // graph locations of starting (spawn) and ending (stair) rooms
    bool visited = false;
};

int map_to_graph_index(int index);
int graph_to_map_index(int index);

Floor Dungeon[FLOORS_MAX];
int FloorLevel = 0;
int LastStairDirection = UP;

#define FLR Dungeon[FloorLevel]

struct Entity {
    int graph_x, graph_y;
    int map_x, map_y;
    int id = -1;
    int index;

    bool check_tile(int offset_x, int offset_y) {
        switch (FLR.Map[map_y + offset_y][map_x + offset_x]) {
            case FLOOR:
                return true;
            default:
                return false;
        }
    }
    // move with bounds check
    void move(int direction) {
        switch (direction) {
            case UP:    if (check_tile(0, -1)) map_y -= 1; break;
            case RIGHT: if (check_tile(1, 0))  map_x += 1; break;
            case DOWN:  if (check_tile(0, 1))  map_y += 1; break;
            case LEFT:  if (check_tile(-1, 0)) map_x -= 1; break;
            default:
                break;
            }
        graph_x = map_to_graph_index(map_x);
        graph_y = map_to_graph_index(map_y);
    }
};

/**
 * Globals
 */

pse::Context *PSE_Context;

Entity Player{};
bool PlayerCanMove = true;
Entity StairUp{}, StairDown{};
Entity *Entities[ENTITY_MAX];
int EntityIndex = 0;

Node Nodes[MAP_SIZE][MAP_SIZE];

/**
 * Debug
 */

void debug_print_map();
void debug_print_player();

void debug_print_map()
{
    for (int i = 0; i < MAP_SIZE; ++i) {
        printf("\n");
        for (int j = 0; j < MAP_SIZE; ++j) {
            printf("%c", (char)FLR.Map[i][j]);
        }
    }
    printf("\n");
}

void debug_print_player()
{
    printf("(%d, %d) -> (%d, %d)\n", Player.graph_x, Player.graph_y, Player.map_x, Player.map_y);
}

/**
 * Util
 */

int map_to_graph_index(int index)
{
    return index / ROOM_WIDTH;
}

int graph_to_map_index(int index)
{
    return index * ROOM_WIDTH + ROOM_WIDTH / 2;
}

/**
 * Spawn entities
 */

void entity_insert(Entity& e);
void spawn_entities(); // spawn all entities onto the Map
void spawn_player(); // spawn player at center of Start_i/j
void spawn_stairs(); // spawn stairs at center of End_i/j

void astar_init();
void astar_reset();
void astar_solve(int start_i, int start_j, int end_i, int end_j);
void astar_walk(int *i, int *j);

void entity_insert(Entity& e)
{
    if (EntityIndex + 1 < ENTITY_MAX) {
        e.index = EntityIndex;
        Entities[EntityIndex++] = &e;
    }
}

void spawn_entities()
{
    // stop looking at each entity
    for (int i = 0; i < ENTITY_MAX; ++i) {
        Entities[i] = nullptr;
    }
    // reset
    EntityIndex = 0;

    // "permanent" entities
    spawn_player();
    spawn_stairs();
}

void spawn_player()
{
    // spawn player at last stair taken
    if (LastStairDirection == UP) {
        Player.map_y = graph_to_map_index(FLR.Start_i);
        Player.map_x = graph_to_map_index(FLR.Start_j);
        Player.graph_y = FLR.Start_i;
        Player.graph_x = FLR.Start_j;
    }
    else if (LastStairDirection == DOWN) {
        Player.map_y = graph_to_map_index(FLR.End_i);
        Player.map_x = graph_to_map_index(FLR.End_j);
        Player.graph_y = FLR.End_i;
        Player.graph_x = FLR.End_j;
    }
    else {
        fprintf(stderr, "Error: Invalid stair direction %d\n", LastStairDirection);
        exit(-1);
    }
    Player.id = ID_PLAYER;
    FLR.Graph[Player.graph_y][Player.graph_x].is_explored = true;

    entity_insert(Player);
}

void spawn_stairs()
{
    StairDown.graph_x = FLR.End_j;
    StairDown.graph_y = FLR.End_i;
    StairDown.map_x = graph_to_map_index(FLR.End_j);
    StairDown.map_y = graph_to_map_index(FLR.End_i);
    StairDown.id = ID_STAIR_DOWN;

    StairUp.graph_x = FLR.Start_j;
    StairUp.graph_y = FLR.Start_i;
    StairUp.map_x = graph_to_map_index(FLR.Start_j);
    StairUp.map_y = graph_to_map_index(FLR.Start_i);
    StairUp.id = ID_STAIR_UP;

    entity_insert(StairDown);
    entity_insert(StairUp);
}

void astar_init()
{
    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int j = 0; j < MAP_SIZE; ++j) {
            if (i > 0)
                Nodes[i][j].neighbors.push_back(&Nodes[i - 1][j]);
            if (i < MAP_SIZE - 1)
                Nodes[i][j].neighbors.push_back(&Nodes[i + 1][j]);
            if (j > 0)
                Nodes[i][j].neighbors.push_back(&Nodes[i][j - 1]);
            if (j < MAP_SIZE - 1)
                Nodes[i][j].neighbors.push_back(&Nodes[i][j + 1]);
        }
    }
}

void astar_reset()
{
    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int j = 0; j < MAP_SIZE; ++j) {
            Nodes[i][j].x = j;
            Nodes[i][j].y = i;
            if (FLR.Map[i][j] == WALL)
                Nodes[i][j].obstacle = true;
            else
                Nodes[i][j].obstacle = false;
            Nodes[i][j].parent = nullptr;
            Nodes[i][j].visited = false;
            Nodes[i][j].global_goal = INFINITY;
            Nodes[i][j].local_goal = INFINITY;
        }
    }
}

void astar_solve(int start_i, int start_j, int end_i, int end_j)
{
    astar_reset();
    
    auto distance = [](Node *a, Node *b) {
        return sqrtf((a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y));
    };

    auto heuristic = [distance](Node *a, Node *b) {
        return distance(a, b);
    };

    // start conditions
    Node *start = &Nodes[start_i][start_j];
    Node *end = &Nodes[end_i][end_j];

    Node *current = start;
    current->local_goal = 0.0f;
    current->global_goal = heuristic(start, end);

    std::list<Node *> untested_nodes;
    untested_nodes.push_back(start);

    while (!untested_nodes.empty()) {
        // sort by global goal
        untested_nodes.sort(Node::cmp);

        // ignore nodes already visited
        while (!untested_nodes.empty() && untested_nodes.front()->visited)
            untested_nodes.pop_front();
        
        // popped last node
        if (untested_nodes.empty())
            break;

        current = untested_nodes.front();
        current->visited = true;

        // check neighbors
        for (auto neighbor: current->neighbors) {
            // record the neighbor if it wasn't visited yet
            if (!neighbor->visited && !neighbor->obstacle) {
                untested_nodes.push_back(neighbor);
            }

            // find local goals
            float possible_goal = current->local_goal + distance(current, neighbor);
            if (possible_goal < neighbor->local_goal) {
                neighbor->parent = current;
                neighbor->local_goal = possible_goal;
                neighbor->global_goal = neighbor->local_goal + heuristic(neighbor, end);
            }
        }
    }
}

// TODO: Hack for now
void astar_walk(int *i, int *j)
{
    for (Node *n = &Nodes[StairDown.map_y][StairDown.map_x]; n->parent; n = n->parent) {
        pse::rect_fill(PSE_Context->renderer, pse::Blue,
            SDL_Rect{
                n->x * TILE_WIDTH, n->y * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH
            });
    }
}

/**
 * Player movement
 */

void player_move(int direction)
{
    Player.move(direction);
    FLR.Graph[Player.graph_y][Player.graph_x].is_explored = true;
    
    astar_solve(Player.map_y, Player.map_x, StairDown.map_y, StairDown.map_x);
}

/******************************************************************************
 * Floor Generation
 * 
 * https://web.archive.org/web/20131025132021/http://kuoi.org/~kamikaze/GameDesign/art07_rogue_dungeon.php
 * 
 */

void gen_graph(); // generate a set of rooms from globals, write data structure into the global 'Graph'
bool graph_has_unconnected_neighbors_at(int i, int j);  // return true if a room has at least 1 unconnected neighbor
bool room_try_insert(int *out_direction, int i, int j); // use try_insert fns to randomly connect a room
bool up_try_insert(int i, int j);    // try to make room connection up, return false on fail
bool right_try_insert(int i, int j); // try to make room connection right, return false on fail
bool down_try_insert(int i, int j);  // try to make room connection down, return false on fail
bool left_try_insert(int i, int j);  // try to make room connection left, return false on fail

void gen_map(); // generate the map from the graph of the floor
void gen_floor(); // generate entire floor from subroutines
void floor_switch(int direction); // switch to a different floor

bool up_try_insert(int i, int j)
{
    if (i - 1 >= 0 && FLR.Graph[i - 1][j].index < MAX_NEIGHBORS - 1) {
        FLR.Graph[i][j].insert_neighbor(UP);
        FLR.Graph[i - 1][j].insert_neighbor(DOWN);
        return true;
    }
    return false;
}

bool right_try_insert(int i, int j)
{
    if (j + 1 < GRAPH_SIZE && FLR.Graph[i][j + 1].index < MAX_NEIGHBORS - 1) {
        FLR.Graph[i][j].insert_neighbor(RIGHT);
        FLR.Graph[i][j + 1].insert_neighbor(LEFT);
        return true;
    }
    return false;
}

bool down_try_insert(int i, int j)
{
    if (i + 1 < GRAPH_SIZE && FLR.Graph[i + 1][j].index < MAX_NEIGHBORS - 1) {
        FLR.Graph[i][j].insert_neighbor(DOWN);
        FLR.Graph[i + 1][j].insert_neighbor(UP);
        return true;
    }
    return false;
}

bool left_try_insert(int i, int j)
{
    if (j - 1 >= 0 && FLR.Graph[i][j - 1].index < MAX_NEIGHBORS - 1) {
        FLR.Graph[i][j].insert_neighbor(LEFT);
        FLR.Graph[i][j - 1].insert_neighbor(RIGHT);
        return true;
    }
    return false;
}

bool room_try_insert(int *out_direction, int i, int j)
{
    // randomly select a direction that was not yet chosen
    do {
        *out_direction = rand_range(0, MAX_NEIGHBORS);
    } while (FLR.Graph[i][j].check_neighbor(*out_direction));

    // attempt to connect to the direction
    switch (*out_direction) {
        case UP:    return up_try_insert(i, j);
        case RIGHT: return right_try_insert(i, j);
        case DOWN:  return down_try_insert(i, j);
        case LEFT:  return left_try_insert(i, j);
        default:
            fprintf(stderr, "Error: Invalid room choice: %d\n", *out_direction);
            exit(-1);
    }

    // out variable direction can be recorded
};

bool graph_has_unconnected_neighbors_at(int i, int j)
{
    bool connected = false;

    // bounds check before checking neighbors
    if (i - 1 >= 0)         connected = connected || !FLR.Graph[i - 1][j].is_connected;
    if (i + 1 < GRAPH_SIZE) connected = connected || !FLR.Graph[i + 1][j].is_connected;
    if (j - 1 >= 0)         connected = connected || !FLR.Graph[i][j - 1].is_connected;
    if (j + 1 < GRAPH_SIZE) connected = connected || !FLR.Graph[i][j + 1].is_connected;
    
    return connected;
}

void gen_graph()
{
    // clear global Graph
    for (int i = 0; i < GRAPH_SIZE; ++i) {
        for (int j = 0; j < GRAPH_SIZE; ++j) {
            FLR.Graph[i][j] = Room{};
        }
    }

    // pick a random room to start with (for random walk and player spawn)
    int curr_i = rand_range(0, GRAPH_SIZE);
    int curr_j = rand_range(0, GRAPH_SIZE);
    FLR.Graph[curr_i][curr_j].is_connected = true;
    FLR.Start_i = curr_i; FLR.Start_j = curr_j;

    // connect unconnected neighbors, change the state of direction
    int direction; // direction is modified within room_try_insert

    // random walk across the graph until blocked or finished (no backtracking)
    while (graph_has_unconnected_neighbors_at(curr_i, curr_j)) {
        if (room_try_insert(&direction, curr_i, curr_j)) {
            switch (direction) {
                case UP:    curr_i -= 1; break;
                case RIGHT: curr_j += 1; break;
                case DOWN:  curr_i += 1; break;
                case LEFT:  curr_j -= 1; break;
                default:
                    fprintf(stderr, "Error: Invalid room direction: %d\n", direction);
                    exit(-1);
            }
        }
    }
    // record stair room
    FLR.End_i = curr_i; FLR.End_j = curr_j;

    // connect any still unconnected rooms with at least 2 neighbors, prevent dead room connections
    for (int i = 0; i < GRAPH_SIZE; ++i) {
        for (int j = 0; j < GRAPH_SIZE; ++j) {
            if (!FLR.Graph[i][j].is_connected) {
                int tries = 0;
                while (FLR.Graph[i][j].index < 2) {
                    // direction still passed, but not needed
                    room_try_insert(&direction, i, j);
                    if (tries++ > ROOM_CONNECT_TRIES)
                        break;
                }
            }
        }
    }
}

void gen_map()
{
    // create map of just walls
    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int j = 0; j < MAP_SIZE; ++j) {
            FLR.Map[i][j] = WALL;
        }
    }

    // draw rooms and their doors for each node in the graph into the Map
    for (int i = 0; i < GRAPH_SIZE; ++i) {
        for (int j = 0; j < GRAPH_SIZE; ++j) {
            // ignore empty nodes
            if (FLR.Graph[i][j].index == 0)
                continue;

            // random room width and height relative to the map
            int room_w = rand_range(ROOM_TOLERANCE, ROOM_WIDTH);
            int room_h = rand_range(ROOM_TOLERANCE, ROOM_WIDTH);
            int room_i = i * ROOM_WIDTH;
            int room_j = j * ROOM_WIDTH;
            int center_i = i * ROOM_WIDTH + ROOM_WIDTH / 2;
            int center_j = j * ROOM_WIDTH + ROOM_WIDTH / 2;

            // skip filling room area if a gone room is to be used
            if (rand_uniform() < ROOM_GONE_CHANCE)
                goto create_corridor;

            // fill room area with floor
            for (int ri = room_i + room_h / ROOM_TOLERANCE; ri < room_i + room_h; ++ri) {
                for (int rj = room_j + room_w / ROOM_TOLERANCE; rj < room_j + room_w; ++rj) {
                    FLR.Map[ri][rj] = FLOOR;
                }
            }

create_corridor:
            // walk towards door in each direction from room center if it has a door that way
            if (FLR.Graph[i][j].check_neighbor(DOWN)) {
                for (int ci = center_i; ci < i * ROOM_WIDTH + ROOM_WIDTH * ROOM_PATH_MODIFIER; ++ci)
                    FLR.Map[ci][center_j] = FLOOR;
            }
            if (FLR.Graph[i][j].check_neighbor(UP)) {
                for (int ci = center_i; ci > i * ROOM_WIDTH - ROOM_WIDTH * ROOM_PATH_MODIFIER; --ci)
                    FLR.Map[ci][center_j] = FLOOR;
            }
            if (FLR.Graph[i][j].check_neighbor(RIGHT))
                for (int cj = center_j; cj < j * ROOM_WIDTH + ROOM_WIDTH * ROOM_PATH_MODIFIER; ++cj)
                    FLR.Map[center_i][cj] = FLOOR;
            if (FLR.Graph[i][j].check_neighbor(LEFT))
                for (int cj = center_j; cj > j * ROOM_WIDTH - ROOM_WIDTH * ROOM_PATH_MODIFIER; --cj)
                    FLR.Map[center_i][cj] = FLOOR;
        }
    }

    // draw surrounding border wall around entire map
    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int j = 0; j < MAP_SIZE; ++j) {
            if (i == 0 || i == MAP_SIZE - 1 || j == 0 || j == MAP_SIZE - 1)
                FLR.Map[i][j] = WALL;
        }
    }
}

void gen_floor()
{
    gen_graph();
    gen_map();
    FLR.visited = true;
    spawn_entities();
}

void floor_switch(int direction)
{
    switch (direction) {
    case UP:
        if (FloorLevel - 1 >= 0) {
            FloorLevel--;
            LastStairDirection = DOWN;
        }
        else
            printf("No rooms above...\n");
        break;
    case DOWN:
        if (FloorLevel + 1 < FLOORS_MAX) {
            FloorLevel++;
            LastStairDirection = UP;
            if (!FLR.visited)
                gen_floor();
        }
        else
            printf("You've explored all rooms!\n");
        break;
    default:
        fprintf(stderr, "Error: Invalid floor switch %d\n", direction);
        exit(-1);
    }
    spawn_entities();

    printf("Floor: %d\n", FloorLevel);
}

/******************************************************************************
 * Drawing
 * 
 */

void draw_graph();
void draw_graph_room(int i, int j);
void draw_graph_doors(int i, int j);
void draw_map();
void draw_entities();

void draw_graph()
{
    // draw rooms and doors
    for (int i = 0; i < GRAPH_SIZE; ++i) {
        for (int j = 0; j < GRAPH_SIZE; ++j) {
            draw_graph_room(i, j);
        }
    }
    for (int i = 0; i < GRAPH_SIZE; ++i) {
        for (int j = 0; j < GRAPH_SIZE; ++j) {
            draw_graph_doors(i, j);
        }
    }
}

void draw_graph_room(int i, int j)
{
    SDL_Color c;
    if (i == FLR.Start_i && j == FLR.Start_j)
        c = pse::Sky;
    else if (i == FLR.End_i && j == FLR.End_j)
        c = pse::Orange;
    else if (FLR.Graph[i][j].index == 0)
        c = pse::Red;
    else
        c = pse::Blue;

    pse::rect_fill(PSE_Context->renderer, c, SDL_Rect{
        i * TILE_SCALING + TILE_SCALING / 20,
        j * TILE_SCALING + TILE_SCALING / 20,
        TILE_SCALING - TILE_SCALING / 10,
        TILE_SCALING - TILE_SCALING / 10 });
}

void draw_graph_doors(int i, int j)
{
    for (int k = 0; k < FLR.Graph[i][j].index; ++k) {
        int x = j * TILE_SCALING;
        int y = i * TILE_SCALING;
        switch (FLR.Graph[i][j].neighbors[k]) {
        case UP:
            pse::rect_fill(PSE_Context->renderer, pse::Purple, SDL_Rect{
                x + TILE_SCALING / 2 - TILE_WIDTH / 2,
                y, TILE_WIDTH, TILE_WIDTH
                });
            break;
        case RIGHT:
            pse::rect_fill(PSE_Context->renderer, pse::Purple, SDL_Rect{
                x + TILE_SCALING - TILE_WIDTH,
                y + TILE_SCALING / 2 - TILE_WIDTH / 2,
                TILE_WIDTH, TILE_WIDTH
                });
            break;
        case DOWN:
            pse::rect_fill(PSE_Context->renderer, pse::Purple, SDL_Rect{
                x + TILE_SCALING / 2 - TILE_WIDTH / 2,
                y + TILE_SCALING - TILE_WIDTH,
                TILE_WIDTH, TILE_WIDTH
                });
            break;
        case LEFT:
            pse::rect_fill(PSE_Context->renderer, pse::Purple, SDL_Rect{
                x,
                y + TILE_SCALING / 2 - TILE_WIDTH / 2,
                TILE_WIDTH, TILE_WIDTH
                });
            break;
        }
    }
}

void draw_map()
{
    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int j = 0; j < MAP_SIZE; ++j) {
            SDL_Color c;
            switch (FLR.Map[i][j]) {
                case WALL: c = pse::Dark; break;
                case FLOOR:
                    if (Player.graph_x == map_to_graph_index(j) && Player.graph_y == map_to_graph_index(i))
                        c = pse::White;
                    else if (FLR.Graph[map_to_graph_index(i)][map_to_graph_index(j)].is_explored)
                        c = pse::Gray;
                    else
                        c = pse::Dark;
                    break;
                case EMPTY: c = pse::Black; break;
                default:
                    c = pse::Magenta;
            }
            pse::rect_fill(PSE_Context->renderer, c, SDL_Rect{
                j * TILE_WIDTH, i * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH
            });
        }
    }
}

void draw_entities()
{
    // traverse backwards, make first inserted displayed on top
    for (int i = EntityIndex - 1; i >= 0; --i) {
        if (!Entities[i]) {
#ifdef DEBUG
            printf("Invalid entity: %d\n", i);
#endif
            break;
        }
        
        SDL_Rect rect{ Entities[i]->map_x * TILE_WIDTH, Entities[i]->map_y * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH };
        SDL_Color c;

        switch (Entities[i]->id) {
            case ID_PLAYER:     c = pse::Red; break;
            case ID_STAIR_DOWN:
                if (FLR.Graph[StairDown.graph_y][StairDown.graph_x].is_explored)
                    c = pse::Orange;
                else
                    c = pse::Dark;
                break;
            case ID_STAIR_UP:   c = pse::Salmon; break;
            default:
                c = pse::Magenta;
        }
        pse::rect_fill(PSE_Context->renderer, c, rect);
    }
}

/******************************************************************************
 * PSE Interface
 *
 */

void rogue_setup(pse::Context& ctx)
{
    PSE_Context = &ctx;
    gen_floor();
    astar_init();
}

void rogue_update(pse::Context& ctx)
{
    if (ctx.check_key(SDL_SCANCODE_LSHIFT))
        gen_floor();

    if (ctx.check_key_invalidate(SDL_SCANCODE_K)
            || ctx.check_key_invalidate(SDL_SCANCODE_UP))
        player_move(UP);
    else if (ctx.check_key_invalidate(SDL_SCANCODE_L)
            || ctx.check_key_invalidate(SDL_SCANCODE_RIGHT))
        player_move(RIGHT);
    else if (ctx.check_key_invalidate(SDL_SCANCODE_J)
            || ctx.check_key_invalidate(SDL_SCANCODE_DOWN))
        player_move(DOWN);
    else if (ctx.check_key_invalidate(SDL_SCANCODE_H)
            || ctx.check_key_invalidate(SDL_SCANCODE_LEFT))
        player_move(LEFT);

    if (Player.map_x == StairDown.map_x
            && Player.map_y == StairDown.map_y
            && ctx.check_key_invalidate(SDL_SCANCODE_SPACE))
        floor_switch(DOWN);
    else if (Player.map_x == StairUp.map_x
            && Player.map_y == StairUp.map_y
            && ctx.check_key_invalidate(SDL_SCANCODE_SPACE))
        floor_switch(UP);

    draw_map();
    draw_entities();
    // astar_solve called in player_move
    astar_walk(nullptr, nullptr);
    //debug_print_player();
}

}
