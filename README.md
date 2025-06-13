# 🎮 EntityUnknown: A 2.5D Game in the Making (WIP)

![Gameplay Demo](gifs/output-2.gif)

> **EntityUnknown** is a **work-in-progress 2.5D game** being built **entirely from scratch** using raw **Win32 API**, **DirectX 11**, and **C++20** — without relying on any prebuilt engine or graphics library.

---

## 🚧 Project Status: 10–20% Complete

This project is currently in **active development** and focuses on building a robust low-level game architecture from the ground up. It is **not a complete game** yet — but a technical foundation is already in place.

---

## ✅ What’s Done So Far

### 🪟 1. **Windows System**
- Built using raw **Win32 API**
- Custom **keyboard & mouse input handlers**
  - Key state tracking (`pressed`, `down`, `released`)
  - Raw mouse input with cursor lock/hide support

### 🔔 2. **Event Bus**
- Lightweight **observer/event system**
- Allows decoupled communication between systems (e.g., input to gameplay, or UI to renderer)

### 🖼️ 3. **Render System**
- Dual-queue design:
  - **2D Rendering Queue** (UI, HUD, sprites)
  - **3D Rendering Queue** (world objects)
- Custom vertex/index buffer management

### 🖌️ 4. **Dynamic Object Rendering**
- Support for creating and drawing **dynamic 2D and 3D images**
- Easily extendable for animated objects or tilemaps

### 🧵 5. **Texture Loader**
- Fully custom **TGA loader**
  - Supports RLE compression and alpha transparency
- Prepares `ID3D11ShaderResourceView` directly from raw texture files

### 💡 6. **Dynamic Lighting Support**
- Add any number of **lights (point, spot, directional)**
- Supports selecting nearby lights per object (e.g., max 5)
- Light buffer system designed for scalability

---

## 🔜 Planned Features

- 🗺️ **Level Editor**
  - Real-time in-editor object placement
  - Save/load scenes to file
- 🧩 **Entity + Component System**
  - Modular gameplay logic structure
- 🎛️ **Responsive In-Game UI**
  - Menus, health bars, pause/settings
- 🧰 **ImGui Integration**
  - Editor tools for debugging, inspecting, and controlling objects
- 💾 **Save/Load Mechanics**
  - JSON-style save data with custom file system

---

## 🧱 Tech Stack

| Subsystem        | Technology     |
|------------------|----------------|
| Language         | C++20          |
| OS/Platform      | Windows (x64)  |
| API              | Win32 API      |
| Renderer         | DirectX 11     |
| UI Tools (planned) | ImGui       |
| Input            | Raw mouse + keyboard (WM_INPUT) |
| Assets           | TGA format (custom loader) |

## 🧰 Build Instructions

### Requirements:
- Windows 10/11
- Visual Studio 2022
- Windows SDK (for DirectX 11)
- C++20 enabled

### Building:
```bash
git clone https://github.com/shrutibarar08/EntityUnknown.git
cd EntityUnknown
# Open the .sln in Visual Studio and build
