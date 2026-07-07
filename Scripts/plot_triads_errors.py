import json
import os
from collections import Counter

import matplotlib.pyplot as plt

# 1. Dateipfade definieren (Exakt anpassen!)
file_hpcp = "C:\\dev\\ACR\\TestResults\\ThesisTests\\results_HPCP_newBase.json"
file_ml = "C:\\dev\\ACR\\TestResults\\results_DeepChroma_WithKeySmoothing.json"


def categorize_dur_moll_error(gt, pred):
    # 1. Prüfung auf Stille / Kein Akkord erkannt
    if pred == "" or pred == "N":
        return "Stille (Kein Akkord)"

    # 2. Grundtöne extrahieren
    gt_root = gt.split(" ")[0]

    # Prädiktion könnte ein Powerchord ("C5", "C 5") oder normales Label sein
    pred_root = pred.replace("5", "").strip().split(" ")[0] if pred else ""

    # 3. Prüfung auf falsche Root Note (komplett daneben gegriffen)
    if gt_root != pred_root:
        return "Falsche Root Note"

    # 4. Wenn die Root Note stimmt, prüfe auf die Art des Fehlers
    if "5" in pred:
        return "Fälschlich als Powerchord"

    # 5. Prüfung auf Tongeschlecht-Verwechslung (Dur zu Moll oder Moll zu Dur)
    if "Maj" in gt and "Min" in pred:
        return "Dur/Moll Verwechslung"
    elif "Min" in gt and "Maj" in pred:
        return "Dur/Moll Verwechslung"
    else:
        return "Sonstige (Gleiche Root)"


def analyze_dur_moll_errors_grouped(filepath):
    if not os.path.exists(filepath):
        print(f"Warnung: Datei {filepath} nicht gefunden.")
        return None, 0, 0, None

    with open(filepath, "r", encoding="utf-8") as f:
        data = json.load(f)

    test_name = data.get("test_name", os.path.basename(filepath))

    total_dur_moll = 0
    incorrect_dur_moll = 0
    error_counter = Counter()

    # Durch alle Songs und Frames iterieren
    for song in data.get("songs", []):
        for frame in song.get("frames", []):
            gt = frame.get("ground_truth", "")
            pred = frame.get("prediction", "")

            # Prüfen, ob Ground Truth Dur oder Moll ist
            if "Maj" in gt or "Min" in gt:
                total_dur_moll += 1

                # Prüfen, ob die Vorhersage fehlerhaft war
                if gt != pred:
                    incorrect_dur_moll += 1

                    # Fehler kategorisieren anstatt das exakte Label zu nehmen
                    category = categorize_dur_moll_error(gt, pred)
                    error_counter[category] += 1

    return test_name, total_dur_moll, incorrect_dur_moll, error_counter


# 2. Daten analysieren
hpcp_name, hpcp_total, hpcp_incorrect, hpcp_errors = analyze_dur_moll_errors_grouped(
    file_hpcp
)
ml_name, ml_total, ml_incorrect, ml_errors = analyze_dur_moll_errors_grouped(file_ml)


# 3. Konsolen-Tabelle ausgeben
def print_top_categories(name, total, incorrect, counter):
    if total == 0:
        return
    print(f"\n{'=' * 60}")
    print(f"TESTFALL: {name}")
    print(f"{'=' * 60}")
    print(f"Dur/Moll-Frames gesamt: {total}")
    print(f"Davon falsch klassifiziert: {incorrect} ({incorrect / total * 100:.2f}%)")
    print("-" * 60)
    print(f"Fehler-Kategorien (relativ zu allen Dur/Moll-Fehlern):")

    # Top 4 Kategorien ausgeben
    top_4 = counter.most_common(4)
    for i, (category, count) in enumerate(top_4, 1):
        percentage = (count / incorrect) * 100
        print(f"{i}. {category:<30} | {count:5d} Frames | {percentage:5.2f}%")
    print("=" * 60)


if hpcp_total:
    print_top_categories(hpcp_name, hpcp_total, hpcp_incorrect, hpcp_errors)
if ml_total:
    print_top_categories(ml_name, ml_total, ml_incorrect, ml_errors)

# 4. Visualisierung (Horizontales Balkendiagramm für die Thesis)
if hpcp_total and ml_total:
    # Daten für den Plot vorbereiten (Wir nehmen hier 4 Kategorien)
    hpcp_top = hpcp_errors.most_common(4)
    ml_top = ml_errors.most_common(4)

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))

    # Plot für HPCP
    hpcp_labels = [item[0] for item in hpcp_top][::-1]  # Umkehren für Top-Down Ansicht
    hpcp_pcts = [(item[1] / hpcp_incorrect) * 100 for item in hpcp_top][::-1]

    bars1 = ax1.barh(hpcp_labels, hpcp_pcts, color="#4C72B0")
    ax1.set_title(f"HPCP: Fehlerkategorien bei Dur/Moll", fontsize=13, pad=15)
    ax1.set_xlabel("Anteil an allen Dur/Moll-Fehlern (%)", fontsize=11)
    ax1.set_xlim(0, max(hpcp_pcts + [0]) + 15)
    ax1.bar_label(bars1, fmt="%.1f%%", padding=5, color="#333333", fontweight="bold")

    # Plot für ML
    ml_labels = [item[0] for item in ml_top][::-1]
    ml_pcts = [(item[1] / ml_incorrect) * 100 for item in ml_top][::-1]

    bars2 = ax2.barh(ml_labels, ml_pcts, color="#DD8452")
    ax2.set_title(f"Deep Learning: Fehlerkategorien bei Dur/Moll", fontsize=13, pad=15)
    ax2.set_xlabel("Anteil an allen Dur/Moll-Fehlern (%)", fontsize=11)
    ax2.set_xlim(0, max(ml_pcts + [0]) + 15)
    ax2.bar_label(bars2, fmt="%.1f%%", padding=5, color="#333333", fontweight="bold")

    plt.tight_layout()
    plt.savefig("dur_moll_fehler_gruppiert.png", dpi=300)
    print("\nVisualisierung als 'dur_moll_fehler_gruppiert.png' gespeichert.")
    plt.show()
