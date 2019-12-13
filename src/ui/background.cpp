#include <algorithm>
#include <cmath>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include "background.h"
#include "graphics/colors.h"
#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"
#include "misc/random.h"
#include "misc/utils/range.h"

constexpr int num_nodes = 100;
constexpr glm::vec2 wind = {0.01f, 0.01f};
constexpr float drift_strength = 0.004f;

namespace SingularityTrainer
{
Background::Background(Random &rng)
    : delta_time(0),
      neighbor_indices(num_nodes),
      rng(rng)
{
    std::iota(neighbor_indices.begin(), neighbor_indices.end(), 0);

    nodes.reserve(num_nodes);
    for (int i = 0; i < num_nodes; ++i)
    {
        nodes.push_back({{rng.next_float(-1, 1),
                          rng.next_float(-1, 1)},
                         {rng.next_float(-drift_strength, drift_strength),
                          rng.next_float(-drift_strength, drift_strength)}});
    }

    for (auto &node : nodes)
    {
        recalculate_neighbors(node);
    }
}

void Background::draw(Renderer &renderer) const
{
    const auto view = renderer.get_view();
    float view_x = 1.f / view[0][0] * 1.5f;
    float view_y = 1.f / view[1][1] * 1.5f;
    float scale = view_x > view_y ? view_x : view_y;

    for (const auto &node : nodes)
    {
        Circle circle{0.01f * scale,
                      cl_base0,
                      cl_base0,
                      0.005f * scale};
        circle.transform.set_position(node.position * scale);
        renderer.draw(circle);

        for (const auto neighbor : node.neighbors)
        {
            if (!neighbor)
            {
                continue;
            }
            const float distance = std::pow(
                                       glm::length(node.position - neighbor->position) + 1,
                                       3.f) *
                                   10.f;
            renderer.draw(Line{{node.position * scale, neighbor->position * scale},
                               {0.003f * scale, 0.003f * scale},
                               {set_alpha(cl_white, 1.f / distance),
                                set_alpha(cl_white, 1.f / distance)}});
        }
    }
}

void Background::recalculate_neighbors(Node &node)
{
    std::sort(neighbor_indices.begin(), neighbor_indices.end(),
              [&](std::size_t lhs, std::size_t rhs) {
                  const auto lhs_dist = glm::length2(nodes[lhs].position - node.position);
                  const auto rhs_dist = glm::length2(nodes[rhs].position - node.position);
                  return lhs_dist < rhs_dist;
              });
    for (const int i : range(1, rng.next_int(3, 8)))
    {
        node.neighbors[i - 1] = &nodes[neighbor_indices[i]];
    }
}

void Background::update(double delta_time)
{
    for (auto &node : nodes)
    {
        node.position += (wind + node.drift) * static_cast<float>(delta_time);
        bool should_recalculate_neighbors = false;
        if (node.position.x > 1)
        {
            node.position.x = -1;
            should_recalculate_neighbors = true;
        }
        if (node.position.y > 1)
        {
            node.position.y = -1;
            should_recalculate_neighbors = true;
        }

        if (should_recalculate_neighbors)
        {
            recalculate_neighbors(node);
            for (auto &neighbor : node.neighbors)
            {
                if (neighbor)
                {
                    recalculate_neighbors(*neighbor);
                }
            }
        }
    }
}
}