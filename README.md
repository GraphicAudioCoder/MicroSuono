# MicroSuono - Quick Guide

## ⚡ Essential Commands

```bash
# Initial setup (first time)
cmake -B build

# Compile
cmake --build build

# Run
./build/MicroSuono

# Compile + Run
cmake --build build && ./build/MicroSuono

# Clean everything
rm -rf build/
```

## 🔧 After modifying CMakeLists.txt

```bash
cmake -B build              # Reconfigure
cmake --build build         # Recompile
# In Neovim: :LspRestart    # Restart LSP
```

## 🧹 Cleaning

```bash
# Only compiled files
cmake --build build --target clean

# Everything (removes config as well)
rm -rf build/
```

---

**LSP Notes**: The `.clangd` file and `CMAKE_EXPORT_COMPILE_COMMANDS` are already configured to make clangd work automatically.
