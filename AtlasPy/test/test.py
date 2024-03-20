import numpy as np
import scipy.stats as stats


def skewness(data):
    n = len(data)
    mean = np.mean(data)
    skewness = sum((data - mean) ** 3) / ((n) * (np.var(data) ** (3 / 2)))
    return skewness


# Example usage:
data = np.random.normal(0, 1, 15)
window = 10

scipy_skew = stats.skew(data)
custom_skew = skewness(data)

print("Custom skewness:", custom_skew)
print("Scipy skewness:", scipy_skew)
