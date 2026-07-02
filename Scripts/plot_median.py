import re

import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

# Deine gesammelten Daten aus beiden Sweeps
log_data = """
Median Size: 1, Accuracy: 50.5397
Median Size: 3, Accuracy: 51.4221
Median Size: 5, Accuracy: 52.0833
Median Size: 7, Accuracy: 52.6789
Median Size: 9, Accuracy: 53.1311
Median Size: 11, Accuracy: 53.4422
Median Size: 13, Accuracy: 53.5662
Median Size: 15, Accuracy: 53.8604
Median Size: 17, Accuracy: 54.1326
Median Size: 19, Accuracy: 54.5289
Median Size: 21, Accuracy: 54.7331
Median Size: 23, Accuracy: 54.8814
Median Size: 25, Accuracy: 55.088
Median Size: 27, Accuracy: 55.2728
Median Size: 29, Accuracy: 55.37
Median Size: 31, Accuracy: 55.477
Median Size: 33, Accuracy: 55.669
Median Size: 35, Accuracy: 55.861
Median Size: 37, Accuracy: 56.058
Median Size: 39, Accuracy: 56.2403
Median Size: 41, Accuracy: 56.4323
Median Size: 43, Accuracy: 56.5733
Median Size: 45, Accuracy: 56.6754
Median Size: 47, Accuracy: 56.7897
Median Size: 49, Accuracy: 56.8504
Median Size: 51, Accuracy: 57.006
Median Size: 53, Accuracy: 57.2856
Median Size: 55, Accuracy: 57.4971
Median Size: 57, Accuracy: 57.6259
Median Size: 59, Accuracy: 57.6916
Median Size: 61, Accuracy: 57.7694
Median Size: 63, Accuracy: 57.7499
Median Size: 65, Accuracy: 57.852
Median Size: 67, Accuracy: 57.959
Median Size: 69, Accuracy: 57.9687
Median Size: 71, Accuracy: 57.8763
Median Size: 73, Accuracy: 57.9541
Median Size: 75, Accuracy: 57.976
Median Size: 77, Accuracy: 58.0951
Median Size: 79, Accuracy: 58.275
Median Size: 81, Accuracy: 58.3576
Median Size: 83, Accuracy: 58.4695
Median Size: 85, Accuracy: 58.4208
Median Size: 87, Accuracy: 58.4427
Median Size: 89, Accuracy: 58.4233
Median Size: 91, Accuracy: 58.3698
Median Size: 93, Accuracy: 58.3528
Median Size: 95, Accuracy: 58.3844
Median Size: 97, Accuracy: 58.3771
Median Size: 99, Accuracy: 58.4208
Median Size: 101, Accuracy: 58.4063
Median Size: 103, Accuracy: 58.3868
Median Size: 105, Accuracy: 58.4889
Median Size: 107, Accuracy: 58.4379
Median Size: 109, Accuracy: 58.5108
Median Size: 111, Accuracy: 58.574
Median Size: 113, Accuracy: 58.6323
Median Size: 115, Accuracy: 58.7004
Median Size: 117, Accuracy: 58.6664
Median Size: 119, Accuracy: 58.7174
Median Size: 121, Accuracy: 58.8779
Median Size: 123, Accuracy: 58.9435
Median Size: 125, Accuracy: 58.9459
Median Size: 127, Accuracy: 58.9921
Median Size: 129, Accuracy: 59.2036
Median Size: 131, Accuracy: 59.2133
Median Size: 133, Accuracy: 59.1404
Median Size: 135, Accuracy: 59.0505
Median Size: 137, Accuracy: 59.0432
Median Size: 139, Accuracy: 59.138
Median Size: 141, Accuracy: 59.0578
Median Size: 143, Accuracy: 58.9459
Median Size: 145, Accuracy: 58.963
Median Size: 147, Accuracy: 58.9459
Median Size: 149, Accuracy: 58.8049
Median Size: 151, Accuracy: 58.6834
Median Size: 153, Accuracy: 58.7539
Median Size: 155, Accuracy: 58.7174
Median Size: 157, Accuracy: 58.7393
Median Size: 159, Accuracy: 58.7976
Median Size: 161, Accuracy: 58.7685
Median Size: 163, Accuracy: 58.7709
Median Size: 165, Accuracy: 58.7831
Median Size: 167, Accuracy: 58.7369
Median Size: 169, Accuracy: 58.6639
Median Size: 171, Accuracy: 58.6105
Median Size: 173, Accuracy: 58.5618
Median Size: 175, Accuracy: 58.6494
Median Size: 177, Accuracy: 58.6761
Median Size: 179, Accuracy: 58.7028
Median Size: 181, Accuracy: 58.6785
Median Size: 183, Accuracy: 58.7563
Median Size: 185, Accuracy: 58.9897
Median Size: 187, Accuracy: 58.9654
Median Size: 189, Accuracy: 59.1258
Median Size: 191, Accuracy: 59.155
Median Size: 193, Accuracy: 59.2474
Median Size: 195, Accuracy: 59.2352
Median Size: 197, Accuracy: 59.279
Median Size: 199, Accuracy: 59.2765
Median Size: 201, Accuracy: 59.3616
Median Size: 203, Accuracy: 59.2863
Median Size: 205, Accuracy: 59.2595
Median Size: 207, Accuracy: 59.2061
Median Size: 209, Accuracy: 59.2522
Median Size: 211, Accuracy: 59.189
Median Size: 213, Accuracy: 59.2547
Median Size: 215, Accuracy: 59.2547
Median Size: 217, Accuracy: 59.2668
Median Size: 219, Accuracy: 59.3203
Median Size: 221, Accuracy: 59.3641
Median Size: 223, Accuracy: 59.3714
Median Size: 225, Accuracy: 59.3081
Median Size: 227, Accuracy: 59.2838
Median Size: 229, Accuracy: 59.3033
Median Size: 231, Accuracy: 59.3227
"""

# Daten parsen
parsed_data = []
pattern = r"Median Size: (\d+), Accuracy: ([0-9.]+)"

for line in log_data.strip().split("\n"):
    match = re.search(pattern, line)
    if match:
        size = int(match.group(1))
        acc = float(match.group(2))
        parsed_data.append({"Median Window Size": size, "Accuracy": acc})

df = pd.DataFrame(parsed_data)

# Styling
sns.set_theme(style="whitegrid")
plt.figure(figsize=(12, 6))

# Liniengraph erstellen
ax = sns.lineplot(
    data=df,
    x="Median Window Size",
    y="Accuracy",
    color="#2c7bb6",
    linewidth=2,
    marker="o",
    markersize=4,
    markerfacecolor="#d7191c",
)

# Maximum finden und markieren
max_idx = df["Accuracy"].idxmax()
max_size = df.loc[max_idx, "Median Window Size"]
max_acc = df.loc[max_idx, "Accuracy"]

# Optisches Highlight für das Maximum
plt.plot(max_size, max_acc, marker="o", markersize=10, color="#d7191c", zorder=5)
plt.annotate(
    f"Max: {max_acc:.2f}% (Size {max_size})",
    xy=(max_size, max_acc),
    xytext=(max_size - 40, max_acc - 1.5),
    arrowprops=dict(facecolor="black", shrink=0.05, width=1.5, headwidth=8),
    fontsize=12,
    bbox=dict(boxstyle="round,pad=0.3", fc="white", ec="gray", alpha=0.8),
)

# Achsenbeschriftung und Titel
plt.title("Einfluss der Median Window Size auf die Genauigkeit", fontsize=16, pad=15)
plt.xlabel("Median Window Size (Frames)", fontsize=13)
plt.ylabel("Genauigkeit (%)", fontsize=13)
plt.tight_layout()

output_file = "median_chart.png"
plt.savefig(output_file, dpi=300)
print(f"Erfolgreich! Graph gespeichert als '{output_file}'.")

# Raster und Layout anpassen
plt.show()
