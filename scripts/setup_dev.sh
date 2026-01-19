#!/bin/bash
set -e

# Brain Replica - Development Setup Script
# This script installs necessary system dependencies for local C++ development.

echo "🧠 Initializing Brain Replica Development Environment..."

# Detect OS
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$NAME
else
    OS=$(uname -s)
fi

echo "📍 Detected OS: $OS"

case "$OS" in
    "Ubuntu"|"Debian GNU/Linux"|"Linux Mint"|"KDE neon")
        echo "📦 Installing dependencies via apt..."
        sudo apt-get update
        sudo apt-get install -y \
            build-essential \
            cmake \
            g++ \
            libtbb-dev \
            libsqlite3-dev \
            libhiredis-dev \
            libpq-dev \
            git \
            curl \
            python3 \
            python3-pip
        ;;
    "Arch Linux")
        echo "📦 Installing dependencies via pacman..."
        sudo pacman -S --needed \
            base-devel \
            cmake \
            gcc \
            tbb \
            sqlite \
            hiredis \
            postgresql-libs \
            git \
            python
        ;;
    "Darwin") # macOS
        echo "📦 Installing dependencies via brew..."
        if ! command -v brew &> /dev/null; then
            echo "❌ Homebrew not found. Please install it first at https://brew.sh/"
            exit 1
        fi
        brew install \
            cmake \
            tbb \
            sqlite \
            hiredis \
            libpq \
            python
        ;;
    *)
        echo "⚠️  Unsupported OS: $OS"
        echo "Please manually install: CMake, C++20 Compiler, SQLite3, Hiredis, and LibPQ."
        ;;
esac

echo "✅ System dependencies installed."

# Build the project
echo "🛠️  Performing initial build..."
mkdir -p build
cd build
cmake .. -DENABLE_POSTGRES=ON -DENABLE_REDIS=ON
make -j$(nproc) brain_replica

echo "🎉 Setup complete! You can now run the brain with: ./build/brain_replica"
echo "Note: If you don't have Postgres/Redis running locally, the brain will fallback to SQLite only."
