import os
import sys
import logging
from abc import ABC, abstractmethod

from graphviz import Digraph

import atlas_internal
from atlas_internal.core import Hydra, Exchange, Allocator, Strategy


class PyStrategy(Strategy):
    parent_strategy: Allocator = None
    exchange: Exchange = None

    def __init__(
        self,
        exchange: Exchange,
        parent_strategy: Allocator,
        name: str,
        portfolio_weight: float,
    ) -> None:
        super().__init__(name, exchange, parent_strategy, portfolio_weight)
        self.exchange = exchange
        self.parent_strategy = parent_strategy

    @abstractmethod
    def loadAST(self) -> atlas_internal.ast.StrategyBufferOpNode:
        pass


class ImmediateStrategy(PyStrategy):
    ast: atlas_internal.ast.StrategyBufferOpNode = None

    def __init__(
        self,
        exchange: Exchange,
        parent_strategy: Allocator,
        name: str,
        portfolio_weight: float,
        ast: atlas_internal.ast.StrategyBufferOpNode,
    ) -> None:
        super().__init__(
            exchange=exchange,
            parent_strategy=parent_strategy,
            name=name,
            portfolio_weight=portfolio_weight,
        )
        self.exchange = exchange
        self.ast = ast

    def loadAST(self) -> atlas_internal.ast.StrategyBufferOpNode:
        return self.ast
