import re

import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

# Deine gesammelten Daten aus beiden Sweeps
log_data = """
Median Size: 1, Accuracy: 61.7083
Median Size: 3, Accuracy: 61.6458
Median Size: 5, Accuracy: 61.75
Median Size: 7, Accuracy: 61.875
Median Size: 9, Accuracy: 62.1667
Median Size: 11, Accuracy: 62.1667
Median Size: 13, Accuracy: 62.3125
Median Size: 15, Accuracy: 62.8542
Median Size: 17, Accuracy: 63.0417
Median Size: 19, Accuracy: 62.5625
Median Size: 21, Accuracy: 63.2708
Median Size: 23, Accuracy: 64.625
Median Size: 25, Accuracy: 65.3958
Median Size: 27, Accuracy: 66
Median Size: 29, Accuracy: 66.1458
Median Size: 31, Accuracy: 65.9167
Median Size: 33, Accuracy: 65.4792
Median Size: 35, Accuracy: 65.6875
Median Size: 37, Accuracy: 65.7083
Median Size: 39, Accuracy: 65.4375
Median Size: 41, Accuracy: 65.5833
Median Size: 43, Accuracy: 66.4167
Median Size: 45, Accuracy: 65.875
Median Size: 47, Accuracy: 68.2292
Median Size: 49, Accuracy: 67.7083
Median Size: 51, Accuracy: 67.7083
Median Size: 53, Accuracy: 66.8958
Median Size: 55, Accuracy: 67.2083
Median Size: 57, Accuracy: 65.7917
Median Size: 59, Accuracy: 63.9167
Median Size: 61, Accuracy: 62.5625
Median Size: 63, Accuracy: 62.6042
Median Size: 65, Accuracy: 63.3125
Median Size: 67, Accuracy: 62.4375
Median Size: 69, Accuracy: 62.7292
Median Size: 71, Accuracy: 60.6458
Median Size: 73, Accuracy: 55.8125
Median Size: 75, Accuracy: 47.4792
Median Size: 77, Accuracy: 44.1458
Median Size: 79, Accuracy: 42.5625
Median Size: 81, Accuracy: 42.3125
Median Size: 83, Accuracy: 41.2708
Median Size: 85, Accuracy: 41.25
Median Size: 87, Accuracy: 41.0417
Median Size: 89, Accuracy: 40.75
Median Size: 91, Accuracy: 39.7292
Median Size: 93, Accuracy: 38.75
Median Size: 95, Accuracy: 38.75
Median Size: 97, Accuracy: 38.2708
Median Size: 99, Accuracy: 38.3125
Median Size: 101, Accuracy: 35.8333
Median Size: 103, Accuracy: 36.2917
Median Size: 105, Accuracy: 35.9792
Median Size: 107, Accuracy: 36.0625
Median Size: 109, Accuracy: 36.0625
Median Size: 111, Accuracy: 36.0833
Median Size: 113, Accuracy: 36.1042
Median Size: 115, Accuracy: 37.8542
Median Size: 117, Accuracy: 37.8542
Median Size: 119, Accuracy: 38.1042
Median Size: 121, Accuracy: 37.9583
Median Size: 123, Accuracy: 38.1875
Median Size: 125, Accuracy: 38.0625
Median Size: 127, Accuracy: 37.9375
Median Size: 129, Accuracy: 37.9792
Median Size: 131, Accuracy: 37.9792
Median Size: 133, Accuracy: 37.9583
Median Size: 135, Accuracy: 37.8958
Median Size: 137, Accuracy: 37.8333
Median Size: 139, Accuracy: 37.8333
Median Size: 141, Accuracy: 37.9583
Median Size: 143, Accuracy: 37.9583
Median Size: 145, Accuracy: 37.9167
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

output_file = "median_chart_ml.png"
plt.savefig(output_file, dpi=300)
print(f"Erfolgreich! Graph gespeichert als '{output_file}'.")

# Raster und Layout anpassen
plt.show()
