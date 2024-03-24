import os
import sys

sys.path.insert(
    0,
    os.path.abspath(
        os.path.join(os.path.dirname(os.path.dirname(__file__)), "../../..")
    ),
)

from AtlasPy import atlas_internal
from AtlasPy.strategy import *


class TestStrategy1(PyStrategy):
    def __init__(
        self,
        exchange: Exchange,
        parent_strategy: Allocator,
        name: str,
        portfolio_weight: float,
    ) -> None:
        super().__init__(exchange, parent_strategy, name, portfolio_weight)

    def loadAST(self) -> atlas_internal.ast.AllocationNode:
        close = atlas_internal.ast.AssetReadNode.make("Close", 0, self.exchange)
        ev = atlas_internal.ast.exchangeViewNode.make(self.exchange, close)
        allocation = atlas_internal.ast.AllocationNode.make(ev)
        return atlas_internal.ast.StrategyNode.make(allocation, self.portfolio)
