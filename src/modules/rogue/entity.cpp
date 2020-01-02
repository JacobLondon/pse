#include <algorithm>

#include "entity.hpp"
#include "gen.hpp"
#include "globals.hpp"
#include "types.hpp"

namespace Modules {

void entity_insert(Entity *e)
{
    if (EntityIndex + 1 < ENTITY_MAX) {
        Entities[EntityIndex++] = e;
    }
}

void rand_room_tile(int gi, int gj, int *i, int *j)
{
    if (FLR.Graph[gi][gj].is_gone)
        FLR.Graph[gi][gj].rand_corridor(i, j);
    else
        FLR.Graph[gi][gj].rand_tile(i, j);
}

void rand_room_tile_no_overlap(int gi, int gj, int *i, int *j)
{
    do {
        rand_room_tile(gi, gj, i, j);
    } while (!empty_coords(*i, *j));
}

bool empty_coords(int i, int j)
{
    for (int k = 0; k < ENTITY_MAX; ++k) {
        if (!Entities[k])
            continue;
        if (Entities[k]->map_y == i && Entities[k]->map_x == j)
            return false;
    }
    return true;
}

void spawn_entities()
{
    // stop looking at each entity
    for (int i = 0; i < ENTITY_MAX; ++i) {
        if (!Entities[i])
            continue;
        
        if (Entities[i]->is_enemy) {
            delete Entities[i];
        }

        Entities[i] = nullptr;
    }
    // reset
    EntityIndex = 0;

    // "permanent" entities
    spawn_stairs();
    spawn_player();
    spawn_enemies();
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

    entity_insert(&Player);
}

void spawn_stairs()
{
    if (!FLR.visited) {
        rand_room_tile_no_overlap(FLR.End_i, FLR.End_j, &FLR.StairDown.map_y, &FLR.StairDown.map_x);
        rand_room_tile_no_overlap(FLR.Start_i, FLR.Start_j, &FLR.StairUp.map_y, &FLR.StairUp.map_x);

        FLR.StairDown.graph_x = FLR.End_j;
        FLR.StairDown.graph_y = FLR.End_i;
        FLR.StairDown.id = ID_STAIR_DOWN;

        FLR.StairUp.graph_x = FLR.Start_j;
        FLR.StairUp.graph_y = FLR.Start_i;
        FLR.StairUp.id = ID_STAIR_UP;

        FLR.visited = true;
    }

    entity_insert(&FLR.StairDown);
    entity_insert(&FLR.StairUp);
}

void spawn_enemies()
{
    for (int i = 0; i < rand_range(ENEMY_MIN, ENEMY_MAX); ++i) {
        Entity *enemy = new Entity{};

        enemy->is_enemy = true;
        enemy->id = ID_ENEMY;
        // pick random room
        enemy->graph_x = rand_range(0, GRAPH_SIZE);
        enemy->graph_y = rand_range(0, GRAPH_SIZE);
        rand_room_tile_no_overlap(enemy->graph_y, enemy->graph_x, &enemy->map_y, &enemy->map_x);

        entity_insert(enemy);
    }
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

void astar_walk(int *start_i, int *start_j, int end_i, int end_j)
{
    astar_solve(*start_i, *start_j, end_i, end_j);

    // find next adjacent square to walk to
    for (Node *n = &Nodes[end_i][end_j]; n->parent; n = n->parent) {
        if (!n->parent->parent) {
            *start_i = n->y;
            *start_j = n->x;
        }
    }
}

void entity_move(int direction)
{
    enemy_move();
    player_move(direction);
}

void player_move(int direction)
{
    Player.move(direction);
    FLR.Graph[Player.graph_y][Player.graph_x].is_explored = true;
}

void enemy_move()
{
    for (int i = 0; i < ENTITY_MAX; ++i) {
        if (!Entities[i] || !Entities[i]->is_enemy)
            continue;
        
        int tmp_y = Entities[i]->map_y;
        int tmp_x = Entities[i]->map_x;
        astar_walk(&tmp_y, &tmp_x, Player.map_y, Player.map_x);

        // ensure there is a spot to walk to
        if (empty_coords(tmp_y, tmp_x)) {
            Entities[i]->map_x = tmp_x;
            Entities[i]->map_y = tmp_y;
            Entities[i]->graph_x = map_to_graph_index(Entities[i]->map_x);
            Entities[i]->graph_y = map_to_graph_index(Entities[i]->map_y);
        }
    }
}

}