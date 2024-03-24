import pandas as pd
import numpy as np
import statsmodels.api as sm


from context import *


class SimpleTestStrategy(unittest.TestCase):
    def setUp(self) -> None:
        hydra_path = os.path.join(os.path.dirname(__file__), HYDRA_DIR_MULTI)
        parser = Parser(hydra_path)
        self.hydra = parser.getHydra()

    def test_strategy_load(self):
        pass


if __name__ == "__main__":
    unittest.main()
