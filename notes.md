Flow
====
- TrainingManager creates/gets N Environments from the scene.
- TrainingManager creates Trainer, registering every environment in the constructor.
- TrainingManager tells Trainer to `BeginTraining()`.
- Trainer tells each environment to `BeginTraining()`.
- An Environment puts an observation into the Queue.
  ```
  {
    environment: <The environment that sent the observation>,
    agentNumber: 1,
    observation: [[0.1, ..., 1.3], [0.5, ..., 1.2]]
  }
  ```
- Trainer gets the observation from the Queue and gets the action from the server.
- Trainer calls `Act(actions)` on Environment.agents[agentNumber].
  `actions: [1, 5, 3]`
  - Agent converts action numbers into concrete ICommands.
  - Agent executes Commands.
- During the `Update()` cycle, Environment
- Trainer calls `GetReward(agentNumber)` on the Environment to get a reward.
- Trainer sends the reward to the server.
- If batch_size has been reached, Trainer tells {every Environment | someone else?} to `Pause()`.
  - Trainer calls `train()` on the server.
  - Trainer calls `UnPause()` on {every Environment | the other thing}.