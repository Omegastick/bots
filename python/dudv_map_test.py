"""
Du/dv map test.
"""
import numpy as np
import tifffile

IMAGE_SIZE = 1000
IMAGE_CENTER = np.array((IMAGE_SIZE / 2, IMAGE_SIZE / 2))


def main():
    """
    Make a test du/dv map.
    """
    # image = np.full((3, IMAGE_SIZE, IMAGE_SIZE), 0.5, np.float32)
    # for x in range(IMAGE_SIZE):
    #     for y in range(IMAGE_SIZE):
    #         distortion = (np.array((x, y)) - IMAGE_CENTER) / IMAGE_SIZE
    #         distance = np.linalg.norm(distortion)
    #         distortion *= np.clip(0.5 - distance, 0, 1)
    #         image[0:2, x, y] = distortion + 0.5

    image = np.random.rand(500, 500, 3).astype(np.float32)
    tifffile.imsave('dudv.tiff', image)


if __name__ == '__main__':
    main()
