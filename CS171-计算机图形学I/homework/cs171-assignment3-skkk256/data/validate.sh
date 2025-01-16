#!/usr/bin/env bash

# You might use it?
# Config your rdr path here
RDR_EXEC="../build/src/renderer"
EXRTOOLS_EXEC="../build/src/exrtools"

CONFIGS=(env_sphere_1 glass_sphere cbox veach matpreview)

# Create a temporary directory to store the rendered images
echo "Executing in $(pwd)"
echo "Creating temporary directory at $(pwd)/output/"
OUTPUT_DIR="$(pwd)/output/$(date +%Y%m%d%H%M%S)"
mkdir -p "${OUTPUT_DIR}"

# Render all the images
for CONFIG in "${CONFIGS[@]}"; do
  RDR_CONFIG="${CONFIG}.json"
  echo ">>> =========================================="
  echo "Rendering with [ $RDR_CONFIG ]"
  time "${RDR_EXEC}" -q "$(pwd)/${RDR_CONFIG}" 2>/dev/null
  echo "Mitsuba: " $(${EXRTOOLS_EXEC} -a "$(pwd)/mitsuba/reference/${CONFIG}.exr")
  echo "RDR171:  " $(${EXRTOOLS_EXEC} -a "$(pwd)/${CONFIG}.exr")
  echo "[MSE]:   " $(${EXRTOOLS_EXEC} -mse "$(pwd)/${CONFIG}.exr" "$(pwd)/mitsuba/reference/${CONFIG}.exr")
  mv "$(pwd)/${CONFIG}.exr" "${OUTPUT_DIR}"
done
