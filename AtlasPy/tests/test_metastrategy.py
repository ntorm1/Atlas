from typing import cast
import pandas as pd
import numpy as np

from context import *

from AtlasPy.strategy import PyStrategy, PyMetaStrategy


class SimpleTestStrategy(unittest.TestCase):
    def setUp(self) -> None:
        hydra_path = os.path.join(os.path.dirname(__file__), HYDRA_DIR_MULTI)
        parser = Parser(hydra_path, logging_level=logging.ERROR)
        self.hydra = parser.getHydra()

    def test_strategy_load(self):
        root = self.hydra.getStrategy("root")
        self.assertIsNotNone(root)
        root = cast(PyMetaStrategy, root)


if __name__ == "__main__":
    unittest.main()
