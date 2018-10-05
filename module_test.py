import random
import itertools


class Action:
    def __init__(self):
        self.options = random.randint(1, 10)

    def act(self, option):
        assert option < self.options, f"{option}, {self.options}"
        print("Pew")


class Module:
    def __init__(self, children_count):
        if random.random() < 0.4:
            self.children = [Module(random.randint(1, 4))
                             for _ in range(children_count)]
        else:
            self.children = []
        self.actions = [Action() for _ in range(random.randint(0, 3))]

    def traverse(self, modules=None):
        if modules is None:
            modules = []
        for child in self.children:
            child.traverse(modules)
        modules.append(self)
        return modules


class BaseModule(Module):
    def __init__(self, children_count):
        self.children = [Module(random.randint(3, 6))
                         for _ in range(children_count)]
        self.actions = [Action() for _ in range(random.randint(0, 3))]


class Agent:
    def __init__(self):
        self.base_module = BaseModule(3)


def main():
    agent = Agent()
    modules = agent.base_module.traverse()
    print(len(modules))
    required_actions = [x.actions for x in modules]
    required_actions = list(itertools.chain.from_iterable(required_actions))
    actions = []
    for action in required_actions:
        actions.append(random.randint(0, action.options - 1))
    for i, action in enumerate(actions):
        required_actions[i].act(action)


if __name__ == '__main__':
    main()
