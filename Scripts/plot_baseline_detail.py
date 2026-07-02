import json
import os

import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

# 1. Dateipfad zur JSON-Datei definieren (Anpassen!)
json_file = "C:\\dev\\ACR\\TestResults\\ThesisTests\\results_ML_Base.json"

# 2. Datenstruktur für Pandas vorbereiten
data_list = []

if os.path.exists(json_file):
    with open(json_file, "r", encoding="utf-8") as f:
        data = json.load(f)

        # Auf die Liste der einzelnen Tracks zugreifen
        songs = data.get("songs", [])

        for song in songs:
            # ".wav" entfernen, falls es im Namen steht
            track_name = song.get("track_name", "").replace(".wav", "")
            accuracy = song.get("track_accuracy", 0.0) * 100

            # Verzerrungsgrad und Basis-Namen durch String-Matching extrahieren
            if track_name.endswith("_highGain"):
                base_name = track_name.replace("_highGain", "")
                gain_level = "High Gain"
            elif track_name.endswith("_lowGain"):
                base_name = track_name.replace("_lowGain", "")
                gain_level = "Low Gain"
            elif track_name.endswith("_noGain"):
                base_name = track_name.replace("_noGain", "")
                gain_level = "Clean"
            else:
                continue

            data_list.append(
                {
                    "Testfall": base_name,
                    "Verzerrung": gain_level,
                    "Genauigkeit": accuracy,
                }
            )
else:
    print(f"Fehler: Datei '{json_file}' nicht gefunden.")

# DataFrame aus der Liste erstellen
df = pd.DataFrame(data_list)

# 3. Daten sortieren und ordnen
# NEU: X-Achse nach "Clean"-Genauigkeit absteigend sortieren
# a) Filtere nur die "Clean"-Ergebnisse und sortiere sie absteigend nach Genauigkeit
clean_df = df[df["Verzerrung"] == "Clean"].sort_values(
    by="Genauigkeit", ascending=False
)
# b) Extrahiere die Testfall-Namen in dieser absteigenden Reihenfolge als Liste
sorted_testfaelle = clean_df["Testfall"].tolist()

# c) Fallback: Falls ein Testfall zufällig keinen "Clean"-Eintrag hat, wird er hinten angehängt
missing = [tf for tf in df["Testfall"].unique() if tf not in sorted_testfaelle]
sorted_testfaelle.extend(missing)

# d) Wende diese sortierte Liste als feste Kategorie-Reihenfolge auf die Spalte 'Testfall' an
df["Testfall"] = pd.Categorical(
    df["Testfall"], categories=sorted_testfaelle, ordered=True
)

# Sicherstellen, dass die Reihenfolge der Linien logisch ist (Clean -> Low -> High)
gain_order = ["Clean", "Low Gain", "High Gain"]
df["Verzerrung"] = pd.Categorical(df["Verzerrung"], categories=gain_order, ordered=True)

# 4. Plot erstellen
sns.set_theme(style="whitegrid")
plt.figure(figsize=(12, 7))

# Liniengraph mit Seaborn
sns.lineplot(
    data=df,
    x="Testfall",
    y="Genauigkeit",
    hue="Verzerrung",
    style="Verzerrung",  # Setzt unterschiedliche Marker für s/w-Druckbarkeit
    markers=["o", "s", "D"],  # Kreis, Quadrat, Diamant
    dashes=False,
    linewidth=2.5,
    markersize=8,
    palette=["#2ca02c", "#ff7f0e", "#d62728"],  # Grün, Orange, Rot
)

# 5. Styling: Titel und Labels
plt.title(
    "Einfluss der Verzerrung auf die Akkorderkennung (ML Baseline)",
    fontsize=15,
    pad=15,
)
plt.xlabel("Testfall", fontsize=12)
plt.ylabel("Genauigkeit (%)", fontsize=12)

# X-Achsen-Beschriftung rotieren, falls die Track-Namen lang sind
plt.xticks(rotation=30, ha="right")

# Legende anpassen
plt.legend(title="Verzerrungsgrad", fontsize=11, title_fontsize=12)

# Y-Achse Limit setzen (für bessere Vergleichbarkeit immer 0 bis 100)
plt.ylim(0, 105)

plt.tight_layout()

# 6. Bild speichern und anzeigen
output_file = "ml_baseline_details.png"
plt.savefig(output_file, dpi=300)
print(f"Erfolgreich! Graph gespeichert als '{output_file}'.")

plt.show()
