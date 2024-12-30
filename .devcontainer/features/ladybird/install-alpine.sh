#!/bin/sh
set -e

# Git and GitHub CLI
apk add git github-cli

# Ladybird dev dependencies
apk add autoconf-archive automake build-base ccache clang clang-dev cmake curl \
    diffutils font-liberation nasm ninja-build \
    ninja-is-really-ninja qt6-qtbase-dev qt6-qtmultimedia-dev qt6-qttools-dev \
    qt6-qtwayland-dev tar unzip zip
