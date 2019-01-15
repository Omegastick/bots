#include <SFML/Graphics.hpp>
#include <iomanip>
#include <sstream>

#include "idrawable.h"
#include "resource_manager.h"
#include "test_screen/score_display.h"

namespace SingularityTrainer
{
ScoreDisplay::ScoreDisplay(float x, float y, float smoothing_weight, ResourceManager &resource_manager)
    : smoothing_weight(smoothing_weight), running_score(0)
{
    resource_manager.load_font("hack", "cpp/assets/fonts/RobotoCondensed-Regular.ttf");
    text.setFillColor(sf::Color::White);
    text.setPosition(x, y);
    text.setFont(*resource_manager.font_store.get("hack"));
    text.setCharacterSize(100);
    text.setScale(0.01, 0.01);
}

ScoreDisplay::~ScoreDisplay() {}

void ScoreDisplay::add_score(float score)
{
    running_score = running_score * smoothing_weight + (1 - smoothing_weight) * score;
}

void ScoreDisplay::draw(sf::RenderTarget &render_target)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << running_score;
    text.setString(stream.str());
    render_target.draw(text);
}
}