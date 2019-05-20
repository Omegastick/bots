#include <memory>

#include <glm/vec2.hpp>

namespace SingularityTrainer
{
class Agent;
class ModuleLink;
class IModule;

struct ModuleLinkAndDistance
{
    ModuleLink *module_link;
    double distance;
};

class ShipBuilder
{
  private:
    std::unique_ptr<Agent> agent;

  public:
    ShipBuilder();

    IModule *get_module_at_point(glm::vec2 point);
    ModuleLinkAndDistance get_nearest_module_link(glm::vec2 point);
    void update();

    inline Agent *get_agent() const { return agent.get(); }
};
}