"""
Quick tests for the Elo formula
"""
from enum import Enum
import math
import random
from typing import List
import matplotlib.pyplot as plt

NUMBER_OF_TRIALS = 10
ELO_CONSTANT = 16


class EvaluationResult(Enum):
    """
    Enum containing possible game results
    """
    A = 0
    B = 1
    Draw = 2


def expected_win_chance(a_rating: float, b_rating: float) -> float:
    """
    Calculate A's expected chance of winning [0-1].
    """
    return 1. / (1. + (10 ** ((b_rating - a_rating) / 400.)))


def calculate_elos(
        a_rating: float,
        b_rating: float,
        k: float,
        result: EvaluationResult):
    """
    Calculate new elos based on ratings, an Elo constant and the game result.
    """
    a_win_chance = expected_win_chance(a_rating, b_rating)
    b_win_chance = 1. - a_win_chance

    if result == EvaluationResult.A:
        a_rating = a_rating + k * (1 - a_win_chance)
        b_rating = b_rating + k * (0 - b_win_chance)
    elif result == EvaluationResult.B:
        a_rating = a_rating + k * (0 - a_win_chance)
        b_rating = b_rating + k * (1 - b_win_chance)
    else:
        a_rating = a_rating + k * (0.5 - a_win_chance)
        b_rating = b_rating + k * (0.5 - b_win_chance)

    return (a_rating, b_rating)


class Agent:
    """
    A mock agent.
    Contains a name and a power.
    """

    def __init__(self, power: float, name: str):
        self.power = power
        self.name = name


def trial(agent_a: Agent, agent_b: Agent) -> EvaluationResult:
    """
    Run a trial between two agents
    """
    result = (random.random() * 50) - 25 + agent_a.power
    if result > agent_b.power + 25:
        return EvaluationResult.A
    if result < agent_b.power - 25:
        return EvaluationResult.B
    return EvaluationResult.Draw


class EloEvaluator:
    """
    Calculates the Elo scores of a pool of Agents.
    """

    def __init__(self):
        self.main_agent = None
        self.opponents = []
        self.elos = {}

    def evaluate(self, agent: Agent, new_opponents: List[Agent]) -> float:
        """
        Add some new Agents to the pool, evaluate their Elo, then evaluate the
        main Agent's Elo against the whole pool.
        """
        if not self.main_agent:
            self.main_agent = agent
            self.elos[self.main_agent] = 0
        else:
            self.main_agent = agent

        # Add new opponents to opponent pool
        for opponent in new_opponents:
            self.opponents.append(opponent)
            self.elos[opponent] = 0

        # Calculate new opponent elos
        for agent_a in new_opponents:
            # Run new opponent against current pool
            results = {}
            for agent_b in self.opponents:
                results[(agent_a, agent_b)] = [
                    trial(agent_a, agent_b)
                    for _ in range(NUMBER_OF_TRIALS)]

            # Calculate elos
            for agents in results:
                for result in results[agents]:
                    (self.elos[agents[0]],
                     self.elos[agents[1]]) = calculate_elos(
                        self.elos[agents[0]],
                        self.elos[agents[1]],
                        ELO_CONSTANT,
                        result)

        # Run main agent against current pool
        results = {}
        for opponent in self.opponents:
            results[opponent] = [trial(self.main_agent, opponent)
                                 for _ in range(NUMBER_OF_TRIALS)]

        # Calculate elos
        for opponent in results:
            for result in results[opponent]:
                (self.elos[self.main_agent],
                 self.elos[opponent]) = calculate_elos(
                    self.elos[self.main_agent],
                    self.elos[opponent],
                    ELO_CONSTANT,
                    result)

        print(f"Main agent: {self.elos[self.main_agent]}")

        return self.elos[self.main_agent]


def main():
    """
    Calculate the Elos for agents slowly increasing in power.
    """
    evaluator = EloEvaluator()
    main_agent = Agent(0, "Main")
    elos = []
    for i in range(1000):
        power = math.sqrt(i)
        main_agent.power = power
        elos.append(evaluator.evaluate(main_agent, [Agent(power, f"{power}")]))
    plt.plot(elos)
    plt.show()


if __name__ == '__main__':
    main()
