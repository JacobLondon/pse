#include <algorithm>

#include "entity.hpp"
#include "globals.hpp"
#include "types.hpp"

namespace Modules {

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
    spawn_stairs();
    spawn_player();
}

void spawn_player()
{
    // spawn player at last stair taken
    if (LastStairDirection == UP) {
        Player.map_y = FLR.StairUp.map_y;
        Player.map_x = FLR.StairUp.map_x;
        Player.graph_y = FLR.Start_i;
        Player.graph_x = FLR.Start_j;
    }
    else if (LastStairDirection == DOWN) {
        Player.map_y = FLR.StairDown.map_y;
        Player.map_x = FLR.StairDown.map_x;
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
    if (!FLR.visited) {
        // spawn in the corridors only
        if (FLR.Graph[FLR.End_i][FLR.End_j].is_gone) {
            FLR.Graph[FLR.End_i][FLR.End_j].rand_corridor(&FLR.StairDown.map_y, &FLR.StairDown.map_x);
            // don't let stairs overlap
            do {
                FLR.Graph[FLR.Start_i][FLR.Start_j].rand_corridor(&FLR.StairUp.map_y, &FLR.StairUp.map_x);
            } while (FLR.StairUp.map_y == FLR.StairDown.map_y && FLR.StairUp.map_x == FLR.StairDown.map_x);
        }
        // spawn in the room
        else {
            FLR.Graph[FLR.End_i][FLR.End_j].rand_tile(&FLR.StairDown.map_y, &FLR.StairDown.map_x);
            // don't let stairs overlap
            do {
                FLR.Graph[FLR.Start_i][FLR.Start_j].rand_tile(&FLR.StairUp.map_y, &FLR.StairUp.map_x);
            } while (FLR.StairUp.map_y == FLR.StairDown.map_y && FLR.StairUp.map_x == FLR.StairDown.map_x);
        }

        FLR.StairDown.graph_x = FLR.End_j;
        FLR.StairDown.graph_y = FLR.End_i;
        FLR.StairDown.id = ID_STAIR_DOWN;

        FLR.StairUp.graph_x = FLR.Start_j;
        FLR.StairUp.graph_y = FLR.Start_i;
        FLR.StairUp.id = ID_STAIR_UP;

        FLR.visited = true;
    }

    entity_insert(FLR.StairDown);
    entity_insert(FLR.StairUp);
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
    UntestedNodes.resize((size_t)((float)MAP_SIZE * logf(MAP_SIZE)));
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

    // start conditions
    Node *start = &Nodes[start_i][start_j];
    Node *end = &Nodes[end_i][end_j];

    Node *current = start;
    current->local_goal = 0.0f;
    current->global_goal = Node::dist(start, end);
    UntestedNodes.clear();
    UntestedNodes.push_back(start);

    while (!UntestedNodes.empty()) {
        // sort by global goal
        std::sort(std::begin(UntestedNodes), std::end(UntestedNodes), Node::cmp);

        // ignore nodes already visited
        while (!UntestedNodes.empty() && UntestedNodes.front()->visited)
            UntestedNodes.pop_front();
        
        // popped last node
        if (UntestedNodes.empty())
            break;

        current = UntestedNodes.front();
        current->visited = true;

        // check neighbors
        for (auto neighbor: current->neighbors) {
            // record the neighbor if it wasn't visited yet
            if (!neighbor->visited && !neighbor->obstacle)
                UntestedNodes.push_back(neighbor);

            // find local goals
            float possible_goal = current->local_goal + Node::dist(current, neighbor);
            if (possible_goal < neighbor->local_goal) {
                neighbor->parent = current;
                neighbor->local_goal = possible_goal;
                neighbor->global_goal = neighbor->local_goal + Node::dist(neighbor, end);
            }
        }
    }
}

// TODO: Hack for now
void astar_walk()
{
    for (Node *n = &Nodes[FLR.StairDown.map_y][FLR.StairDown.map_x]; n->parent; n = n->parent) {
        pse::rect_fill(PSE_Context->renderer, pse::Blue,
            SDL_Rect{
                n->x * TILE_WIDTH, n->y * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH
            });
    }
}

void player_move(int direction)
{
    Player.move(direction);
    FLR.Graph[Player.graph_y][Player.graph_x].is_explored = true;
    
    astar_solve(Player.map_y, Player.map_x, FLR.StairDown.map_y, FLR.StairDown.map_x);
}

}