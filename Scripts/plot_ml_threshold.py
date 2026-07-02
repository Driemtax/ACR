import json

import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

# Deine JSON-Daten
json_data = """[
  {
    "similarity_threshold": 0.100000001490116,
    "accuracy": 0.561874985694885
  },
  {
    "similarity_threshold": 0.150000005960464,
    "accuracy": 0.561874985694885
  },
  {
    "similarity_threshold": 0.200000002980232,
    "accuracy": 0.561874985694885
  },
  {
    "similarity_threshold": 0.25,
    "accuracy": 0.561874985694885
  },
  {
    "similarity_threshold": 0.300000011920929,
    "accuracy": 0.561874985694885
  },
  {
    "similarity_threshold": 0.350000023841858,
    "accuracy": 0.561874985694885
  },
  {
    "similarity_threshold": 0.400000035762787,
    "accuracy": 0.561874985694885
  },
  {
    "similarity_threshold": 0.450000047683716,
    "accuracy": 0.561874985694885
  },
  {
    "similarity_threshold": 0.500000059604645,
    "accuracy": 0.561874985694885
  },
  {
    "similarity_threshold": 0.550000071525574,
    "accuracy": 0.561874985694885
  },
  {
    "similarity_threshold": 0.600000083446503,
    "accuracy": 0.561874985694885
  },
  {
    "similarity_threshold": 0.650000095367432,
    "accuracy": 0.56229168176651
  },
  {
    "similarity_threshold": 0.700000107288361,
    "accuracy": 0.5625
  },
  {
    "similarity_threshold": 0.75000011920929,
    "accuracy": 0.56229168176651
  },
  {
    "similarity_threshold": 0.800000131130219,
    "accuracy": 0.562083303928375
  },
  {
    "similarity_threshold": 0.850000143051147,
    "accuracy": 0.558749973773956
  },
  {
    "similarity_threshold": 0.900000154972076,
    "accuracy": 0.551249980926514
  }
]"""

# 1. Daten laden und in einen Pandas DataFrame konvertieren
data = json.loads(json_data)
df = pd.DataFrame(data)

# 2. Daten bereinigen
# Threshold auf 2 Nachkommastellen runden für eine saubere X-Achse
df["similarity_threshold"] = df["similarity_threshold"].round(2)
# Accuracy in Prozent umwandeln (x 100)
df["accuracy"] = df["accuracy"] * 100

# 3. Maximum ermitteln
max_idx = df["accuracy"].idxmax()
max_threshold = df.loc[max_idx, "similarity_threshold"]
max_acc = df.loc[max_idx, "accuracy"]

# 4. Styling und Plot
sns.set_theme(style="whitegrid")
plt.figure(figsize=(10, 6))

# Liniengraph zeichnen
sns.lineplot(
    data=df,
    x="similarity_threshold",
    y="accuracy",
    color="#2ba02b",  # Ein schöner Grünton zur Abwechslung
    linewidth=2.5,
    marker="o",
    markersize=6,
    markerfacecolor="#d62728",
)

# 5. Optisches Highlight für das Maximum
plt.plot(max_threshold, max_acc, marker="o", markersize=10, color="#d62728", zorder=5)

# Annotation Text platzieren
plt.annotate(
    f"Max: {max_acc:.3f}%\n(Threshold {max_threshold})",
    xy=(max_threshold, max_acc),
    xytext=(max_threshold - 0.15, max_acc - 0.2),  # Verschiebung des Textes
    arrowprops=dict(facecolor="black", shrink=0.05, width=1.5, headwidth=8),
    fontsize=11,
    bbox=dict(boxstyle="round,pad=0.3", fc="white", ec="gray", alpha=0.9),
)

# 6. Achsenbeschriftung und Titel
plt.title("Einfluss des Similarity Thresholds auf die Accuracy", fontsize=15, pad=15)
plt.xlabel("Similarity Threshold", fontsize=12)
plt.ylabel("Accuracy (%)", fontsize=12)

# Y-Achsen-Limits leicht anpassen, damit der Graph nicht an den Rand stößt
plt.ylim(df["accuracy"].min() - 0.2, df["accuracy"].max() + 0.2)

# Layout optimieren und anzeigen
plt.tight_layout()

output_file = "ml_threshold.png"

plt.savefig(output_file, dpi=300)
print(f"Erfolgreich! Graph gespeichert als '{output_file}'.")

plt.show()
