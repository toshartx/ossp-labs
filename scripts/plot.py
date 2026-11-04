import pandas as pd
import matplotlib.pyplot as plt

# Read data
df = pd.read_csv('../data/results.csv')
df['Avg'] = df[['Run1', 'Run2', 'Run3']].mean(axis=1)

# Graph 1: Comparison of sync modes (Bar chart)
plt.figure(figsize=(8, 5))
plt.bar(df['Config'] + " (" + df['Mode'] + ")", df['Avg'], color=['#4c72b0', '#4c72b0', '#dd8452', '#dd8452'])
plt.ylabel('Average time (ms)')
plt.title('Performance comparison: Mutex vs InterlockedIncrement')
plt.xticks(rotation=15)
plt.grid(axis='y', linestyle='--', alpha=0.7)
plt.tight_layout()
plt.savefig('../report/graph1_modes.png', dpi=300)
plt.show()

# Graph 2: Scaling (Line chart)
plt.figure(figsize=(8, 5))
mutex_data = df[df['Mode'] == 'Mutex']
inter_data = df[df['Mode'] == 'Interlocked']

plt.plot(mutex_data['Workers'], mutex_data['Avg'], marker='o', label='Mutex', color='#4c72b0', linewidth=2)
plt.plot(inter_data['Workers'], inter_data['Avg'], marker='s', label='Interlocked', color='#dd8452', linewidth=2)

plt.xlabel('Number of worker processes')
plt.ylabel('Average time (ms)')
plt.title('Scaling: impact of process count on execution time')
plt.xticks([2, 3])
plt.legend()
plt.grid(True, linestyle='--', alpha=0.7)
plt.tight_layout()
plt.savefig('../report/graph2_scaling.png', dpi=300)
plt.show()