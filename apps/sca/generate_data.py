import os
import subprocess
import csv
import matplotlib.pyplot as plt
from tqdm import tqdm
# run generate_metrics
def single_run(width, height):
    proc = subprocess.run(["./a.out", str(width), str(height)], stdout=subprocess.PIPE)
    # collect data from stdout

    output = proc.stdout.decode("utf-8")
    lst = [float(num) for num in output.split()]
    
    compute = lst[4] + lst[5] + lst[6]

    bandwidth = lst[2] + lst[3]
    time = lst[7]
    if bandwidth == 0:
        ratio = 0
    elif compute == 0:
        # error
        print("compute is 0")
        return
    else:
        ratio = bandwidth / compute
    return [ratio, width, time]
def generate_graph_data():
    data = []
    times = []
    for i in tqdm(range(0, 300)):
        width = i
        sum = 0
        time = 0
        trials = 5
        for j in range(0, trials):
            sum += single_run(i, i)[0]
            time += single_run(i, i)[2]
        data.append([sum / trials, i])
        times.append([time / trials, i])
    return [data, times]

def plot_line_graph(pairs, pairs2):
    x = [pair[1] for pair in pairs]
    y = [pair[0] for pair in pairs]
    plt.figure(figsize=(10, 5))
    plt.plot(x, y, label='blue')
    x2 = [pair[1] for pair in pairs2]
    y2 = [pair[0] for pair in pairs2]
    # plt.plot(x2, y2,label='red')
    plt.title('Bandwidth/Compute Ratio vs Domain Size (Higher is Bandwidth Limited)')
    plt.xlabel('Domain Size')
    plt.ylabel('Bandwidth/Compute Ratio')
    plt.show()
    save_to_csv('data/store_compute_non_sca.csv', pairs)
def plot_line_graph_time(pairs):
    x = [pair[1] for pair in pairs]
    y = [pair[0] for pair in pairs]
    plt.figure(figsize=(10, 5))
    plt.plot(x, y, color='blue')
    plt.title('Time vs Domain Size')
    plt.xlabel('Domain Size')
    plt.ylabel('Time')
    plt.show()
    # save to csv
    save_to_csv('data/store_time_non-sca.csv', pairs)

def save_to_csv(file_name, data):
    # Open the file in write mode (this will overwrite existing data)
    with open(file_name, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['y', 'x'])  # Write header
        writer.writerows(data)  # Write data rows


# save_to_csv('data/store_compute.csv', generate_graph_data())
# single_run(4, 4)
# print(generate_graph_data())
print("done")
pairs = generate_graph_data()
plot_line_graph(pairs[0], pairs[1])
plot_line_graph_time(pairs[1])
