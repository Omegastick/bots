#pragma once

#include <vector>

#include <glm/vec2.hpp>

namespace ai
{
class Random;
class Renderer;

class Background
{
  private:
    struct Node
    {
        glm::vec2 position;
        glm::vec2 drift;
        Node *neighbors[6] = {nullptr};
    };

    double delta_time;
    std::vector<std::size_t> neighbor_indices;
    std::vector<Node> nodes;
    Random &rng;

  public:
    Background(Random &rng);

    void draw(Renderer &renderer) const;
    void recalculate_neighbors(Node &node);
    void update(double delta_time);
};
}