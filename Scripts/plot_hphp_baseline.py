import json
import os

import matplotlib.pyplot as plt

# 1. Dateipfade definieren (Dateinamen ggf. exakt anpassen)
json_files = [
    "C:\\dev\\ACR\\TestResults\\ThesisTests\\results_ML_Base_noGain_noPowerchords.json",
    "C:\\dev\\ACR\\TestResults\\ThesisTests\\results_ML_Base_noGain.json",
    "C:\\dev\\ACR\\TestResults\\ThesisTests\\results_ML_Base.json",
    "C:\\dev\\ACR\\TestResults\\results_DeepChroma_WithKeySmoothing.json",
]

test_names = []
accuracies = []

# 2. JSON-Dateien einlesen
for file_path in json_files:
    if os.path.exists(file_path):
        with open(file_path, "r", encoding="utf-8") as f:
            data = json.load(f)
            # Extrahiere test_name und overall_accuracy
            test_names.append(data.get("test_name", "Unknown"))
            # Umrechnung in Prozent
            accuracies.append(data.get("overall_accuracy", 0.0) * 100)
    else:
        print(f"Warnung: Die Datei '{file_path}' existiert nicht.")
        test_names.append(file_path.replace(".json", ""))
        accuracies.append(0.0)

# 3. Plot konfigurieren (Thesis-optimiert)
fig, ax = plt.subplots(figsize=(8, 6))

# Balkendiagramm zeichnen (zorder=3 legt Balken ÜBER die Gridlinien)
bars = ax.bar(
    ["noGain_noPowerchords", "noGain", "base", "newBase"],
    accuracies,
    color="#4C72B0",
    width=0.5,
    zorder=3,
)

# Titel und Achsenbeschriftungen
ax.set_title("Deep Learning Baseline Vergleich (optimiert)", fontsize=14, pad=15)
ax.set_ylabel("Genauigkeit (%)", fontsize=12)

# Y-Achse sinnvoll skalieren (Damit oben etwas Platz für die Labels bleibt)
max_val = max(accuracies) if accuracies else 100
ax.set_ylim(0, max_val + 10)

# Schwache horizontale Linien für gute Ablesbarkeit
ax.grid(axis="y", linestyle="--", alpha=0.7, zorder=0)

# 4. Exakte Prozentwerte (auf 2 Nachkommastellen) über die Balken schreiben
ax.bar_label(
    bars, fmt="%.2f%%", padding=5, fontsize=11, color="#333333", fontweight="bold"
)

# Damit Labels an der X-Achse nicht abgeschnitten werden
plt.tight_layout()

# 5. Bild als hochauflösendes PNG exportieren
output_file = "ml_new_baseline_barchart.png"
plt.savefig(output_file, dpi=300)
print(f"Erfolgreich! Graph gespeichert als '{output_file}'.")

# Graph zur Vorschau anzeigen
plt.show()
