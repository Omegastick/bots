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