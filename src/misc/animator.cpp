#include "misc/animator.h"

#include <algorithm>

#include <doctest.h>

namespace SingularityTrainer
{
Animator::Animator() {}

void Animator::add_animation(Animation &animation)
{
    animations.push_back(animation);
}

void Animator::update(double delta_time)
{
    for (auto &animation : animations)
    {
        animation.step_function(delta_time);
        animation.elapsed_time += delta_time;
    }

    animations.erase(std::remove_if(
                         animations.begin(), animations.end(),
                         [](const Animation &animation) {
                             return animation.elapsed_time >= animation.length;
                         }),
                     animations.end());
}

TEST_CASE("Animator")
{
    Animator animator;

    SUBCASE("Added animations are called in update()")
    {
        bool called = false;
        Animation animation{
            [&](double) { called = true; },
            1.};
        animator.add_animation(animation);

        animator.update(1);

        CHECK(called == true);
    }

    SUBCASE("Multiple added animations are called in update()")
    {
        bool called_1 = false;
        Animation animation_1{
            [&](double) { called_1 = true; },
            1.};
        animator.add_animation(animation_1);

        bool called_2 = false;
        Animation animation_2{
            [&](double) { called_2 = true; },
            1.};
        animator.add_animation(animation_2);

        animator.update(1);

        CHECK(called_1 == true);
        CHECK(called_2 == true);
    }

    SUBCASE("Animations are removed once they time out")
    {
        bool called = false;
        Animation animation{
            [&](double) { called = !called; },
            1.};
        animator.add_animation(animation);

        animator.update(1);

        CHECK(called == true);

        animator.update(1);

        CHECK(called == true);
    }

    SUBCASE("Animations can be stepped in non-integer increments")
    {
        bool called = false;
        Animation animation{
            [&](double) { called = !called; },
            1.};
        animator.add_animation(animation);

        animator.update(0.5);

        CHECK(called == true);

        animator.update(0.5);

        CHECK(called == false);
    }
}
}