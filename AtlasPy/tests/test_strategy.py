import pandas as pd
import numpy as np

from context import *


class SimpleTestStrategy(unittest.TestCase):
    def setUp(self) -> None:
        hydra_path = os.path.join(os.path.dirname(__file__), HYDRA_DIR_2)
        parser = Parser(hydra_path)
        self.hydra = parser.getHydra()
        self.exchange = self.hydra.getExchange(EXCHANGE_ID)
        self.portfolio = self.hydra.getPortfolio(PORTFOLIO_ID)

        self.asset_id1 = "asset1"
        self.asset_id2 = "asset2"
        self.intial_cash = 100.0
        self.asset1_index = self.exchange.getAssetIndex(self.asset_id1)
        self.asset2_index = self.exchange.getAssetIndex(self.asset_id2)

        self.asset1_close = [101, 103, 105, 106]
        self.asset2_close = [101.5, 99, 97, 101.5, 101.5, 96]

    def testBuild(self) -> None:
        timestamps = self.exchange.getTimestamps()
        assert len(timestamps) == 6

    def testSimpleAlloc(self) -> None:
        read_close = AssetReadNode.make("close", 0, self.exchange)
        exchange_view = ExchangeViewNode.make(self.exchange, read_close)
        allocation = AllocationNode.make(exchange_view, AllocationType.UNIFORM, 0.0)

        # build final strategy and insert into hydra
        strategy_node = StrategyNode.make(allocation, self.portfolio)
        self.hydra.build()
        strategy = ImmediateStrategy(
            self.exchange, self.portfolio, STRATEGY_ID, 1.0, strategy_node
        )
        _ = self.hydra.addStrategy(strategy, True)
        self.hydra.step()
        nlv = strategy.getNLV()
        assert nlv == self.intial_cash
        self.hydra.step()

        asset_2_return = (
            self.asset2_close[1] - self.asset2_close[0]
        ) / self.asset2_close[0]
        nlv = self.intial_cash * (1 + asset_2_return)
        assert strategy.getNLV() == nlv

        self.hydra.step()
        asset_2_return2 = (
            self.asset2_close[2] - self.asset2_close[1]
        ) / self.asset2_close[1]
        asset_1_return2 = (
            self.asset1_close[1] - self.asset1_close[0]
        ) / self.asset1_close[0]
        avg_return = (0.5 * asset_2_return2) + (0.5 * asset_1_return2)
        nlv *= 1.0 + avg_return
        assert strategy.getNLV() == nlv

    def testFixedAlloc(self) -> None:
        alloc = [(self.asset_id1, 0.3), (self.asset_id2, 0.7)]
        allocation = FixedAllocationNode.make(alloc, self.exchange, 0.0)
        strategy_node = StrategyNode.make(allocation, self.portfolio)

        trigger_node = PeriodicTriggerNode.make(self.exchange, 2)
        strategy_node.setTrigger(trigger_node)
        strategy_node.setWarmupOverride(1)

        self.hydra.build()
        strategy = ImmediateStrategy(
            self.exchange, self.portfolio, STRATEGY_ID, 1.0, strategy_node
        )
        _ = self.hydra.addStrategy(strategy, True)
        commission_manager = strategy.initCommissionManager()
        fixed_commission = 1.0
        pct_commission = 0.001
        commission_manager.setFixedCommission(fixed_commission)
        commission_manager.setCommissionPct(pct_commission)

        # test warmup override
        self.hydra.step()
        allocation = strategy.getAllocationBuffer()
        self.assertTrue(np.allclose(allocation, np.zeros_like(allocation)))
        self.assertAlmostEqual(strategy.getNLV(), self.intial_cash)

        # trigger node prevents alloc on second step so repeat first step
        self.hydra.step()
        self.assertTrue(np.allclose(allocation, np.zeros_like(allocation)))
        self.assertAlmostEqual(strategy.getNLV(), self.intial_cash)

        # now we are 30% in asset1 and 70% in asset2. Two trades needed
        self.hydra.step()
        self.assertTrue(np.allclose(allocation, np.array([0.3, 0.7])))
        commission = 2 * fixed_commission + pct_commission * self.intial_cash
        self.assertAlmostEqual(strategy.getNLV(), self.intial_cash - commission)
        self.assertAlmostEqual(allocation.sum(), 1.0)

        # trigger node prevents alloc
        self.hydra.step()
        # get deepy copy of strategy allocation buffer
        weights_2 = np.copy(strategy.getAllocationBuffer())

        # fixed alloc forced rebalance, should have two fixed commissions and two pct commissions
        # proportional to the size of the rebalance
        nlv = strategy.getNLV()
        self.hydra.step()
        returns = self.exchange.getMarketReturns()
        portfolio_return = np.dot(returns, weights_2)
        nlv = nlv * (1 + portfolio_return)

        weights_delta = np.abs(weights_2 - strategy.getAllocationBuffer())
        commission = 2 * fixed_commission + (pct_commission * nlv * weights_delta).sum()
        self.assertAlmostEqual(allocation.sum(), 1.0)
        self.assertAlmostEqual(strategy.getNLV(), nlv - commission)


class VectorBTCompare(unittest.TestCase):
    def setUp(self) -> None:
        hydra_path = os.path.join(os.path.dirname(__file__), HYDRA_DIR)
        parser = Parser(hydra_path)
        self.hydra = parser.getHydra()
        self.exchange = self.hydra.getExchange(EXCHANGE_ID)
        self.portfolio = self.hydra.getPortfolio(PORTFOLIO_ID)
        self.exchange_path = EXCHANGE_PATH

    def test_max_observer(self):
        n = 5
        close = AssetReadNode.make("Close", 0, self.exchange)
        prev_close = AssetReadNode.make("Close", -1, self.exchange)
        change = AssetOpNode.make(close, prev_close, AssetOpType.SUBTRACT)

        close_max = self.exchange.registerObserver(
            MaxObserverNode("change_max", change, n)
        )
        self.exchange.enableNodeCache("close_max", close_max, False)

        ev = ExchangeViewNode.make(self.exchange, close)
        allocation = AllocationNode.make(ev)
        strategy_node_signal = StrategyNode.make(allocation, self.portfolio)
        strategy = ImmediateStrategy(
            self.exchange, self.portfolio, STRATEGY_ID, 1.0, strategy_node_signal
        )
        _ = self.hydra.addStrategy(strategy, True)
        self.hydra.run()

        ticker = "BTC-USD"
        btc_idx = self.exchange.getAssetIndex(ticker)
        path = os.path.join(self.exchange_path, f"{ticker}.csv")
        df = pd.read_csv(path)
        df["close_max_atlas"] = close_max.cache()[btc_idx].T
        df["close_max_pd"] = df["Close"].diff().rolling(n).max()
        df["close_max_pd"].fillna(0, inplace=True)
        df = df.iloc[n:]
        self.assertTrue(np.allclose(df["close_max_atlas"], df["close_max_pd"]))

    def test_ma_cross(self):
        fast_n = 50
        slow_n = 200
        close = AssetReadNode.make("Close", 0, self.exchange)
        fast_ma = AssetScalerNode(
            self.exchange.registerObserver(SumObserverNode("fast_sum", close, fast_n)),
            AssetOpType.DIVIDE,
            fast_n,
        )
        slow_ma = AssetScalerNode(
            self.exchange.registerObserver(SumObserverNode("slow_sum", close, slow_n)),
            AssetOpType.DIVIDE,
            slow_n,
        )
        spread = AssetOpNode.make(fast_ma, slow_ma, AssetOpType.SUBTRACT)
        spread_filter = ExchangeViewFilter(
            ExchangeViewFilterType.GREATER_THAN, 0.0, None
        )
        exchange_view = ExchangeViewNode.make(self.exchange, spread, spread_filter)

        allocation = AllocationNode.make(exchange_view)
        strategy_node = StrategyNode.make(allocation, self.portfolio)
        strategy = ImmediateStrategy(
            self.exchange, self.portfolio, STRATEGY_ID, 1.0, strategy_node
        )
        _ = self.hydra.addStrategy(strategy, True)
        strategy.enableTracerHistory(TracerType.NLV)
        self.hydra.run()

        nlv = strategy.getHistory(TracerType.NLV)
        returns = nlv[-1] / nlv[0] - 1
        # self.assertAlmostEqual(returns, 1.5693292786263853)

    def test_ma_signal(self):
        fast_n = 50
        slow_n = 200
        close = AssetReadNode.make("Close", 0, self.exchange)
        fast_ma = AssetScalerNode(
            self.exchange.registerObserver(SumObserverNode("fast_sum", close, fast_n)),
            AssetOpType.DIVIDE,
            fast_n,
        )
        slow_ma = AssetScalerNode(
            self.exchange.registerObserver(SumObserverNode("slow_sum", close, slow_n)),
            AssetOpType.DIVIDE,
            slow_n,
        )
        spread = AssetOpNode.make(fast_ma, slow_ma, AssetOpType.SUBTRACT)

        spread_filter_up = ExchangeViewFilter(
            ExchangeViewFilterType.GREATER_THAN, 0.0, None
        )
        spread_filter_down = ExchangeViewFilter(
            ExchangeViewFilterType.LESS_THAN, 0.0, None
        )
        exchange_view_down = ExchangeViewNode.make(
            self.exchange, spread, spread_filter_down
        )
        exchange_view_up = ExchangeViewNode.make(
            self.exchange, spread, spread_filter_up, exchange_view_down
        )

        exchange_view = ExchangeViewNode.make(self.exchange, spread, None)
        self.exchange.enableNodeCache("ev", exchange_view, False)
        self.exchange.enableNodeCache("ev_signal", exchange_view_up, False)

        allocation = AllocationNode.make(
            exchange_view, AllocationType.CONDITIONAL_SPLIT, 0.0
        )
        allocation_signal = AllocationNode.make(
            exchange_view_up, AllocationType.CONDITIONAL_SPLIT, 0.0
        )

        strategy_node = StrategyNode.make(allocation, self.portfolio)
        strategy_node_signal = StrategyNode.make(allocation_signal, self.portfolio)

        strategy = ImmediateStrategy(
            self.exchange, self.portfolio, STRATEGY_ID, 1.0, strategy_node
        )

        strategy_signal = ImmediateStrategy(
            self.exchange,
            self.portfolio,
            STRATEGY_ID + "_signal",
            1.0,
            strategy_node_signal,
        )
        _ = self.hydra.addStrategy(strategy, True)
        _ = self.hydra.addStrategy(strategy_signal, True)

        strategy.enableTracerHistory(TracerType.NLV)
        strategy.enableTracerHistory(TracerType.ORDERS_EAGER)

        strategy_signal.enableTracerHistory(TracerType.NLV)
        strategy_signal.enableTracerHistory(TracerType.ORDERS_EAGER)

        self.hydra.run()

        cache = exchange_view.cache()
        cache_signal = exchange_view_up.cache()
        # signal only update when opposite ev is triggered so cache is not equal
        self.assertFalse(np.allclose(cache, cache_signal))

        tracer1 = strategy.getTracer()
        tracer2 = strategy_signal.getTracer()
        orders1 = tracer1.getOrders()
        orders2 = tracer2.getOrders()
        df_orders1 = pd.DataFrame([order.to_dict() for order in orders1])
        df_orders2 = pd.DataFrame([order.to_dict() for order in orders2])
        df_orders1["fill_time"] = pd.to_datetime(df_orders1["fill_time"], unit="ns")
        df_orders2["fill_time"] = pd.to_datetime(df_orders2["fill_time"], unit="ns")

        self.assertEqual(len(orders1), len(orders2))

        nlv1 = strategy.getHistory(TracerType.NLV)[-1]
        nlv2 = strategy_signal.getHistory(TracerType.NLV)[-1]
        self.assertAlmostEqual(nlv1, nlv2)

        self.hydra.reset()
        self.hydra.run()

        nlv1_reset = strategy.getHistory(TracerType.NLV)[-1]
        nlv2_reset = strategy_signal.getHistory(TracerType.NLV)[-1]
        self.assertAlmostEqual(nlv1_reset, nlv2_reset)
        self.assertAlmostEqual(nlv1, nlv1_reset)

    def test_supertrend(self):
        multiplier = 3
        close = AssetReadNode.make("Close", 0, self.exchange)
        previous_close = AssetReadNode.make("Close", -1, self.exchange)

        atr_node = ATRNode.make(self.exchange, "High", "Low", 14)
        median_node = AssetMedianNode.make(
            self.exchange,
            "High",
            "Low",
        )

        # ===== Lower Band =====
        lower_band = AssetOpNode.make(
            median_node,
            AssetScalerNode(atr_node, AssetOpType.MULTIPLY, multiplier),
            AssetOpType.SUBTRACT,
        )
        lower_left_cond = AssetIfNode(
            lower_band, AssetCompType.GREATER, DummyNode(self.exchange)
        )
        lower_right_cond = AssetIfNode(
            previous_close, AssetCompType.LESS, DummyNode(self.exchange)
        )
        final_lower_band = AssetCompNode(
            lower_left_cond,
            LogicalType.OR,
            lower_right_cond,
            lower_band,
            DummyNode(self.exchange),
        )
        lagged_final_lower_band = final_lower_band.lag(1)
        lower_left_cond.swapRightEval(lagged_final_lower_band)
        lower_right_cond.swapRightEval(lagged_final_lower_band)
        final_lower_band.swapFalseEval(lagged_final_lower_band)
        # ======================

        # ===== Upper Band =====
        upper_band = AssetOpNode.make(
            median_node,
            AssetScalerNode(atr_node, AssetOpType.MULTIPLY, multiplier),
            AssetOpType.ADD,
        )
        upper_left_cond = AssetIfNode(
            upper_band, AssetCompType.LESS, DummyNode(self.exchange)
        )
        upper_right_cond = AssetIfNode(
            previous_close, AssetCompType.GREATER, DummyNode(self.exchange)
        )
        final_upper_band = AssetCompNode(
            upper_left_cond,
            LogicalType.OR,
            upper_right_cond,
            upper_band,
            DummyNode(self.exchange),
        )
        lagged_final_upper_band = final_upper_band.lag(1)
        upper_left_cond.swapRightEval(lagged_final_upper_band)
        upper_right_cond.swapRightEval(lagged_final_upper_band)
        final_upper_band.swapFalseEval(lagged_final_upper_band)
        # ======================

        self.exchange.enableNodeCache("final_lower_band", final_lower_band, True)
        self.exchange.enableNodeCache("final_upper_band", final_upper_band, True)
        self.assertAlmostEqual(final_lower_band.cache()[0][-1], 40304.66724192)
        self.assertAlmostEqual(final_upper_band.cache()[0][15], 16137.834342208951)
        self.assertAlmostEqual(final_upper_band.cache()[0][16], 14852.429043211883)
        self.assertAlmostEqual(final_upper_band.cache()[0][16], 14852.429043211883)


if __name__ == "__main__":
    unittest.main()
