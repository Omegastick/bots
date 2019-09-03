#pragma once

#include <doctest.h>
#include <doctest/trompeloeil.hpp>

#include "graphics/renderers/renderer.h"

namespace SingularityTrainer
{
class IScreen
{
  public:
    virtual ~IScreen() = 0;

    virtual void update(double delta_time) = 0;
    virtual void draw(Renderer &renderer, bool lightweight = false) = 0;
};

inline IScreen::~IScreen() {}

class MockScreen : public IScreen
{
    MAKE_MOCK1(update, void(double), override);
    MAKE_MOCK2(draw, void(Renderer &, bool), override);
};

class IScreenFactory
{
  public:
    virtual ~IScreenFactory() = 0;

    virtual std::shared_ptr<IScreen> make() = 0;
};

inline IScreenFactory::~IScreenFactory() {}

class MockScreenFactory : public IScreenFactory
{
  public:
    MAKE_MOCK0(make, std::shared_ptr<IScreen>(), override);
};
}