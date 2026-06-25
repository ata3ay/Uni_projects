# TU Graz projects

This repository contains my academic and personal programming projects.
For clarity, all projects are grouped into three categories.

---

## 📁 TU_Graz_programming_projects

This folder includes **C programming projects** developed during my studies at **TU Graz**.
All source files are stored directly in this repository.

### Examples of projects:
- `blackjack.c` — Blackjack game using dynamic memory, structs, enums, and file I/O  
- `cinema_project.c` — Cinema booking system  
- `image_structuring_program.c` — Image processing with BMP structures  
- `stringtango.c` — Text editor project with dynamic memory management  
- `password_validator.c` — String and pointer-based validation logic  

These projects demonstrate:
- structured programming in C  
- dynamic memory management  
- pointers, arrays, structs, enums  
- file input/output  

---

## 📁 TU_Graz_computer_systems_project

This folder contains projects related to **Computer Systems, Networks, and LLM Security**.

Instead of duplicating repositories, each project is referenced via a text file:

- `GitHub_link.txt` — contains a direct link to the corresponding GitHub repository

### 🔐 LLM Friend or Foe — Guardrail Pipeline for LLM Safety

This project explores **security risks of Large Language Models (LLMs)** and demonstrates how **intent-shifted prompts** can be detected and mitigated using a **judge-then-answer guardrail pipeline**.

The system analyzes each user query before responding by:
- detecting intent shifts, harmful domains, and jailbreak attempts,
- assigning a risk level (LOW / MEDIUM / HIGH),
- applying a safety policy (ALLOW / SAFE_COMPLETE / REFUSE),
- generating only **non-actionable, safe responses**.

The project was developed as part of the course  
**“LLM – Friend or Foe: Security Risks & Defences”** and focuses on **LLM safety, misuse prevention, and explainable defensive design**.

📎 The full implementation is available via the GitHub link in this folder.

---

## 📁 TU_Graz_stats_project

This folder includes projects related to **statistics, data analysis, and quantitative methods**.

Project repositories are linked via:
- `GitHub_link.txt` — direct link to the full project repository

### 📊 CMS Project — Salary Analysis in AI and Data Science

This project analyzes salary data of professionals working in **Artificial Intelligence and Data Science** to study the impact of work experience on earnings.

The main research question is:
**Do professionals with more than five years of experience earn significantly more than those with 0–5 years of experience?**

Key aspects of the project:
- dataset from Kaggle (AI and Data Science Job Salaries, 2020–2025),
- data preprocessing with currency normalization (USD only),
- grouping by experience level (0–5 years vs. >5 years),
- statistical analysis using:
  - two-sample t-test,
  - bootstrapping for confidence intervals.

The project demonstrates:
- applied statistical reasoning,
- hypothesis testing,
- robustness analysis using resampling methods.

📎 The full implementation and code are available via the GitHub link in this folder.

---

## ℹ️ Notes

- Some university projects are stored in separate repositories due to course regulations.
- This portfolio repository serves as a **structured overview** of my work.
- Each linked project contains its own documentation.

---

## 👤 Author

**Dinmukhamed Atabay**  
Software Engineering Student at TU Graz

**My instagram: @ata3ay**
