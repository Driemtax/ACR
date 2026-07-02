import re

import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

# Deine Testergebnisse (gekürzt auf den wesentlichen Block für das Beispiel)
# Du kannst hier deinen kompletten Output reinkopieren.
log_data = """
Test s=0.1, treshold=0.3 -> Accuracy: 45.8625%
Test s=0.1, treshold=0.4 -> Accuracy: 45.8625%
Test s=0.1, treshold=0.5 -> Accuracy: 45.8625%
Test s=0.1, treshold=0.6 -> Accuracy: 45.3739%
Test s=0.1, treshold=0.7 -> Accuracy: 36.462%
Test s=0.1, treshold=0.8 -> Accuracy: 22.1922%
Test s=0.1, treshold=0.9 -> Accuracy: 8.46461%
Test s=0.2, treshold=0.3 -> Accuracy: 46.5456%
Test s=0.2, treshold=0.4 -> Accuracy: 46.5456%
Test s=0.2, treshold=0.5 -> Accuracy: 46.5456%
Test s=0.2, treshold=0.6 -> Accuracy: 46.0546%
Test s=0.2, treshold=0.7 -> Accuracy: 36.6783%
Test s=0.2, treshold=0.8 -> Accuracy: 22.377%
Test s=0.2, treshold=0.9 -> Accuracy: 8.51322%
Test s=0.3, treshold=0.3 -> Accuracy: 47.4135%
Test s=0.3, treshold=0.4 -> Accuracy: 47.4135%
Test s=0.3, treshold=0.5 -> Accuracy: 47.4135%
Test s=0.3, treshold=0.6 -> Accuracy: 46.8714%
Test s=0.3, treshold=0.7 -> Accuracy: 36.8436%
Test s=0.3, treshold=0.8 -> Accuracy: 22.5496%
Test s=0.3, treshold=0.9 -> Accuracy: 8.55941%
Test s=0.4, treshold=0.3 -> Accuracy: 48.4199%
Test s=0.4, treshold=0.4 -> Accuracy: 48.4199%
Test s=0.4, treshold=0.5 -> Accuracy: 48.4199%
Test s=0.4, treshold=0.6 -> Accuracy: 47.8048%
Test s=0.4, treshold=0.7 -> Accuracy: 36.7172%
Test s=0.4, treshold=0.8 -> Accuracy: 22.5326%
Test s=0.4, treshold=0.9 -> Accuracy: 8.42328%
Test s=0.5, treshold=0.3 -> Accuracy: 49.4482%
Test s=0.5, treshold=0.4 -> Accuracy: 49.4482%
Test s=0.5, treshold=0.5 -> Accuracy: 49.4482%
Test s=0.5, treshold=0.6 -> Accuracy: 48.6727%
Test s=0.5, treshold=0.7 -> Accuracy: 36.355%
Test s=0.5, treshold=0.8 -> Accuracy: 22.3114%
Test s=0.5, treshold=0.9 -> Accuracy: 7.90305%
Test s=0.6, treshold=0.3 -> Accuracy: 50.5397%
Test s=0.6, treshold=0.4 -> Accuracy: 50.5397%
Test s=0.6, treshold=0.5 -> Accuracy: 50.5397%
Test s=0.6, treshold=0.6 -> Accuracy: 49.4798%
Test s=0.6, treshold=0.7 -> Accuracy: 35.6841%
Test s=0.6, treshold=0.8 -> Accuracy: 21.9224%
Test s=0.6, treshold=0.9 -> Accuracy: 6.94282%
Test s=0.7, treshold=0.3 -> Accuracy: 51.3881%
Test s=0.7, treshold=0.4 -> Accuracy: 51.3881%
Test s=0.7, treshold=0.5 -> Accuracy: 51.3881%
Test s=0.7, treshold=0.6 -> Accuracy: 49.9028%
Test s=0.7, treshold=0.7 -> Accuracy: 34.2595%
Test s=0.7, treshold=0.8 -> Accuracy: 21.2806%
Test s=0.7, treshold=0.9 -> Accuracy: 5.24115%
Test s=0.8, treshold=0.3 -> Accuracy: 51.9666%
Test s=0.8, treshold=0.4 -> Accuracy: 51.9666%
Test s=0.8, treshold=0.5 -> Accuracy: 51.9666%
Test s=0.8, treshold=0.6 -> Accuracy: 49.8906%
Test s=0.8, treshold=0.7 -> Accuracy: 32.1884%
Test s=0.8, treshold=0.8 -> Accuracy: 20.2694%
Test s=0.8, treshold=0.9 -> Accuracy: 3.22832%
Test s=0.9, treshold=0.3 -> Accuracy: 51.991%
Test s=0.9, treshold=0.4 -> Accuracy: 51.991%
Test s=0.9, treshold=0.5 -> Accuracy: 51.991%
Test s=0.9, treshold=0.6 -> Accuracy: 49.1516%
Test s=0.9, treshold=0.7 -> Accuracy: 29.7258%
Test s=0.9, treshold=0.8 -> Accuracy: 18.7524%
Test s=0.9, treshold=0.9 -> Accuracy: 1.68709%
Test s=1, treshold=0.3 -> Accuracy: 51.6652%
Test s=1, treshold=0.4 -> Accuracy: 51.6652%
Test s=1, treshold=0.5 -> Accuracy: 51.6652%
Test s=1, treshold=0.6 -> Accuracy: 48.0528%
Test s=1, treshold=0.7 -> Accuracy: 27.9026%
Test s=1, treshold=0.8 -> Accuracy: 17.0508%
Test s=1, treshold=0.9 -> Accuracy: 0.867853%
"""

# 1. Daten parsen
parsed_data = []
# Regex sucht nach den Mustern im Text
pattern = r"Test s=([0-9.]+), treshold=([0-9.]+) -> Accuracy: ([0-9.]+)%"

for line in log_data.strip().split("\n"):
    match = re.search(pattern, line)
    if match:
        s_val = float(match.group(1))
        t_val = float(match.group(2))
        acc = float(match.group(3))
        parsed_data.append({"s": s_val, "Threshold": t_val, "Accuracy": acc})

df = pd.DataFrame(parsed_data)

# 2. Daten für die Heatmap in eine Matrix umwandeln (Pivot)
# Falls Werte doppelt vorkommen (z.B. aus grober und feiner Suche), nimmt aggfunc='mean' den Durchschnitt
pivot_df = df.pivot_table(
    index="Threshold", columns="s", values="Accuracy", aggfunc="mean"
)

# 3. Plot erstellen
plt.figure(figsize=(12, 8))
sns.heatmap(
    pivot_df,
    annot=True,  # Zahlen in den Kacheln anzeigen
    fmt=".1f",  # Eine Nachkommastelle (z.B. 52.3)
    cmap="viridis",  # Farbschema (Dunkelblau = schlecht, Gelb = gut)
    cbar_kws={"label": "Accuracy (%)"},
)

plt.title(
    "Grid Search: Genauigkeit der ACR in Abhängigkeit von s und similarityThreshold",
    fontsize=14,
    pad=15,
)
plt.xlabel("Parameter s", fontsize=12)
plt.ylabel("Similarity Threshold", fontsize=12)

# Y-Achse umdrehen, damit die höchsten Thresholds oben sind (wie in einem Koordinatensystem)
plt.gca().invert_yaxis()

plt.tight_layout()

output_file = "hpcp_floatParameters_sweep.png"
plt.savefig(output_file, dpi=300)

plt.show()
