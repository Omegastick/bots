#pragma once

#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <glm/glm.hpp>

#include "training/agents/agent.h"

namespace SingularityTrainer
{
class IModule;
class IO;
class ModuleLink;
class Random;
class RenderData;

class GetAllQueryCallback : public b2QueryCallback
{
  private:
    std::vector<b2Fixture *> fixtures;

  public:
    virtual bool ReportFixture(b2Fixture *fixture);

    inline std::vector<b2Fixture *> &get_collisions() { return fixtures; }
};

struct NearestModuleLinkResult
{
    ModuleLink *nearest_link;
    ModuleLink *origin_link;
    double distance;
};

class ShipBuilder
{
  private:
    Agent agent;
    IO *io;
    b2World *b2_world;
    glm::mat4 projection;

  public:
    ShipBuilder(b2World &b2_world, Random &rng, IO &io);

    std::shared_ptr<IModule> get_module_at_screen_position(glm::vec2 point);
    NearestModuleLinkResult get_nearest_module_link_to_world_position(glm::vec2 point);
    NearestModuleLinkResult get_nearest_module_link_to_module(IModule &module);
    RenderData get_render_data(bool lightweight = false);
    std::shared_ptr<IModule> place_module(std::shared_ptr<IModule> selected_module);

    inline Agent *get_agent() { return &agent; }
    inline glm::mat4 &get_projection() { return projection; }
};
}