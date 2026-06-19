import numpy as np
import matplotlib.pyplot as plt
import os

WIDTH = 736
HEIGHT = 736

def load_raw(filename):
    if not os.path.exists(filename):
        print(f"Could not find {filename}")
        return np.zeros((HEIGHT, WIDTH), dtype=np.uint8)
    return np.fromfile(filename, dtype=np.uint8).reshape((HEIGHT, WIDTH))

# Load all 4 images
blur_scalar = load_raw("blur.raw")
blur_rvv    = load_raw("blur_rvv.raw")
mag_scalar  = load_raw("mag.raw")
mag_rvv     = load_raw("mag_rvv.raw")

# Create a 2x2 grid for comparison
fig, axes = plt.subplots(2, 2, figsize=(12, 12))

# Top Left: Scalar Blur
axes[0, 0].imshow(blur_scalar, cmap="gray")
axes[0, 0].set_title("Gaussian Blur (Scalar)", fontsize=14)
axes[0, 0].axis("off")

# Top Right: RVV Blur
axes[0, 1].imshow(blur_rvv, cmap="gray")
axes[0, 1].set_title("Gaussian Blur (RVV)", fontsize=14)
axes[0, 1].axis("off")

# Bottom Left: Scalar Magnitude
axes[1, 0].imshow(mag_scalar, cmap="gray")
axes[1, 0].set_title("Sobel Magnitude (Scalar)", fontsize=14)
axes[1, 0].axis("off")

# Bottom Right: RVV Magnitude
axes[1, 1].imshow(mag_rvv, cmap="gray")
axes[1, 1].set_title("Sobel Magnitude (RVV)", fontsize=14)
axes[1, 1].axis("off")

plt.tight_layout()
output_file = "final_comparison.png"
plt.savefig(output_file, bbox_inches="tight", dpi=150)
print(f"Success! 2x2 grid saved to {output_file}")