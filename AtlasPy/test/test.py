import sys

atlas_py_path = "C:/Users/natha/OneDrive/Desktop/C++/Atlas/x64/Release"
sys.path.append(atlas_py_path)

from AtlasPy.core import Hydra


exchange_id = "test"
portfolio_id = "test_p"
strategy_id = "test_s"
exchange_path_sp500 = "C:/Users/natha/OneDrive/Desktop/C++/Atlas/AtlasTest/scripts/data_sp500.h5"
initial_cash = 100.0


hydra = Hydra()
print("Hydra created")
exchange = hydra.addExchange(exchange_id, exchange_path_sp500)

print("DONE")