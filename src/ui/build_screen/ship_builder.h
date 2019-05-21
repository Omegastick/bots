#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <glm/glm.hpp>

namespace SingularityTrainer
{
class Agent;
class IModule;
class IO;
class ModuleLink;
class Random;

class GetAllQueryCallback : public b2QueryCallback
{
  private:
    std::vector<b2Fixture *> fixtures;

  public:
    virtual bool ReportFixture(b2Fixture *fixture);

    inline std::vector<b2Fixture *> &get_collisions() { return fixtures; }
};

struct ModuleLinkAndDistance
{
    ModuleLink *module_link;
    double distance;
};

class ShipBuilder
{
  private:
    std::unique_ptr<Agent> agent;
    IO *io;
    b2World *b2_world;
    glm::mat4 projection;

  public:
    ShipBuilder(b2World &b2_world, Random &rng, IO &io);

    IModule *get_module_at_point(glm::vec2 point);
    ModuleLinkAndDistance get_nearest_module_link(glm::vec2 point);
    IModule *update(IModule *selected_module);

    inline Agent *get_agent() const { return agent.get(); }
};
}