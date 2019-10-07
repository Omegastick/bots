#pragma once

#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <nlohmann/json_fwd.hpp>

#include "graphics/idrawable.h"
#include "training/actions/iaction.h"
#include "training/modules/module_link.h"

namespace SingularityTrainer
{
class ModuleLink;
class IAction;
class Body;
struct RenderData;
class Sprite;

class IModule : public IDrawable
{
  protected:
    Body *body;
    IModule *root;
    std::vector<ModuleLink> module_links;
    std::vector<std::unique_ptr<IAction>> actions;
    b2Transform transform;
    std::vector<b2PolygonShape> shapes;
    std::unique_ptr<Sprite> sprite;

  public:
    IModule();
    virtual ~IModule() = 0;

    virtual void update();
    virtual void sub_update();
    virtual std::vector<IModule *> get_children();
    virtual std::vector<IModule *> get_children(std::vector<IModule *> child_list);
    virtual std::vector<float> get_sensor_reading() const;
    virtual RenderData get_render_data(bool lightweight = false);
    virtual b2Transform get_global_transform() const;
    virtual int get_observation_count() const = 0;
    virtual nlohmann::json to_json() const = 0;

    inline const std::vector<std::unique_ptr<IAction>> &get_actions() const { return actions; }
    inline const Body *get_body() const { return body; };
    inline void set_body(Body *body) { this->body = body; };
    inline std::vector<ModuleLink> &get_module_links() { return module_links; }
    inline const IModule *get_root_module() const { return root; }
    inline const std::vector<b2PolygonShape> &get_shapes() const { return shapes; }
    inline b2Transform &get_transform() { return transform; }
};

inline IModule::~IModule() {}
}