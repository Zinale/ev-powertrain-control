import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('dati_sensori_20260506_121000.csv', on_bad_lines='skip') 

df = df.apply(pd.to_numeric, errors='coerce')

df = df.dropna()

df = df[df['Time_ms'] >= df['Time_ms'].shift().fillna(0)]

plt.plot(df['Time_ms'], df['Speed'])
plt.show()