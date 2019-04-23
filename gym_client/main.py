#!/usr/bin/env python
"""
Main script.
"""
import argparse
import logging
from training_server.train import HyperParams

from gym_client.train import Trainer
from gym_client.client import Client


def main():
    """
    Train an agent on an environment.
    """
    logging.basicConfig(level=logging.DEBUG,
                        format=('%(asctime)s %(funcName)s '
                                '[%(levelname)s]: %(message)s'),
                        datefmt='%Y%m%d %H:%M:%S')

    parser = argparse.ArgumentParser(
        description="Gym client for training_server")
    parser.add_argument('--env_name', type=str, default='CartPole-v1')
    parser.add_argument('--env_type', type=str, default='linear')
    parser.add_argument('--max_frames', type=int, default=1000000)
    parser.add_argument('--num_environments', type=int, default=16)
    parser.add_argument('--learning_rate', type=float, default=2.5e-4)
    parser.add_argument('--batch_size', type=int, default=128)
    parser.add_argument('--num_minibatch', type=int, default=4)
    parser.add_argument('--epochs', type=int, default=4)
    parser.add_argument('--discount_factor', type=float, default=0.99)
    parser.add_argument('--gae', type=float, default=0.95)
    parser.add_argument('--critic_coef', type=float, default=0.5)
    parser.add_argument('--entropy_coef', type=float, default=0.01)
    parser.add_argument('--max_grad_norm', type=float, default=0.5)
    parser.add_argument('--clip_factor', type=float, default=0.2)
    parser.add_argument('--use_gpu', action='store_true', default=False)
    parser.add_argument('--normalize_rewards', action='store_true',
                        default=False)

    args = parser.parse_args()

    hyperparams = HyperParams(
        learning_rate=args.learning_rate,
        batch_size=args.batch_size,
        num_minibatch=args.num_minibatch,
        epochs=args.epochs,
        discount_factor=args.discount_factor,
        gae=args.gae,
        critic_coef=args.critic_coef,
        entropy_coef=args.entropy_coef,
        max_grad_norm=args.max_grad_norm,
        clip_factor=args.clip_factor,
        use_gpu=args.use_gpu,
        normalize_rewards=args.normalize_rewards
    )

    client = Client()

    trainer = Trainer(hyperparams, args.num_environments, args.env_name,
                      args.env_type, client)
    try:
        trainer.train(args.max_frames)
    except Exception:
        import pdb
        pdb.post_mortem()
    finally:
        trainer.end_session()


if __name__ == '__main__':
    main()
