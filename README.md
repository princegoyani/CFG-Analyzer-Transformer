# Grammar Analyzer

A C++ tool that reads a context-free grammar and performs various analyses based on a command-line task number.

## Overview

Supports six tasks:
1. List terminals and non-terminals  
2. Compute nullable non-terminals  
3. Calculate FIRST sets  
4. Calculate FOLLOW sets  
5. Left-factor the grammar  
6. Eliminate left-recursion  

Based on CSE340 Spring 2025 – Project 2 :contentReference[oaicite:1]{index=1}.

## Features

- **Flexible CLI**: Choose analysis via `./grammar_tool <task_number>`.
- **Robust Parsing**: Recursive-descent parser with syntax validation.
- **Grammar Transformations**: Left-factoring & left-recursion elimination with lexicographic ordering.
- **Comprehensive Output**: Exactly matches expected formats for Gradescope.

## Tech Stack

- C++11  
- GNU GCC on Ubuntu 22.04  
- Shell scripts for batch testing

## Repo Structure

