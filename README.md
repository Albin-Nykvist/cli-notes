# 📓 Quick & Lightweight Command-Line Notetaking

This is a minimalist **CLI notetaking tool**.  
It was built because I wanted a simple way to take notes during lectures, meetings, or any situation where it’s convenient to take notes from the terminal.  

This app separates **writing notes** from **organizing/saving notes**, so you can take notes instantly without worrying about organizing them.  

---

## ✨ Features
- ⚡ Ultra-fast notetaking from the command line  
- 📝 Review unsaved notes before committing them to collections  
- 📂 Organize notes into collections or delete them quickly  
- 📤 Export collections to text files for backup or sharing  
- 🧹 Lightweight C implementation, no external dependencies  

---

## 🚀 Usage

### 1. Writing notes
Write a note directly from your terminal:
```bash
note This is my note

### 2. Reviewing notes
Review notes and organize them into collections:
```bash
note -r
overfitting - too much focus on the training set
[save: s | delete: d | ignore: enter]>

### 3. View collections of notes
Handle collections of notes:
```bash
note -v
[1] School (#2)
[2] Machine_learning (#3)
[print: p + <number> | delete: d + <number> | export to textfile: e + <number> | create: c + <collection name> | quit: q]>