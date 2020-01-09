#include "misc/animator.h"

#include <algorithm>

#include <doctest.h>

namespace ai
{
template <class Key, class T, class Compare, class Alloc, class Predicate>
void erase_if_with_callback(std::map<Key, T, Compare, Alloc> &c, Predicate predicate)
{
    for (auto i = c.begin(), last = c.end(); i != last;)
    {
        if (predicate(*i))
        {
            i->second.finish_callback();
            i = c.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

Animator::Animator() : current_id(0) {}

unsigned long Animator::add_animation(Animation &&animation)
{
    animations[current_id++] = std::move(animation);
    return current_id - 1;
}

void Animator::delete_animation(unsigned long id)
{
    animations.erase(id);
}

void Animator::update(double delta_time)
{
    for (auto &[id, animation] : animations)
    {
        animation.step_function(delta_time / animation.length);
        animation.elapsed_time += delta_time;
    }

    erase_if_with_callback(animations,
                           [](const std::pair<const long unsigned int,
                                              ai::Animation>
                                  animation) {
                               return animation.second.elapsed_time >= animation.second.length;
                           });
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

        DOCTEST_CHECK(called == true);
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

        DOCTEST_CHECK(called_1 == true);
        DOCTEST_CHECK(called_2 == true);
    }

    SUBCASE("Animations are removed once they time out")
    {
        bool called = false;
        Animation animation{
            [&](double) { called = !called; },
            1.};
        animator.add_animation(std::move(animation));

        animator.update(1);

        DOCTEST_CHECK(called == true);

        animator.update(1);

        DOCTEST_CHECK(called == true);
    }

    SUBCASE("Animations can be stepped in non-integer increments")
    {
        bool called = false;
        Animation animation{
            [&](double) { called = !called; },
            1.};
        animator.add_animation(std::move(animation));

        animator.update(0.5);

        DOCTEST_CHECK(called == true);

        animator.update(0.5);

        DOCTEST_CHECK(called == false);
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

        DOCTEST_CHECK(finished == false);

        animator.update(0.5);

        DOCTEST_CHECK(finished == true);
    }

    SUBCASE("Animations step callbacks are called with the percentage by which to step forward")
    {
        double passed_value = 0;
        Animation animation{
            [&](double step_percent) { passed_value = step_percent; },
            2.};
        animator.add_animation(std::move(animation));

        animator.update(1);

        DOCTEST_CHECK(passed_value == doctest::Approx(0.5));
    }

    SUBCASE("IDs are assigned sequentially")
    {
        DOCTEST_CHECK(animator.add_animation({[](double) {}, 1.f, [] {}}) == 0);
        DOCTEST_CHECK(animator.add_animation({[](double) {}, 1.f, [] {}}) == 1);

        animator.delete_animation(1);

        DOCTEST_CHECK(animator.add_animation({[](double) {}, 1.f, [] {}}) == 2);
    }

    SUBCASE("Deleted animations don't trigger")
    {
        bool called = false;
        Animation animation{
            [&](double) { called = true; },
            1.};
        animator.add_animation(std::move(animation));
        animator.delete_animation(0);

        animator.update(1);

        DOCTEST_CHECK(called == false);
    }
}
}