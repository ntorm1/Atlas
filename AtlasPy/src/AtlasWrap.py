import sys
import os
import unittest
import pandas as pd
import numpy as np

atlas_path = "C:/Users/natha/OneDrive/Desktop/C++/Atlas/x64/Debug"
sys.path.append(atlas_path)


from AtlasPy.core import Hydra, Portfolio, Strategy
from AtlasPy.ast import *


exchange_path_sp500_ma = r"C:/Users/natha/OneDrive/Desktop/C++/Atlas/AtlasPy/test/data_sp500_ma.h5"


class RiskTest(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        self.data = pd.read_csv("data.csv")
        self.data["Date"] = pd.to_datetime(self.data["Date"])
        self.data.set_index("Date", inplace=True)
        self.hydra = Hydra()
        self.exchange = self.hydra.addExchange("test", exchange_path_sp500_ma)
        self.portfolio = self.hydra.addPortfolio("test_p", self.exchange, 100.0)
    def testInit(self):
        self.assertEqual(self.exchange.getName(),"test")

    def testCovariance(self):
        monthly_trigger_node = StrategyMonthlyRunnerNode.make(self.exchange)
        covariance_node = self.exchange.getCovarianceNode("20_PERIOD_COV", monthly_trigger_node, 30)
        timestamps = self.exchange.getTimestamps()
        timestamps = pd.to_datetime(timestamps, unit="ns")

        
        # data starts 2010-01-04, and the first day of february will not 
        # have 20 days of data to calculate covariance, so the first trigger date
        # will be 2010-03-01
        index = 0 
        for i in range(len(timestamps)):
            if timestamps[i].year == 2010 and timestamps[i].month == 3:
                index= i
                break
        self.hydra.build()
        for i in range(0,index):
            self.hydra.step()

        cov_matrix = covariance_node.getCovarianceMatrix()
        cov_sum = cov_matrix.sum()
        self.assertAlmostEqual(cov_sum, 0.000000000)
        self.hydra.step()
        cov_matrix = covariance_node.getCovarianceMatrix()

        # get the covariance matrix from pandas, the last row should be 2010-03-01
        # and the first row should be 21 rows before that
        end_idx = self.data.index.get_loc("2010-03-01")
        start_idx = end_idx - 30
        matrix_subset = self.data.iloc[start_idx:end_idx+1,:]
        ordered_columns = sorted(matrix_subset.columns, key=lambda x: self.exchange.getAssetMap()[x])
        matrix_subset = matrix_subset[ordered_columns]
        matrix_subset = matrix_subset.pct_change().dropna().cov(ddof=1)

        # assert all values are within 1e-6
        self.assertTrue(np.allclose(cov_matrix, matrix_subset, atol=1e-8))
        
        


if __name__ == "__main__":
    unittest.main()