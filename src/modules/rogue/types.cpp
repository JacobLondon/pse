#include "../../modules.hpp"
#include "gen.hpp"
#include "types.hpp"

namespace Modules {

/**
 * Room
 */

void Room::insert_neighbor(int neighbor) {
    neighbors[index++] = neighbor;
    is_connected = true;
}
// check if the given neighbor is already connected to
bool Room::check_neighbor(int neighbor) {
    for (int i = 0; i < index; ++i) {
        if (neighbors[i] == neighbor)
            return true;
    }
    return false;
}

// range within the room, not including outer area/walls
void Room::rand_tile(int* i, int *j) {
    *i = rand_range(map_i + 2, map_i + map_h - 2);
    *j = rand_range(map_j + 2, map_j + map_w - 2);
}

void Room::rand_corridor(int *i, int *j) {
    // pick a direction
    switch (neighbors[rand_range(0, index)]) {
        case UP:    *i = rand_range(center_i - map_h / 2, center_i); *j = center_j; break;
        case RIGHT: *j = rand_range(center_j, center_j + map_h / 2); *i = center_i; break;
        case DOWN:  *i = rand_range(center_i, center_i + map_h / 2); *j = center_j; break;
        case LEFT:  *j = rand_range(center_j - map_h / 2, center_j); *i = center_i; break;
    }
}

void Room::print() {
    for (int i = 0; i < index; ++i) {
        switch (neighbors[i]) {
            case UP:    printf("Up ");    break;
            case RIGHT: printf("Right "); break;
            case DOWN:  printf("Down ");  break;
            case LEFT:  printf("Left ");  break;
        }
    }
}

/**
 * Entity
 */

bool Entity::check_tile(int offset_x, int offset_y)
{
    switch (FLR.Map[map_y + offset_y][map_x + offset_x]) {
        case FLOOR:
            return true;
        default:
            return false;
    }
}

void Entity::move(int direction)
{
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

}
