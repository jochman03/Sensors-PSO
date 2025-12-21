# PSO Sensor Placement

This project implements **Particle Swarm Optimization (PSO)** to solve a sensor placement problem on a 2D grid loaded from a bitmap image.

The goal is to place sensors of different types in predefined candidate locations while:
- respecting a total budget,
- respecting per-type sensor availability,
- satisfying coverage requirements for each point,
- minimizing uncovered points and total cost.

The application includes a graphical UI built with **raylib** for configuration, visualization, and result export.

---

## Features

- Bitmap-based input (sensor candidates and coverage points)
- Multiple sensor types with configurable:
  - range
  - cost
  - availability
- PSO-based optimization with tunable parameters
- Real-time progress visualization
- Result preview with coverage visualization
- Export:
  - solution image (PNG)
  - solution data (JSON)

---

## Input Image Format

The input is a BMP image with color-coded pixels:

| Color (RGB)        | Meaning                          |
|--------------------|----------------------------------|
| (0, 0, 255)        | Sensor candidate position        |
| (0, 255, 0)        | Coverage point (coverage = 1)    |
| (0, 150, 0)        | Coverage point (coverage = 2)    |
| (0, 100, 0)        | Coverage point (coverage = 3)    |

Maximum supported image size: **1024 × 1024**  
Larger images are automatically downscaled.

---

## Algorithm Overview

- Each particle represents an assignment of sensor types to candidate positions.
- Continuous PSO positions are rounded to discrete sensor types:
  - `0` = no sensor
  - `1..N` = sensor type
- The objective function consists of:
  - total sensor cost
  - coverage penalty (uncovered points)
  - quantity penalty (exceeding availability)
  - budget penalty (exceeding total budget)

Lower objective value means a better solution.

---

## Configuration Parameters

### PSO Parameters
- `Particles number`
- `Total iterations`
- `c1`, `c2` – cognitive and social coefficients
- `w` – inertia weight

### Penalties
- `Coverage penalty`
- `Quantity penalty`
- `Budget penalty`

### Sensor Parameters
For each sensor type:
- Range
- Price
- Quantity

---

## Output

After the simulation finishes:
- Coverage and sensor placement are displayed visually.
- Summary statistics are shown:
  - total cost
  - objective function value
  - uncovered points
  - used sensors per type
- Results can be exported as:
  - **PNG image**
  - **JSON file** containing full solution data

---

## Dependencies

- [raylib](https://www.raylib.com/)
- [cJSON](https://github.com/DaveGamble/cJSON)

---

## Build Notes

This project is written in **C** and uses a simple procedural structure with shared global state between the UI and the PSO algorithm.

Recommended:
- GCC or Clang
- C99 or newer

---

## Notes

- Sensor type `0` always means *no sensor placed*.
- The application is single-threaded; PSO iterations are processed in small chunks per frame to keep the UI responsive.
- Large images and many sensor points can significantly increase memory usage.

---

## License

MIT (or whatever you decide)
