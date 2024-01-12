import os
import numpy as np 
import pandas as pd
import h5py

T0 = 960163200
T1 = 960249600
T2 = 960336000
T3 = 960422400
T4 = 960508800
T5 = 960768000


asset1_index = np.array([T1, T2, T3, T4])
asset1_open = np.array([100,102,104,105])
asset1_close = np.array([101,103,105,106])
asset1_df = pd.DataFrame({'open': asset1_open, 'close': asset1_close}, index=asset1_index)

asset2_index = np.array([T0, T1, T2, T3, T4, T5])
asset2_open = np.array([101,100,98,101,101,103])
asset2_close = np.array([101.5,99,97,101.5,101.5,96])
asset2_df = pd.DataFrame({'open': asset2_open, 'close': asset2_close}, index=asset2_index)

asset1_df = asset1_df.astype('float64')
asset2_df = asset2_df.astype('float64')
asset1_df.index = asset1_df.index.astype('int64')
asset2_df.index = asset2_df.index.astype('int64')
asset1_df.index.name = 'timestamp'
asset2_df.index.name = 'timestamp'

output_path = os.path.join(os.getcwd(), 'data.h5')
if os.path.exists(output_path):
    os.remove(output_path)

df_map = {'asset1': asset1_df, 'asset2': asset2_df}

with h5py.File(output_path, "a") as file:
    for ticker, df in df_map.items():
        data = df.to_numpy()
        dataset = file.create_dataset(f"{ticker}/data", data=data)
        file.create_dataset(
                f"{ticker}/datetime",
                data=df.index.map(lambda timestamp: int(timestamp * 1e9)).to_numpy()
        )
        for column in df.columns:
            dataset.attrs[column] = column