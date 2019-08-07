#pragma once

namespace SingularityTrainer
{
class Renderer;

class IScreen
{
  public:
    virtual ~IScreen() = 0;

    virtual void update(double delta_time) = 0;
    virtual void draw(Renderer &renderer, bool lightweight = false) = 0;
};

inline IScreen::~IScreen() {}

class IScreenFactory
{
  public:
    virtual ~IScreenFactory() = 0;

    virtual std::shared_ptr<IScreen> make() = 0;
};

inline IScreenFactory::~IScreenFactory() {}
}