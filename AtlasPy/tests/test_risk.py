import pandas as pd
import numpy as np

from context import *

DATA_PATH = r"C:\Users\natha\OneDrive\Desktop\C++\Atlas\AtlasPy\tests\files\data.csv"


class TestRisk(unittest.TestCase):
    def setUp(self) -> None:
        hydra_path = os.path.join(os.path.dirname(__file__), HYDRA_DIR_SP500)
        parser = Parser(hydra_path)
        self.hydra = parser.getHydra()
        self.exchange = self.hydra.getExchange(EXCHANGE_ID)
        self.intial_cash = 100.0
        self.root_strategy = MetaStrategy("root", self.exchange, None, self.intial_cash)
        self.hydra.addStrategy(self.root_strategy, True)
        self.data = pd.read_csv(DATA_PATH)
        self.data["Date"] = pd.to_datetime(self.data["Date"])
        self.data.set_index("Date", inplace=True)

    def testInit(self):
        self.assertEqual(self.exchange.getName(), EXCHANGE_ID)

    # helper function to run to a specific date
    def runTo(self, date):
        timestamps = self.exchange.getTimestamps()
        timestamps = pd.to_datetime(timestamps, unit="ns")
        index = self.data.index.get_loc(date)
        for i in range(index):
            self.hydra.step()

    def testCovariance(self):
        monthly_trigger_node = StrategyMonthlyRunnerNode.make(self.exchange)
        covariance_node = self.exchange.getCovarianceNode(
            "30_PERIOD_COV", monthly_trigger_node, 30, CovarianceType.FULL
        )

        # data starts 2010-01-04, and the first day of february will not
        # have 20 days of data to calculate covariance, so the first trigger date
        # will be 2010-03-01
        self.hydra.build()
        self.runTo("2010-03-01")

        cov_matrix = covariance_node.getCovarianceMatrix()
        cov_sum = cov_matrix.sum()
        self.assertAlmostEqual(cov_sum, 0.000000000)
        self.hydra.step()
        cov_matrix = covariance_node.getCovarianceMatrix()

        # get the covariance matrix from pandas, the last row should be 2010-03-01
        # and the first row should be 21 rows before that
        end_idx = self.data.index.get_loc("2010-03-01")
        start_idx = end_idx - 30
        matrix_subset = self.data.iloc[start_idx : end_idx + 1, :]
        ordered_columns = sorted(
            matrix_subset.columns, key=lambda x: self.exchange.getAssetMap()[x]
        )
        matrix_subset = matrix_subset[ordered_columns]
        matrix_subset = matrix_subset.pct_change().dropna().cov(ddof=1)

        # assert all values are within 1e-6
        cov_matrix_df = pd.DataFrame(
            cov_matrix, columns=ordered_columns, index=ordered_columns
        )
        # print(cov_matrix_df)
        # print(matrix_subset)
        self.assertTrue(np.allclose(cov_matrix, matrix_subset, atol=1e-8))

    def testRankNode(self):
        monthly_trigger_node = StrategyMonthlyRunnerNode.make(self.exchange)

        asset_read_node = AssetReadNode.make("close", 0, self.exchange)
        asser_read_previouse_node = AssetReadNode.make("close", -1, self.exchange)
        spread = AssetOpNode.make(
            asset_read_node, asser_read_previouse_node, AssetOpType.DIVIDE
        )
        exchange_view = ExchangeViewNode.make(self.exchange, spread)

        # rank assets by 1 period return, flag the bottom 2 and top 2
        rank_node = EVRankNode.make(exchange_view, EVRankType.NEXTREME, 2)

        # short the bottom 2 assets, long the top 2
        allocation = AllocationNode.make(
            rank_node, AllocationType.CONDITIONAL_SPLIT, 0.0
        )

        # build final strategy and insert into hydra
        strategy_node = StrategyNode.make(allocation)
        strategy_node.setTrigger(monthly_trigger_node)
        strategy = ImmediateStrategy(
            self.exchange, self.root_strategy, STRATEGY_ID, 1.0, strategy_node
        )
        _ = self.root_strategy.addStrategy(strategy, True)
        self.runTo("2010-02-01")

        allocation = strategy.getAllocationBuffer()
        self.assertTrue(np.allclose(allocation, np.zeros_like(allocation)))

        self.hydra.step()
        current_time = self.exchange.getCurrentTimestamp()
        current_time = pd.to_datetime(current_time, unit="ns")
        # get data from current time including the previous day
        data = self.data.loc[:current_time]
        returns = data.pct_change().iloc[-1]
        ordered_columns = sorted(
            data.columns, key=lambda x: self.exchange.getAssetMap()[x]
        )
        returns = returns[ordered_columns]
        # flip the two largest to 1 and the two smallest to -1, rest to 0
        largest_indices = returns.nlargest(2).index
        smallest_indices = returns.nsmallest(2).index
        returns.loc[largest_indices] = 1
        returns.loc[smallest_indices] = -1
        returns.loc[~returns.index.isin(largest_indices.union(smallest_indices))] = 0
        returns = returns.values
        returns /= np.abs(returns).sum()
        allocation = strategy.getAllocationBuffer()
        self.assertTrue(np.allclose(allocation, returns))


if __name__ == "__main__":
    unittest.main()
