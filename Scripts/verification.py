import glob
import json
import os

import numpy as np

# Madmom fix für np.float, np.int, np.bool (neuere numpy Versionen)
np.float = float
np.int = int

from madmom.audio.chroma import DeepChromaProcessor


def load_json_matrix(filepath):
    """Lädt ein 2D-Array aus einer JSON-Datei."""
    with open(filepath, "r") as f:
        return np.array(json.load(f), dtype=np.float32)


def save_json_matrix(matrix, filepath):
    """Speichert ein 2D numpy-Array als JSON-Datei."""
    data = matrix.tolist()
    with open(filepath, "w") as f:
        json.dump(data, f)
    print(f"  -> Exportiert: {filepath}")


def calculate_madmom_chroma(audio_file_path):
    """Verarbeitet eine Audiodatei direkt mit der originalen Madmom-Pipeline."""
    processor = DeepChromaProcessor()
    return processor(audio_file_path)


def calculate_madmom_log_spectrogram(audio_file_path):
    """
    Kappt die Madmom-Pipeline VOR dem neuronalen Netz ab
    und gibt das reine logarithmische Spektrogramm zurück.
    """
    processor = DeepChromaProcessor()

    # Der DeepChromaProcessor hat intern eine Liste namens 'processors'.
    # Der letzte Eintrag (Index -1) ist das Neuronale Netz.
    # Alles davor ist die DSP-Pipeline (Audio -> FFT -> Filterbank -> Log).

    data = audio_file_path

    # Wir jagen das Audio durch alle DSP-Schritte, aber lassen das Netz weg!
    for step in processor.processors[:4]:
        data = step(data)

    return np.array(data, dtype=np.float32)


def debug_madmom_pipeline():
    """Hilfsfunktion: Zeigt dir einmalig an, welche Schritte Madmom intern macht."""
    processor = DeepChromaProcessor()
    print("\n--- MADMOM INTERNE PIPELINE ---")
    for i, step in enumerate(processor.processors):
        print(f"Schritt {i}: {step.__class__.__name__}")
    print("-------------------------------\n")


def find_audio_file(base_name):
    """Sucht nach einer Audiodatei mit bekannten Endungen für einen gegebenen Basisnamen."""
    for ext in [".wav", ".mp3", ".flac", ".aiff", ".aif", ".ogg"]:
        candidate = base_name + ext
        if os.path.exists(candidate):
            return candidate
    return None


def compare_matrices(matrix_a, matrix_b, label_a="A", label_b="B"):
    """Vergleicht zwei Matrizen mit mehreren Toleranzstufen."""
    if matrix_a.shape != matrix_b.shape:
        print(
            f"  -> SHAPE MISMATCH: {label_a} {matrix_a.shape} vs {label_b} {matrix_b.shape}"
        )
        min_frames = min(matrix_a.shape[0], matrix_b.shape[0])
        print(f"  -> Vergleiche erste {min_frames} Frames...")
        matrix_a = matrix_a[:min_frames]
        matrix_b = matrix_b[:min_frames]

    abs_diff = np.abs(matrix_a - matrix_b)
    total = matrix_a.size

    print(f"  -> Max. absoluter Fehler: {np.max(abs_diff):.6f}")
    print(f"  -> Mittlerer Fehler:      {np.mean(abs_diff):.6f}")
    print(f"  -> Median Fehler:         {np.median(abs_diff):.6f}")

    for tol in [1e-4, 1e-3, 1e-2, 5e-2, 1e-1]:
        matches = np.sum(abs_diff <= tol)
        pct = (matches / total) * 100.0
        print(f"  -> Genauigkeit (tol={tol:.0e}): {pct:6.2f}%")

    return abs_diff


def print_frame_comparison(
    matrix_a, matrix_b, label_a, label_b, frame_indices=None, max_bins=12
):
    """Zeigt Werte beider Matrizen fuer ausgewaehlte Frames nebeneinander."""
    if frame_indices is None:
        frame_indices = [0, 1, matrix_a.shape[0] // 2]

    num_bins = min(matrix_a.shape[1], max_bins)
    chroma_names = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]

    for fi in frame_indices:
        if fi >= matrix_a.shape[0] or fi >= matrix_b.shape[0]:
            continue
        print(f"\n  --- Frame {fi} ---")
        header = "  Note:  " + "".join(
            f"{chroma_names[i] if i < len(chroma_names) else i:>8}"
            for i in range(num_bins)
        )
        print(header)
        row_a = (
            "  "
            + f"{label_a}:".ljust(7)
            + "".join(f"{matrix_a[fi, i]:8.4f}" for i in range(num_bins))
        )
        row_b = (
            "  "
            + f"{label_b}:".ljust(7)
            + "".join(f"{matrix_b[fi, i]:8.4f}" for i in range(num_bins))
        )
        row_d = (
            "  "
            + "Diff:".ljust(7)
            + "".join(
                f"{abs(matrix_a[fi, i] - matrix_b[fi, i]):8.4f}"
                for i in range(num_bins)
            )
        )
        print(row_a)
        print(row_b)
        print(row_d)


def main():
    chroma_files = glob.glob("*_chroma.json")

    if not chroma_files:
        print("Keine Dateien mit der Endung '_chroma.json' gefunden.")
        return

    print("=" * 70)
    print("  C++ vs Python Madmom DeepChroma Verifikation")
    print("=" * 70)

    for chroma_file in sorted(chroma_files):
        base_name = chroma_file.replace("_chroma.json", "")
        print(f"\nVerarbeite: {base_name}")
        print("-" * 50)

        audio_file = find_audio_file(base_name)
        if audio_file is None:
            print(
                f"  -> Uebersprungen: Keine Audiodatei fuer '{base_name}' gefunden.\n"
            )
            continue
        print(f"  -> Audiodatei: {audio_file}")

        # C++ Chromagramm laden
        cpp_chromagram = load_json_matrix(chroma_file)
        print(f"  -> C++ Chroma Shape:    {cpp_chromagram.shape}")

        # Python Chroma berechnen
        try:
            python_chromagram = np.array(
                calculate_madmom_chroma(audio_file), dtype=np.float32
            )
            print(f"  -> Python Chroma Shape: {python_chromagram.shape}")
        except Exception as e:
            print(f"  -> Fehler in der Madmom-Verarbeitung: {e}\n")
            continue

        # Python Chroma als JSON exportieren
        python_chroma_file = base_name + "_chroma_python.json"
        save_json_matrix(python_chromagram, python_chroma_file)

        # Chroma Vergleich
        print("\n  === CHROMA VERGLEICH ===")
        compare_matrices(python_chromagram, cpp_chromagram, "Python", "C++")

        # Detaillierter Frame-Vergleich
        min_frames = min(python_chromagram.shape[0], cpp_chromagram.shape[0])
        print_frame_comparison(
            python_chromagram[:min_frames],
            cpp_chromagram[:min_frames],
            "Py",
            "C++",
            frame_indices=[0, 1, min_frames // 4, min_frames // 2, min_frames - 1],
        )

        # Wertebereich-Analyse
        print(f"\n  === WERTEBEREICH ===")
        print(
            f"  C++    -> min={cpp_chromagram.min():.4f}, max={cpp_chromagram.max():.4f}, mean={cpp_chromagram.mean():.4f}"
        )
        print(
            f"  Python -> min={python_chromagram.min():.4f}, max={python_chromagram.max():.4f}, mean={python_chromagram.mean():.4f}"
        )

        # Spektrogramm-Vergleich falls vorhanden
        specto_file = base_name + "_specto.json"
        if os.path.exists(specto_file):
            print(f"\n  === SPEKTROGRAMM (DNN-INPUT) VERGLEICH ===")
            try:
                cpp_spectro = load_json_matrix(specto_file)
                python_spectro = calculate_madmom_log_spectrogram(audio_file)

                print(f"  C++ Spectro Shape:    {cpp_spectro.shape}")
                print(f"  Python Spectro Shape: {python_spectro.shape}")

                compare_matrices(python_spectro, cpp_spectro, "Python", "C++")

                min_f = min(cpp_spectro.shape[0], python_spectro.shape[0])
                num_show = min(10, cpp_spectro.shape[1])
                print(f"\n  Spectro Frame 0, erste {num_show} Bins:")
                print(f"  C++   : {cpp_spectro[0][:num_show].round(4)}")
                print(f"  Python: {python_spectro[0][:num_show].round(4)}")

                python_spectro_file = base_name + "_specto_python.json"
                save_json_matrix(python_spectro, python_spectro_file)
            except Exception as e:
                print(f"  Fehler beim Spektrogramm-Vergleich: {e}")
        else:
            print(f"\n  (Kein C++ Spektrogramm '{specto_file}' gefunden)")

        print("\n")


if __name__ == "__main__":
    main()
