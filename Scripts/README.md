# Goal
This directory contains of several pyhton script files. It's goal is to help with the integration of the DeepChroma Neural Net from the `madmom` Library. Since this library is very old i had to use Python version 3.9.13 to get this running. Using newer versions i got errors installing the Cython dependency. 
## Generate Filterbank
This script extracts the weights of the logarithmic filterbank of the `madmon` Library. Those weights are used to multiply the linear scaled values of the fft and compress them into the logarithmic scale commanded by the model.
## Verification
This script loads in any chromagram in json format. It then compares it to the results of the chromagram produced by the `DeepChromaProcessor` of the `madmom` Library using the same audio file. The goal was to verify the correctness of the integration and portation of the model from Python to this C++ Application. With the lowest tolerance of 1e-04 i still scored a similarity of 86,11% which is pretty good. The remaining differences are probably due to float32 inprecision and different implementations of the hanning windowing function in JUCE and the `madmom` Library.

To use this script yourself you need to create a virtual environment (venv) in this directory. It must contain the dependencies of the requirements.txt file. Additionally you have to paste the audio file (wav format) and the chroma as a json in this directory. I left one audio file and a chroma.json in this directory as an example. The json file must be named the same as the audio file with a '_chroma.json' suffix.
