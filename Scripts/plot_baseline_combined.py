import json
import os

import matplotlib.pyplot as plt
import numpy as np

# 1. Dateipfade definieren (Dateinamen exakt anpassen!)
json_files_hpcp = [
    "C:\\dev\\ACR\\TestResults\\ThesisTests\\results_HPCP_Base_noGain_noPowerchords.json",
    "C:\\dev\\ACR\\TestResults\\ThesisTests\\results_HPCP_Base_noGain.json",
    "C:\\dev\\ACR\\TestResults\\ThesisTests\\results_HPCP_newBase.json",
]

json_files_ml = [
    "C:\\dev\\ACR\\TestResults\\ThesisTests\\results_ML_Base_noGain_noPowerchords.json",
    "C:\\dev\\ACR\\TestResults\\ThesisTests\\results_ML_Base_noGain.json",
    "C:\\dev\\ACR\\TestResults\\results_DeepChroma_WithKeySmoothing.json",
]

# Bezeichnungen für die X-Achse (entsprechend der 3 Testfälle)
test_labels = [
    "Keine Verzerrung,\nkeine Powerchords",
    "Keine Verzerrung",
    "Optimierte Baseline",
]


# 2. Hilfsfunktion zum Einlesen der JSON-Dateien
def load_accuracies(file_list):
    accuracies = []
    for file_path in file_list:
        if os.path.exists(file_path):
            with open(file_path, "r", encoding="utf-8") as f:
                data = json.load(f)
                accuracies.append(data.get("overall_accuracy", 0.0) * 100)
        else:
            print(f"Warnung: Die Datei '{file_path}' existiert nicht.")
            accuracies.append(0.0)
    return accuracies


acc_hpcp = load_accuracies(json_files_hpcp)
acc_ml = load_accuracies(json_files_ml)

# 3. Plot konfigurieren (Thesis-optimiert)
fig, ax = plt.subplots(figsize=(10, 6))

# X-Koordinaten für die Gruppen berechnen
x = np.arange(len(test_labels))
width = 0.35  # Breite der Balken

# Balken für HPCP zeichnen (nach links verschoben)
bars_hpcp = ax.bar(
    x - width / 2,
    acc_hpcp,
    width,
    label="HPCP (Klassische DSP)",
    color="#4C72B0",
    zorder=3,
)

# Balken für ML zeichnen (nach rechts verschoben)
bars_ml = ax.bar(
    x + width / 2, acc_ml, width, label="Deep Learning", color="#DD8452", zorder=3
)

# Titel und Achsenbeschriftungen
ax.set_title(
    "Vergleich der Baseline-Genauigkeit: HPCP vs. Deep Learning", fontsize=15, pad=15
)
ax.set_ylabel("Genauigkeit (%)", fontsize=12)
ax.set_xticks(x)
ax.set_xticklabels(test_labels, fontsize=11)

# Legende hinzufügen
ax.legend(fontsize=11, loc="upper left")

# Y-Achse sinnvoll skalieren (Damit oben Platz für die Text-Labels bleibt)
max_val = max(max(acc_hpcp), max(acc_ml)) if acc_hpcp and acc_ml else 100
ax.set_ylim(0, max_val + 15)

# Schwache horizontale Linien für gute Ablesbarkeit
ax.grid(axis="y", linestyle="--", alpha=0.7, zorder=0)

# 4. Exakte Prozentwerte (auf 2 Nachkommastellen) über die Balken schreiben
ax.bar_label(
    bars_hpcp, fmt="%.2f%%", padding=5, fontsize=10, color="#333333", fontweight="bold"
)
ax.bar_label(
    bars_ml, fmt="%.2f%%", padding=5, fontsize=10, color="#333333", fontweight="bold"
)

# Layout optimieren
plt.tight_layout()

# 5. Bild exportieren und anzeigen
output_file = "vergleich_hpcp_ml_barchart.png"
plt.savefig(output_file, dpi=300)
print(f"Erfolgreich! Graph gespeichert als '{output_file}'.")

plt.show()
