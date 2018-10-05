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
- In `FixedUpdate()`, the Trainer gets the observation from the Queue and gets the relevant action from the server.
- Trainer calls `SendActions(agentNumber, actions)` on the Environment.
  `actions: [1, 5, 3]`
  - Agent converts action numbers into concrete ICommands.
  - Agent executes Commands.
- Trainer calls `GetReward(agentNumber)` on the Environment to get a reward.
- Trainer sends the reward to the server.
- If batch_size has been reached, Trainer tells {every Environment | someone else?} to `Pause()`.
  - Trainer calls `train()` on the server.
  - Trainer calls `UnPause()` on {every Environment | the other thing}.

FixedUpdate() cycle tasks
=========================
- Agent puts an observation in the Trainer's Queue.
- Agent does miscellaneous processing.
  - If necessary, the Agent tells the Environment to `ChangeReward(agent, rewardDelta)`.
- Projectiles check for hits.
  - If a Projectile hits an Agent, calls `Hit(projectile)`.
    - `Hit(projectile)` checks damage, status effects, etc.
    - If necessary, the Agent tells the Environment to `ChangeReward(agent, rewardDelta)`.
- Trainer processes Queue and gives all Agents their actions.
  - Agents process actions as in 'Agent action Flow'

Agent creation flow
===================
- Agent GameObject is instantiated with a tree of Modules (serialised somehow).
  - Each Module has a list of ModuleAttachments.
    - Each ModuleAttachment has a location (relative to parent), a rotation (relative to parent) and an optional child Module (with its own ModuleAttachment.
  - In the Module's constructor it can make changes to its parent or the BaseModule.
  - After all Modules are instantiated, the Agent traverses the tree of modules, registering any Actions and Sensors the Module has.