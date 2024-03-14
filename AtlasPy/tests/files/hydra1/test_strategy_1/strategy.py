import os
import sys
import logging

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(os.path.dirname(__file__)), '../../..')))

from AtlasWrap import AtlasPy


def ast(
        exchange: AtlasPy.core.Exchange
        ) -> AtlasPy.ast.AllocationNode:
        window = 5
        close = AtlasPy.ast.AssetReadNode.make("Close", 0, exchange)
        prev_close = AtlasPy.ast.AssetReadNode.make("Close", -1, exchange)
        change = AtlasPy.ast.AssetOpNode.make(
            close,
            prev_close,
            AtlasPy.ast.AssetOpType.SUBTRACT
        )
        close_arg_max = exchange.registerObserver(AtlasPy.ast.TsArgMaxObserverNode("arg_max", change, window))
        close_max = exchange.registerObserver(AtlasPy.ast.MaxObserverNode("max", change, window))
        exchange.enableNodeCache("close_arg_max",close_arg_max, False)
        exchange.enableNodeCache("close_max",close_max, False)

        ev = AtlasPy.ast.ExchangeViewNode.make(exchange, close)
        return AtlasPy.ast.AllocationNode.make(
            ev
        )