# Classificator
## Goal
This class assigns musical chords to chroma frames. It acts as a pattern matching system, taking processed 12-dimensional chromagram features and identifying which chord template most closely resembles the pitch class distribution of a given timeframe.

## classifyFullChroma
Orchestrates the classification process for an entire audio buffer, mapping each frame of the chromagram to a chord index.

## classifyFrame
Classifies a single chroma frame by comparing it against all pre-generated chord templates. It uses a `similarityThreshold` to reject frames that don't strongly match any known chord. This is a deliberate design decision to prevent the system from confidently misclassifying noise or other segments where no clear harmonic structure is present.

## getGroupedSegments
Post-processes the raw frame-by-frame classifications into contiguous chord segments. Raw classifications can fluctuate rapidly due to passing notes, vibrato, or transient noises during chord transitions. By enforcing a `minSegmentLength`, this function acts as a temporal gate: a new chord is only accepted if the system consistently detects it over a sustained period. This drastically improves the robustness and readability of the final chord progression.

## generateTemplates
Initializes the idealized 12-dimensional chroma profiles for all recognizable chords (Major, Minor, and Powerchords) across all 12 root notes. Each template uses binary weights (`1.0` for active chord tones, `0.0` for others). 
While real-world instrument harmonics mean a played chord will have energy in non-chord tones, using strictly binary, theoretical templates provides a clean baseline. The complex harmonic mapping is already handled during the HPCP calculation in the `ChromaAnalyzer`.

## calculateCosineSimilarity
Computes how closely a chroma frame matches a specific chord template using cosine similarity. 
This metric calculates the cosine of the angle between two multi-dimensional vectors. It is specifically chosen over Euclidean distance because cosine similarity evaluates the *orientation* of the vectors (the relative distribution of pitch energies) while completely ignoring their *magnitude* (the overall volume or energy level of the frame). 

The mathematical formula used is:
$$\text{similarity} = \frac{A \cdot B}{\|A\| \|B\|}$$

Where $A$ is the chord template and $B$ is the current frame. The resulting score ranges from $0.0$ (completely orthogonal, no shared pitch classes) to $1.0$ (perfect match in relative energy distribution).
