# clieditor

A simple, terminal-based text editor written in C from scratch. This project is an exercise in building a terminal user interface (TUI) application, managing low-level terminal I/O, and structuring a C application in a modular way.

## Core Architecture

The editor is built on a layered architecture, where higher-level components depend on lower-level ones, but not the other way around. This separation of concerns makes the codebase easier to understand, maintain, and extend.

The main data flow is as follows:
1.  **Input**: The `io` layer reads raw byte sequences from the terminal.
2.  **Processing**: The input is passed up to the `widgets` layer, where the active widget (e.g., the editor view) interprets the input and updates its state.
3.  **Data Model Update**: The widget may modify the core data model (e.g., the text in the `document` layer).
4.  **Drawing**: The widget tree is recursively drawn onto an abstract `Canvas` provided by the `display` layer. This is a "back-buffer" that holds the desired state of the screen.
5.  **Rendering**: The `Screen` module (in the `io` layer) compares the `Canvas` to its previous state and generates the minimal set of ANSI escape codes needed to update the physical terminal screen.

This can be visualized as a stack:

```
┌───────────────────┐
│      Widgets      │ (UI components like Editor, BottomBar)
└───────────────────┘
         │
         ▼
┌───────────────────┐
│      Document     │ (The actual text data model)
└───────────────────┘
         │
         ▼
┌───────────────────┐
│      Display      │ (Abstract drawing: Canvas, Cell, Style)
└───────────────────┘
         │
         ▼
┌───────────────────┐
│         IO        │ (Terminal setup, input, and screen rendering)
└───────────────────┘
         │
         ▼
┌───────────────────┐
│       Common      │ (Universal helpers: UTF-8, logging)
└───────────────────┘
```

## Directory Structure

The `src/` directory is organized by functional layers:

### `common/`
Contains fundamental, reusable modules that have no dependencies on any other part of the editor, such as UTF-8 string handling and logging utilities.

### `io/`
Handles all direct interaction with the outside world (the operating system, terminal, and file system). This is the only layer that contains platform-specific code, like reading from `stdin` or writing ANSI escape codes to the screen.

### `display/`
Provides the abstract, platform-independent graphics and UI framework. It knows *what* to draw, but not *how* to render it on a specific terminal. This includes the abstract `Canvas` and the base `Widget` structure, which forms the foundation of the UI tree.

### `document/`
Contains the core data model of the application, completely independent of the UI. Its main responsibility is managing the text buffer.

### `widgets/`
Contains all concrete, instantiable UI components that are built upon the base `Widget` from the `display` layer. Examples include the main editor view, status bars, and labels.

### `main.c` (root of `src/`)
The entry point of the application, responsible for initializing all systems and running the main loop.
