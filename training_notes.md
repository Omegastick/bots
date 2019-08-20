# 2019-7-26
## Hyperparameters
```
"hyper_parameters": {
        "actor_loss_coef": 0.666,
        "algorithm": 1,
        "batch_size": 25600,
        "clip_param": 0.2,
        "discount_factor": 0.99,
        "entropy_coef": 0.000001,
        "learning_rate": 0.0007,
        "num_env": 8,
        "num_epoch": 6,
        "num_minibatch": 80,
        "value_loss_coef": 0.333
    }
```
KL target: 0.01

## Observations
Elo climbs initially, but after a while diverges.
Hit a peak of 65 after 1.5 hours, then regressed back to zero for the next 3 hours. Shortly after diverged (-251) but then recovered (0) for a short while, before diverging again completely (-412).
Training divergence was accompanied by high KL-divergence (0.015-0.016 average). KL-divergence during improvement was particularly low (0.006).

## Ideas
Lowering KL-divergence might improve training stability.
3.5 ways to lower KL-divergence: Smaller clip parameter, smaller learning rate, smaller KL target. Increasing batch size might also help, but I don't have much data and I'm not sure why it would help anyway.
A smaller clip parameter might be the best of these, it is designed to reduce the KL-divergence.
A smaller learning rate is another good option, if a smaller clip parameter doesn't work I'll try this.
A smaller KL target is a bit heavy-handed IMO. The early-stopping mechanism is damage control, not proactive. I think it would just hide the underlying problem.
I'm not going to try a larger batch size because the batch size is already pretty big, and further increasing it would reduce sample efficiency by a lot.
On the other hand, KL-divergence might just be an indicator of some other thing that's hurting training.

## Plan
First, find out what happens when training diverges.
Need to add more logging, but also inspect the checkpoints manually.
If that doesn't turn up anything big, I'll reduce the clip parameter to 0.1 (same as Atari).

# 2019-7-28
## Hyperparameters
```
"hyper_parameters": {
        "actor_loss_coef": 0.666,
        "algorithm": 1,
        "batch_size": 6400,
        "clip_param": 0.1,
        "discount_factor": 0.99,
        "entropy_coef": 0.001,
        "learning_rate": 0.001,
        "num_env": 8,
        "num_epoch": 6,
        "num_minibatch": 80,
        "value_loss_coef": 0.333
    }
```
KL target: 0.01

## Observations
It learned quite nicely.
Elo peaked (83) after 2.3 hours, and then flattened out afterwards.
Elo *very* unstable the whole time. Not just evaluation instability, the model's ability actually fluctuated.
Entropy stopped decreasing at 0.57 after 45 minutes.
One very unusual thing happened, which needs more investigation.
Sometimes, a training run has very high action loss, high update duration, low value loss, low clip fraction, low KL divergence, and the entropy doesn't decrease.
This run started off normal, then, after 45 minutes, switched to the mode described above.
The observed behaviour at peak Elo was good.
It consistently got itself unstuck when it got stuck on the walls.
It focused on the hill (as expected) and consistently managed to make it's way to the hill every episode.

## Ideas
The mode talked about above needs to be looked into, I get the feeling that it is very important.
I currently have no idea whatsoever what could be causing it.
Originally, I thought it was because there was no learning signal to use, but there definitely is one in this case.
Another question is how does the agent learn while in that state? The updates seem to be very minimal.
It looks like the agent learns something, because it reached it's highest Elo while in that state.
I thought perhaps it was because the entropy can't decrease below it's current point due to the entropy bonus in the loss function.
But that's not how entropy works. There are also cases of it happening while entropy is still at maximum.
Perhaps ReLU is causing dead layers?
I'll have to debug it.

## Plan
Regardless of the weird mode described above, training worked to an acceptable level.
This means I'll move onto other features.
I'll work on multiplayer next.
I don't feel like doing that this weekend, so I'll try a run with the GRU turned on first.

# 2019-8-20 1
## Hyperparameters
```
"hyper_parameters": {
        "actor_loss_coef": 0.666,
        "algorithm": 1,
        "batch_size": 6400,
        "clip_param": 0.1,
        "discount_factor": 0.99,
        "entropy_coef": 0.001,
        "learning_rate": 0.001,
        "num_env": 8,
        "num_epoch": 6,
        "num_minibatch": 80,
        "value_loss_coef": 0.333
    }
```
KL target: 0.01

## Observations
I tried the above hyperparameters again with the timestep length bug fixed.
It trained wonderfully for the first 20 minutes (200 updates) but diverged after that.
KL divergence rises quickly, and after about 20 minutes it is having to early stop every update.

## Ideas
Probably diverging because of high KL divergence.

## Plan
I'm going to try lowering the learning rate to 0.0003 and see how that works out.

# 2019-8-20 2
## Hyperparameters
```
"hyper_parameters": {
        "actor_loss_coef": 0.666,
        "algorithm": 1,
        "batch_size": 6400,
        "clip_param": 0.1,
        "discount_factor": 0.99,
        "entropy_coef": 0.0003,
        "learning_rate": 0.001,
        "num_env": 8,
        "num_epoch": 6,
        "num_minibatch": 80,
        "value_loss_coef": 0.333
    }
```
KL target: 0.01

## Observations
I made a mistake and instead lowered the entropy coefficient instead of the learning rate.
No real training progress for the first 40 minutes, then a spike in performance.
Unfortunately, the training crashed after an hour because of a bug.
KL divergence stayed low for the first 20 minutes, then rose and remained high for the rest of the run.

## Ideas
KL divergence is still getting uncomfortably high, need to lower it.

## Plan
I'll try reducing the batch size to 3200 and see if that improves the KL divergence.

# 2019-8-20 3
## Hyperparameters
```
"hyper_parameters": {
        "actor_loss_coef": 0.666,
        "algorithm": 1,
        "batch_size": 6400,
        "clip_param": 0.1,
        "discount_factor": 0.99,
        "entropy_coef": 0.0003,
        "learning_rate": 0.001,
        "num_env": 8,
        "num_epoch": 6,
        "num_minibatch": 80,
        "value_loss_coef": 0.333
    }
```
KL target: 0.01

## Observations
Pretty bad performance for the whole run, with a particularly bad spike at 50 minutes.
KL-divergence rose quite quickly.

## Ideas
[ToriLLE: Learning Environment for Hand-to-Hand Combat](https://arxiv.org/pdf/1807.10110.pdf) says that a batch size of 512 worked well, so I'm going to try and copy their hyperparameters.

## Plan
I'll try reducing the batch size to 512,increasing entropy to 0.01, and reducing learning rate to 0.0003.

# 2019-8-20 4
## Hyperparameters
```
"hyper_parameters": {
        "actor_loss_coef": 0.666,
        "algorithm": 1,
        "batch_size": 512,
        "clip_param": 0.1,
        "discount_factor": 0.99,
        "entropy_coef": 0.01,
        "learning_rate": 0.0003,
        "num_env": 8,
        "num_epoch": 6,
        "num_minibatch": 16,
        "value_loss_coef": 0.333
    }
```
KL target: 0.01

## Observations
Elo increased very slowly.
Entropy barely decreased at all.
KL-divergence very low consistently.
Low clip-rate.
No KL early stops at all.

## Ideas
It's obviously hardly learning at all, I should increase the learning rate or decrease entropy.

## Plan
I'll increase the learning rate back to 0.001.

# 2019-8-20 5
## Hyperparameters
```
"hyper_parameters": {
        "actor_loss_coef": 0.666,
        "algorithm": 1,
        "batch_size": 512,
        "clip_param": 0.1,
        "discount_factor": 0.99,
        "entropy_coef": 0.01,
        "learning_rate": 0.001,
        "num_env": 8,
        "num_epoch": 6,
        "num_minibatch": 16,
        "value_loss_coef": 0.333
    }
```
KL target: 0.01

## Observations
Elo was static at ~+15, and slowly dropped off towards the end of the run.
Clip fraction was reasonable at ~0.06 the whole time.
KL-divergence also quite reasonable.
KL-diverge early stopping happened quite often.
Entropy dropped very slowly.

## Ideas
The run was fine, apart from the too-slow entropy drop.
I need to speed up convergence a bit

## Plan
I'll drop the entropy coefficient back down to 0.001.

