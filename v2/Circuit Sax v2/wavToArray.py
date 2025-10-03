import numpy as np
import scipy.io.wavfile as wav
from scipy.signal import resample

# Load WAV file
rate, data = wav.read("magbakk2.wav")

# If stereo, use one channel
if data.ndim > 1:
    data = data[:, 0]

# Normalize to -1.0 to 1.0 range
data = data.astype(np.float32)
data /= np.max(np.abs(data))

# Resample to 256 samples
resampled = resample(data, 256)

# Scale to int16 range
resampled_int16 = np.int16(resampled * 32767)

# Output C array
print("const int16_t magbakk[256] = {")
for i, val in enumerate(resampled_int16):
    comma = "," if i < 255 else ""
    print(f"  {val}{comma}")
print("};")