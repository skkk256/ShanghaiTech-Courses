# Run Code
1. Install required package `opencv-python, scipy, numba, pandas, Pillow, tqdm, bagpy`
2. Open the `localization.ipynb` and click `Run All` if using VSCode
3. Modify the  `KLD`, `MCL` and `SAVE_GIF` variables as you need
4. The output path may occur some errors, modify them according to your local system


# Function Identification

Refer to the **annotation** in the notebook, we label the role of some key blocks and functions 

# Result

### Process

MCL

- noise 0.05, 0.05, 0.02, 0.05, particle_num 5000 ray_num 120 

<img src="https://raw.githubusercontent.com/skkk256/pic/main/2024-04-14/0f0ed4b5a5ee51c4ad7dd95cfb1f4434.png" alt="0f0ed4b5a5ee51c4ad7dd95cfb1f4434" style="zoom:80%;" /> <img src="https://raw.githubusercontent.com/skkk256/pic/main/2024-04-14/0413-234000_MCL_submit-result_5000_120.gif" alt="0413-234000_MCL_submit-result_5000_120" style="zoom:62%;" />

KLD 

- epsilon = 0.1 delta = 0.05 min_particles = 100 max_particles = 1000 bins = 100

![0414-004246_KCL_1000_120](https://raw.githubusercontent.com/skkk256/pic/main/2024-04-14/0414-004246_KCL_1000_120.gif)
