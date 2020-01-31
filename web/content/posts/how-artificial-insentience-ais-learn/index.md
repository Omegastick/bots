---
title: "How ai://artificial_insentience AIs learn"
date: 2020-01-31T16:30:31+09:00
draft: false
comments: true
toc: false
tags:
  - "ai"
  - "reinforcement learning"
---
The AIs you train in `ai://artificial_insentience` are intelligent. They can learn advanced tactics and have the precision to perform them better than any human. This wasn't possible to do on your home computer until recent advances in a field of machine learning called 'reinforcement learning'.

Reinforcement learning is system for teaching AIs to complete tasks, without giving them instructions. In typical machine learning, you might build a large database of examples for your AI to learn from, which has to be constructed from scratch by humans. In reinforcement learning, you tell the AI what you want it to do and let it keep trying until it gets it right.

## Inputs and outputs

Let's put that in the context of `ai://aritificial_insentience`. We want our AI to control a body and win the game. Here's our example body:

{{< img "images/body.png" "A super simple body in action" >}}

Each gun module on the body can be activated to shoot a bullet, and each thruster module can be activated to propel the body forward. The body has two of each, so that's four total outputs. You can think of those as like buttons on a controller. Each frame, you can press any combination of them and activate the corresponding effect.

It also has one input module, the laser sensor. That's the little semi-circle on the front. You can't see them now (don't worry, you can in the game), but that emits 20 little lasers that each report back the distance to the closest object in that direction. This gives the AI something similar to 2D vision. The AI is also aware of its velocity (forwards, sideways, and rotationally) totalling 23 inputs.

Now, every frame the AI gets its inputs, takes an action based on them, and gets a 'reward' depending on how well it did. We have to decide how these rewards are dished out, and that is a *very* large part of training an AI.

## Rewards

So, we might decide that shooting the enemy is worth one 'reward point'. 'Reward points' don't directly count for winning the game, they are just a hint for the AI as to how well it is doing. Capturing the hill might be considered *more* important than shooting the enemy, so maybe we give that two reward points. Winning the game is, of course, the best thing the AI can do, so we might give that 100 reward points. These reward points are completely arbitrary and can be set to whatever you like. A carefully chosen reward scheme will shape the AI's behaviour.

After all is said and done, at the end of one frame we have a little parcel of data like so:

{{< img "images/one-timestep.png" "One frame of data" >}}

This data is stored away and we do everything again for the next frame, storing that too. We repeat this until we have an arbitrary number of frames stored up and then we learn from that.

## Learning from our data

After we've collected a few frames of data, we have this:

{{< img "images/rollout.png" "A few frames of data" >}}

Looking at the rewards, we got 5 on our first frame (quite good), 0 on our second (not good), and 100 on the last (very good!).

So you might think, "well, whatever we did on the last frame is what we need to do all the time! Do that more!". That's along the right track, but not quite right.

The action you took on frame 3 likely wasn't the reason for our big reward. If you got 100 points for winning the game because you destroyed the opponent, then you probably shot the bullet on frame 2 (even though you got 0 reward that frame).

So how do we know which frames were actually important? We don't. If the overall outcome of our batch of frames was good, then we consider the *whole* batch to be good. In essence, we add up the the rewards from each frame and use the total number.

Then, if that total reward was good, we make the series of actions we took more likely. If it was bad, we make them less likely. Repeat that for a few million frames and you have an AI.