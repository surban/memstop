# Memstop

A lightweight `LD_PRELOAD` shared object that delays process execution when system memory is critically low. Memstop monitors available memory and waits until a configurable percentage of memory becomes available before allowing the application to start.

## Purpose

Memstop is designed to prevent crashes caused by memory exhaustion in parallel processing systems. It can be particularly useful in:

- Parallel build systems (like `make -j`) where you want to prevent the build from failing due to the OOM (out-of-memory) killer
- Batch processing systems where you want to ensure adequate memory before starting jobs
- High-memory applications that might crash the system if started when memory is low

## How It Works

When loaded as a shared object (via `LD_PRELOAD`), memstop automatically runs before your application's and all subprocesses' `main()` functions. It:

1. Reads system memory information from `/proc/meminfo`
2. Calculates the required available memory percentage (default: 10%)
3. Waits until the specified percentage of memory is available
4. Releases control to your application once memory requirements are met

## Build

### Prerequisites

- GCC compiler
- Linux system with `/proc/meminfo` support
- Make

### Building from Source

```bash
# Build the shared library
make

# The build creates memstop.so
```

## Install

### System-wide Installation

```bash
# Install to /usr/local/lib (requires root privileges)
sudo make install
```

### Manual Installation

```bash
# Copy to a directory in your library path
cp memstop.so /path/to/your/lib/directory
```

## Usage

### Basic Usage

Use `LD_PRELOAD` to load memstop before running your build process:

```bash
LD_PRELOAD=/usr/local/lib/memstop.so make -j
```

### Configuration

Memstop is configured using environment variables:

#### MEMSTOP_PERCENT

Sets the percentage of total memory that must be available before allowing execution.

```bash
# Require 20% of total memory to be available before make can spawn a task
MEMSTOP_PERCENT=20 LD_PRELOAD=/usr/local/lib/memstop.so make -j

# Require 5% of total memory to be available
MEMSTOP_PERCENT=5 LD_PRELOAD=/usr/local/lib/memstop.so make -j
```

- **Default**: 10
- **Range**: 0-100

#### MEMSTOP_VERBOSE

Enables verbose output showing memory statistics and blocking status.

```bash
# Enable verbose mode
MEMSTOP_VERBOSE=1 LD_PRELOAD=/usr/local/lib/memstop.so your_application
```

When enabled, memstop outputs to stderr:
- Current memory statistics (required %, available %, MB available/total)
- Hold notifications when delaying execution
- Release notifications when allowing execution to continue

**Warning:** Verbose mode can interfere with normal process execution and 
should **normally be disabled**.

##### Sample Output

With `MEMSTOP_VERBOSE=1`:

```
==== memstop: 10% required, 15% available (1234 MB / 8192 MB) ====
```

If memory is insufficient:
```
==== memstop: 10% required, 5% available (410 MB / 8192 MB) ====
==== memstop hold ====
(after enough memory becomes available)
==== memstop release ====
```

## License

This project is licensed under the GNU General Public License v3.0 (GPLv3).

