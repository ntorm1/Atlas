from typing import Optional
from AtlasPy import atlas_internal
from AtlasPy.strategy import PyMetaStrategy, Exchange, Allocator


class TestMetaStrategy(PyMetaStrategy):
    def __init__(
        self,
        exchange: Exchange,
        parent_strategy: Optional[Allocator],
        name: str,
        portfolio_weight: float,
    ) -> None:
        super().__init__(exchange, parent_strategy, name, portfolio_weight)

    def allocate(slef) -> None:
        return super().allocate()
