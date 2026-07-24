import json

import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

# Deine JSON-Daten
json_data = """[
  {
    "sptRatio": 0.0,
    "accuracy": 0.4383265376091
  },
  {
    "sptRatio": 0.100000001490116,
    "accuracy": 0.4383265376091
  },
  {
    "sptRatio": 0.200000002980232,
    "accuracy": 0.438666850328445
  },
  {
    "sptRatio": 0.300000011920929,
    "accuracy": 0.444355309009552
  },
  {
    "sptRatio": 0.400000005960464,
    "accuracy": 0.445789575576782
  },
  {
    "sptRatio": 0.5,
    "accuracy": 0.446956425905228
  },
  {
    "sptRatio": 0.600000023841858,
    "accuracy": 0.484320312738419
  },
  {
    "sptRatio": 0.700000047683716,
    "accuracy": 0.493557959794998
  },
  {
    "sptRatio": 0.800000071525574,
    "accuracy": 0.485851794481277
  },
  {
    "sptRatio": 0.900000095367432,
    "accuracy": 0.485414236783981
  },
  {
    "sptRatio": 1.00000011920929,
    "accuracy": 0.48891481757164
  },
  {
    "sptRatio": 1.100000143051147,
    "accuracy": 0.488890498876572
  },
  {
    "sptRatio": 1.200000166893005,
    "accuracy": 0.498954683542252
  },
  {
    "sptRatio": 1.300000190734863,
    "accuracy": 0.575262546539307
  },
  {
    "sptRatio": 1.400000214576721,
    "accuracy": 0.575189590454102
  },
  {
    "sptRatio": 1.500000238418579,
    "accuracy": 0.572710037231445
  },
  {
    "sptRatio": 1.600000262260437,
    "accuracy": 0.572418332099915
  },
  {
    "sptRatio": 1.700000286102295,
    "accuracy": 0.574241518974304
  },
  {
    "sptRatio": 1.800000309944153,
    "accuracy": 0.574265837669373
  },
  {
    "sptRatio": 1.900000333786011,
    "accuracy": 0.574484646320343
  },
  {
    "sptRatio": 2.000000238418579,
    "accuracy": 0.574338793754578
  },
  {
    "sptRatio": 2.100000143051147,
    "accuracy": 0.574314475059509
  },
  {
    "sptRatio": 2.200000047683716,
    "accuracy": 0.574314475059509
  },
  {
    "sptRatio": 2.299999952316284,
    "accuracy": 0.574581861495972
  },
  {
    "sptRatio": 2.399999856948853,
    "accuracy": 0.575602889060974
  },
  {
    "sptRatio": 2.499999761581421,
    "accuracy": 0.57256418466568
  },
  {
    "sptRatio": 2.599999666213989,
    "accuracy": 0.594540059566498
  },
  {
    "sptRatio": 2.699999570846558,
    "accuracy": 0.601444005966187
  },
  {
    "sptRatio": 2.799999475479126,
    "accuracy": 0.601444005966187
  },
  {
    "sptRatio": 2.899999380111694,
    "accuracy": 0.601444005966187
  },
  {
    "sptRatio": 2.999999284744263,
    "accuracy": 0.601444005966187
  },
  {
    "sptRatio": 3.099999189376831,
    "accuracy": 0.601444005966187
  },
  {
    "sptRatio": 3.199999094009399,
    "accuracy": 0.601444005966187
  },
  {
    "sptRatio": 3.299998998641968,
    "accuracy": 0.601468324661255
  },
  {
    "sptRatio": 3.399998903274536,
    "accuracy": 0.60139536857605
  },
  {
    "sptRatio": 3.499998807907104,
    "accuracy": 0.601225197315216
  },
  {
    "sptRatio": 3.599998712539673,
    "accuracy": 0.601103663444519
  },
  {
    "sptRatio": 3.699998617172241,
    "accuracy": 0.601079344749451
  },
  {
    "sptRatio": 3.79999852180481,
    "accuracy": 0.601127982139587
  },
  {
    "sptRatio": 3.899998426437378,
    "accuracy": 0.601127982139587
  },
  {
    "sptRatio": 3.999998331069946,
    "accuracy": 0.601103663444519
  },
  {
    "sptRatio": 4.099998474121094,
    "accuracy": 0.601103663444519
  },
  {
    "sptRatio": 4.199998378753662,
    "accuracy": 0.601103663444519
  },
  {
    "sptRatio": 4.29999828338623,
    "accuracy": 0.601103663444519
  },
  {
    "sptRatio": 4.399998188018799,
    "accuracy": 0.601103663444519
  },
  {
    "sptRatio": 4.499998092651367,
    "accuracy": 0.601103663444519
  },
  {
    "sptRatio": 4.599997997283936,
    "accuracy": 0.601103663444519
  },
  {
    "sptRatio": 4.699997901916504,
    "accuracy": 0.601200878620148
  },
  {
    "sptRatio": 4.799997806549072,
    "accuracy": 0.600982129573822
  },
  {
    "sptRatio": 4.899997711181641,
    "accuracy": 0.600811958312988
  },
  {
    "sptRatio": 4.999997615814209,
    "accuracy": 0.600714683532715
  },
  {
    "sptRatio": 3.199999094009399,
    "accuracy": 0.601444005966187
  },
  {
    "sptRatio": 3.219999074935913,
    "accuracy": 0.601444005966187
  },
  {
    "sptRatio": 3.239999055862427,
    "accuracy": 0.601468324661255
  },
  {
    "sptRatio": 3.25999903678894,
    "accuracy": 0.601468324661255
  },
  {
    "sptRatio": 3.279999017715454,
    "accuracy": 0.601468324661255
  },
  {
    "sptRatio": 3.299998998641968,
    "accuracy": 0.601468324661255
  },
  {
    "sptRatio": 3.319998979568481,
    "accuracy": 0.601468324661255
  },
  {
    "sptRatio": 3.339998960494995,
    "accuracy": 0.60139536857605
  },
  {
    "sptRatio": 3.359998941421509,
    "accuracy": 0.60139536857605
  },
  {
    "sptRatio": 3.379998922348022,
    "accuracy": 0.60139536857605
  }
]"""

# 1. Daten laden und in einen Pandas DataFrame konvertieren
data = json.loads(json_data)
df = pd.DataFrame(data)

# 2. Daten bereinigen
# Threshold auf 2 Nachkommastellen runden für eine saubere X-Achse
df["sptRatio"] = df["sptRatio"].round(2)
# Accuracy in Prozent umwandeln (x 100)
df["accuracy"] = df["accuracy"] * 100

# 3. Maximum ermitteln
max_idx = df["accuracy"].idxmax()
max_threshold = df.loc[max_idx, "sptRatio"]
max_acc = df.loc[max_idx, "accuracy"]

# 4. Styling und Plot
sns.set_theme(style="whitegrid")
plt.figure(figsize=(10, 6))

# Liniengraph zeichnen
sns.lineplot(
    data=df,
    x="sptRatio",
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
    f"Max: {max_acc:.3f}%\n(Ratio {max_threshold})",
    xy=(max_threshold, max_acc),
    xytext=(max_threshold - 0.15, max_acc - 0.2),  # Verschiebung des Textes
    arrowprops=dict(facecolor="black", shrink=0.05, width=1.5, headwidth=8),
    fontsize=11,
    bbox=dict(boxstyle="round,pad=0.3", fc="white", ec="gray", alpha=0.9),
)

# 6. Achsenbeschriftung und Titel
plt.title("Einfluss der sptRatio auf die Accuracy", fontsize=15, pad=15)
plt.xlabel("Spectral Pitch Tracking Ratio", fontsize=12)
plt.ylabel("Accuracy (%)", fontsize=12)

# Y-Achsen-Limits leicht anpassen, damit der Graph nicht an den Rand stößt
plt.ylim(df["accuracy"].min() - 0.2, df["accuracy"].max() + 0.2)

# Layout optimieren und anzeigen
plt.tight_layout()

output_file = "hpcp_spt_ratio.png"

plt.savefig(output_file, dpi=300)
print(f"Erfolgreich! Graph gespeichert als '{output_file}'.")

plt.show()
