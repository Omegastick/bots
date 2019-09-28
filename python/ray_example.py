import ray
import ray.tune as tune
import singularity_trainer as st


def train(_):
    pass


def main():
    ray.init()
    tune.run(train,
             name="test",
             num_samples=10)


if __name__ == '__main__':
    main()
