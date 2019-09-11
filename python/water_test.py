from typing import List

from celluloid import Camera
import matplotlib.pyplot as plt
import numpy as np


def process_water(source: np.array, dest: np.array):
    for i in range(320, 64000-320):
        dest[i] = (
            ((source[i-1] +
              source[i+1] +
              source[i-320] +
              source[i+320]) >> 1)) - dest[i]

        dest[i] -= (dest[i] >> 5)


def main():
    buffers = [np.zeros((64000), dtype=int), np.zeros((64000), dtype=int)]

    fig = plt.figure()
    camera = Camera(fig)

    active_buffer = 0
    for _ in range(100):
        buffers[active_buffer][
            np.random.randint(0, 64000)] = np.random.randint(1, 10000)
        process_water(buffers[active_buffer],
                      buffers[(active_buffer + 1) % 2])
        plt.imshow(buffers[active_buffer].reshape((200, 320)),
                   cmap='Greys',
                   vmin=0,
                   vmax=1000)
        camera.snap()
        active_buffer = (active_buffer + 1) % 2

    animation = camera.animate()
    animation.save('animation.gif')


if __name__ == '__main__':
    main()
