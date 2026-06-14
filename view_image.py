from PIL import Image
import numpy as np

width = 256
height = 256

try:
    # Load the raw file
    img = np.fromfile("assets/magnitude_l1.raw", dtype=np.uint8)

    # Reshape it to the correct dimensions
    img = img.reshape((height, width))

    # Save as PNG so you can view it easily in Windows
    output_image = Image.fromarray(img)
    output_image.save("check_edges.png")
    print("Success! Open check_edges.png in your folder to see the result.")
except Exception as e:
    print(f"Error: {e}")
