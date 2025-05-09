import pandas as pd
import matplotlib.pyplot as plt
def plot_csv_files():
    # Read the CSV files into DataFrames
    df1 = pd.read_csv('data/store_time.csv')
    # read only the first 300 rows
    df1 = df1.head(300)
    df2 = pd.read_csv('data/store_time_non-sca.csv')
    # df3 = pd.read_csv('data/compute_at_y.csv')
    # df4 = pd.read_csv('data/store_compute.csv')

    plt.plot(df1['x'], df1['y'], linestyle='-', color='b', label='SCA')
    plt.plot(df2['x'], df2['y'], linestyle='-', color='r', label='Pipeline')
    # plt.plot(df3['x'], df3['y'], linestyle='-', color='g', label='Compute at Y')
    # plt.plot(df4['x'], df4['y'], linestyle='-', color='orange', label='Store and Compute')
    # Add titles and labels
    plt.title('Bandwidth/Compute Ratio vs. Domain Size (Higher is Bandwidth Limited)')
    plt.xlabel('Domain Width (Square Realization)')
    plt.ylabel('Time (ms)')

    # Add a legend
    plt.legend()

    # Show the plot
    plt.grid(True)
    plt.show()

plot_csv_files()
