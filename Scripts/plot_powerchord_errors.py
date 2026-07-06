import json
import os
from collections import Counter

import matplotlib.pyplot as plt

# 1. Dateipfade definieren (Exakt anpassen!)
file_hpcp = "C:\\dev\\ACR\\TestResults\\ThesisTests\\results_HPCP_newBase.json"
file_ml = "C:\\dev\\ACR\\TestResults\\results_DeepChroma_WithKeySmoothing.json"


def categorize_error(gt, pred):
    # 1. Prüfung auf Stille / Kein Akkord erkannt
    # (Manche Systeme nutzen einen leeren String "", manche "N" für No Chord)
    if pred == "" or pred == "N":
        return "Stille (Kein Akkord)"

    # 2. Grundtöne extrahieren
    # Nimmt an, dass die Labels nach dem Muster "C Maj", "C# Min", "A5" oder "A 5" aufgebaut sind.
    # Wir entfernen die '5' und nehmen alles vor dem ersten Leerzeichen.
    gt_root = gt.replace("5", "").strip().split(" ")[0]
    pred_root = pred.strip().split(" ")[0]

    # 3. Prüfung auf falsche Root Note
    if gt_root != pred_root:
        return "Falsche Root Note"

    # 4. Wenn die Root Note stimmt, prüfe auf Dur/Moll
    if "Maj" in pred:
        return "Fälschlich als Dur (Dur-Bias)"
    elif "Min" in pred:
        return "Fälschlich als Moll"
    else:
        return "Sonstige (Gleiche Root)"


def analyze_powerchord_errors_grouped(filepath):
    if not os.path.exists(filepath):
        print(f"Warnung: Datei {filepath} nicht gefunden.")
        return None, 0, 0, None

    with open(filepath, "r", encoding="utf-8") as f:
        data = json.load(f)

    test_name = data.get("test_name", os.path.basename(filepath))

    total_powerchords = 0
    incorrect_powerchords = 0
    error_counter = Counter()

    # Durch alle Songs und Frames iterieren
    for song in data.get("songs", []):
        for frame in song.get("frames", []):
            gt = frame.get("ground_truth", "")
            pred = frame.get("prediction", "")

            # Prüfen, ob Ground Truth ein Powerchord ist (enthält '5')
            if "5" in gt:
                total_powerchords += 1

                # Prüfen, ob die Vorhersage fehlerhaft war
                if gt != pred:
                    incorrect_powerchords += 1

                    # Fehler kategorisieren anstatt das exakte Label zu nehmen
                    category = categorize_error(gt, pred)
                    error_counter[category] += 1

    return test_name, total_powerchords, incorrect_powerchords, error_counter


# 2. Daten analysieren
hpcp_name, hpcp_total, hpcp_incorrect, hpcp_errors = analyze_powerchord_errors_grouped(
    file_hpcp
)
ml_name, ml_total, ml_incorrect, ml_errors = analyze_powerchord_errors_grouped(file_ml)


# 3. Konsolen-Tabelle ausgeben
def print_top_categories(name, total, incorrect, counter):
    if total == 0:
        return
    print(f"\n{'=' * 60}")
    print(f"TESTFALL: {name}")
    print(f"{'=' * 60}")
    print(f"Powerchord-Frames gesamt: {total}")
    print(f"Davon falsch klassifiziert: {incorrect} ({incorrect / total * 100:.2f}%)")
    print("-" * 60)
    print(f"Fehler-Kategorien (relativ zu allen Powerchord-Fehlern):")

    # Wir nehmen hier die Top 3, da wir maximal 4-5 Kategorien haben
    top_3 = counter.most_common(4)
    for i, (category, count) in enumerate(top_3, 1):
        percentage = (count / incorrect) * 100
        print(f"{i}. {category:<30} | {count:5d} Frames | {percentage:5.2f}%")
    print("=" * 60)


if hpcp_total:
    print_top_categories(hpcp_name, hpcp_total, hpcp_incorrect, hpcp_errors)
if ml_total:
    print_top_categories(ml_name, ml_total, ml_incorrect, ml_errors)

# 4. Visualisierung (Horizontales Balkendiagramm für die Thesis)
if hpcp_total and ml_total:
    # Daten für den Plot vorbereiten
    hpcp_top3 = hpcp_errors.most_common(3)
    ml_top3 = ml_errors.most_common(3)

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))

    # Plot für HPCP
    hpcp_labels = [item[0] for item in hpcp_top3][::-1]  # Umkehren für Top-Down Ansicht
    hpcp_pcts = [(item[1] / hpcp_incorrect) * 100 for item in hpcp_top3][::-1]

    bars1 = ax1.barh(hpcp_labels, hpcp_pcts, color="#4C72B0")
    ax1.set_title(f"HPCP: Fehlerkategorien bei Powerchords", fontsize=13, pad=15)
    ax1.set_xlabel("Anteil an allen Powerchord-Fehlern (%)", fontsize=11)
    ax1.set_xlim(0, max(hpcp_pcts + [0]) + 15)
    ax1.bar_label(bars1, fmt="%.1f%%", padding=5, color="#333333", fontweight="bold")

    # Plot für ML
    ml_labels = [item[0] for item in ml_top3][::-1]
    ml_pcts = [(item[1] / ml_incorrect) * 100 for item in ml_top3][::-1]

    bars2 = ax2.barh(ml_labels, ml_pcts, color="#DD8452")
    ax2.set_title(
        f"Deep Learning: Fehlerkategorien bei Powerchords", fontsize=13, pad=15
    )
    ax2.set_xlabel("Anteil an allen Powerchord-Fehlern (%)", fontsize=11)
    ax2.set_xlim(0, max(ml_pcts + [0]) + 15)
    ax2.bar_label(bars2, fmt="%.1f%%", padding=5, color="#333333", fontweight="bold")

    plt.tight_layout()
    plt.savefig("powerchord_fehler_gruppiert.png", dpi=300)
    print("\nVisualisierung als 'powerchord_fehler_gruppiert.png' gespeichert.")
    plt.show()
