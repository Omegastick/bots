#include "misc/animator.h"

#include <algorithm>

#include <doctest.h>

namespace SingularityTrainer
{
template <class ForwardIt, class UnaryPredicate>
ForwardIt remove_if_with_callback(ForwardIt first, ForwardIt last, UnaryPredicate predicate)
{
    auto iter = std::find_if(first, last, predicate);
    if (iter != last)
    {
        iter->finish_callback();
        for (ForwardIt i = iter; ++i != last;)
        {
            if (!predicate(*i))
            {
                *iter++ = std::move(*i);
            }
        }
    }

    return iter;
}

Animator::Animator() {}

void Animator::add_animation(Animation &&animation)
{
    animations.push_back(std::move(animation));
}

void Animator::update(double delta_time)
{
    for (auto &animation : animations)
    {
        animation.step_function(delta_time / animation.length);
        animation.elapsed_time += delta_time;
    }

    animations.erase(remove_if_with_callback(
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
        animator.add_animation(std::move(animation));

        animator.update(1);

        CHECK(called == true);
    }

    SUBCASE("Multiple added animations are called in update()")
    {
        bool called_1 = false;
        Animation animation_1{
            [&](double) { called_1 = true; },
            1.};
        animator.add_animation(std::move(animation_1));

        bool called_2 = false;
        Animation animation_2{
            [&](double) { called_2 = true; },
            1.};
        animator.add_animation(std::move(animation_2));

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
        animator.add_animation(std::move(animation));

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
        animator.add_animation(std::move(animation));

        animator.update(0.5);

        CHECK(called == true);

        animator.update(0.5);

        CHECK(called == false);
    }

    SUBCASE("Final callback is called when an animation finishes")
    {
        bool finished = false;
        Animation animation{
            [](double) {},
            1.,
            [&] { finished = true; }};
        animator.add_animation(std::move(animation));

        animator.update(0.5);

        CHECK(finished == false);

        animator.update(0.5);

        CHECK(finished == true);
    }

    SUBCASE("Animations step callbacks are called with the percentage by which to step forward")
    {
        double passed_value = false;
        Animation animation{
            [&](double step_percent) { passed_value = step_percent; },
            2.};
        animator.add_animation(std::move(animation));

        animator.update(1);

        CHECK(passed_value == doctest::Approx(0.5));
    }
}
}