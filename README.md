# 🧮 Calculus Dash  


[![Made with C++](https://img.shields.io/badge/Made%20with-C++-blue.svg?style=for-the-badge)](https://isocpp.org/)
[![Made with Raylib](https://img.shields.io/badge/Graphics-Raylib-green.svg?style=for-the-badge)](https://www.raylib.com/)
[![Web Build: Emscripten](https://img.shields.io/badge/Build-Emscripten-orange.svg?style=for-the-badge)](https://emscripten.org/)
[![GitHub Pages](https://img.shields.io/badge/Hosted%20on-GitHub%20Pages-blueviolet.svg?style=for-the-badge)](https://pages.github.com/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)](LICENSE)

---

A **Geometry Dash-inspired** platformer made from scratch using **Raylib** and **pure C++**, then compiled to the web with **Emscripten**.  
It’s **fast**, **minimal**, and **hard as hell**—don’t say I didn’t warn you.  

🎮 **Play now:** [Calculus Dash](https://9midhunpm.github.io/CaclulusDash/webBuild)  

---

## 🚀 Features  
- **Pure C++ + Raylib**: No engines, no bloat—just raw code.  
- **Cross-Platform**: Runs on desktop and browser (thanks, Emscripten).  
- **Hardcore Gameplay**: Miss one jump, and you’re toast.  
- **Clean, Optimized Code**: Designed for speed and simplicity.  

---

## 🛠 Tech Stack  
- **Language:** C++  
- **Graphics Library:** [Raylib](https://www.raylib.com/)  
- **Web Build:** [Emscripten](https://emscripten.org/)  

---

## 📦 How to Build Locally  
### 1. Clone this repo  
```bash
git clone https://github.com/9MidhunPM/CaclulusDash.git
cd CaclulusDash
```
### 2. Compile for Desktop  
Make sure Raylib is installed. Then:  
```bash
g++ main.cpp -o CalculusDash -lraylib -std=c++17
./CalculusDash
```
### 3. Compile for Web  
You need Emscripten installed and activated:  
```bash
emcc main.cpp -o index.html -s USE_GLFW=3 -s FULL_ES3=1 -O3
```
## 🎯 Gameplay  
- **Goal:** Jump over spikes, survive as long as possible.  
- **Controls:**  
    - **Space / Up Arrow** → Jump  
    - (Maybe rage quit after 5 minutes)  

---

## 🏗 Project Structure  
CalculusDash/
│── main.cpp
│── assets/
│── webBuild/
│ └── index.html
└── README.md


---

## 🌐 Live Demo  
Play here:  
👉 [https://9midhunpm.github.io/CaclulusDash/webBuild](https://9midhunpm.github.io/CaclulusDash/webBuild)

---

## ⚠️ Difficulty Warning  
This game is not casual. It will break your patience. It will make you question your life choices. Good luck.  

---

## 📜 License  
MIT License – Do whatever you want, just give credit.
