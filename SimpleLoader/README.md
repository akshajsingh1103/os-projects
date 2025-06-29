# 🧠 SimpleLoader

A C-based implementation of a **32-bit ELF executable loader**, written from scratch without using any external ELF-parsing libraries.

> Created as part of an Operating Systems assignment, but cleaned, modularized, and extended to meet bonus requirements like a boss 😤

---

## 📁 Folder Structure

```
SimpleLoader/
├── bin/           # Compiled output binaries (lib & launcher)
├── launcher/      # Launcher code to load + run the ELF binary
├── loader/        # Shared library that does the ELF parsing & loading
├── test/          # Sample ELF test file (fib.c)
└── Makefile       # Top-level makefile to build all modules
```

---

## ⚙️ How It Works

1. A test C file like `fib.c` is compiled to a **barebones 32-bit ELF binary** (no stdlib, no PIE).
2. The loader:
   - Parses the ELF header
   - Locates `PT_LOAD` segments
   - Uses `mmap` to load them into memory
   - Finds the `e_entry` address (i.e., `_start`)
   - Jumps to `_start` and captures its return value.
3. `lib_simpleloader.so` contains all this logic.
4. The `launcher` loads the ELF using this shared library.

---

## 🚀 How to Build & Run

### 🛠️ Build everything:
```bash
make
```

### ▶️ Run the test ELF (fib):
```bash
./bin/launch ./test/fib
```

Expected Output:
```
User _start return value = 102334155
```

---

## 🧠 Bonus Features Implemented

✅ Modular folder structure  
✅ Shared library `lib_simpleloader.so`  
✅ Launcher program using the loader as a lib  
✅ Memory mapping with `mmap()`  
✅ Full ELF parsing (headers + segments)  
✅ Top-level `Makefile` that builds everything recursively

---

## 💡 Notes

- Only supports **statically compiled 32-bit ELF executables**
- Designed to work with GCC flags:  
  `-m32 -no-pie -nostdlib`
- Tested in a **WSL Ubuntu environment**

---

