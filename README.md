# MicroSuono - Quick Guide

## ⚡ Comandi Essenziali

```bash
# Configurazione iniziale (prima volta)
cmake -B build

# Compilare
cmake --build build

# Eseguire
./build/MicroSuono

# Compilare + Eseguire
cmake --build build && ./build/MicroSuono

# Pulire tutto
rm -rf build/
```

## 🔧 Dopo aver modificato CMakeLists.txt

```bash
cmake -B build              # Riconfigura
cmake --build build         # Ricompila
# In Neovim: :LspRestart    # Aggiorna LSP
```

## 🧹 Pulizia

```bash
# Solo file compilati
cmake --build build --target clean

# Tutto (rimuove anche config)
rm -rf build/
```

---

**Note LSP**: Il file `.clangd` e `CMAKE_EXPORT_COMPILE_COMMANDS` sono già configurati per far funzionare clangd automaticamente.
