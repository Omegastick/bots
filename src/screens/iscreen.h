#pragma once

#include <doctest.h>
#include <doctest/trompeloeil.hpp>

#include "graphics/renderers/renderer.h"

namespace ai
{
class IScreen
{
  public:
    virtual ~IScreen() = 0;

    virtual void on_show();
    virtual void draw(Renderer &renderer, bool lightweight = false) = 0;
    virtual void update(double delta_time) = 0;
};

inline IScreen::~IScreen() {}
inline void IScreen::on_show() {}

class MockScreen : public trompeloeil::mock_interface<IScreen>
{
    IMPLEMENT_MOCK0(on_show);
    IMPLEMENT_MOCK1(update);
    IMPLEMENT_MOCK2(draw);
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