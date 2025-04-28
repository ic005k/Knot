name: Build Fcitx5 Qt6 Plugin

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]

jobs:
  build:
    runs-on: ubuntu-22.04

    steps:
    - name: Checkout code
      uses: actions/checkout@v2

    - name: Install system dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y \
          git \
          cmake \
          ninja-build \
          g++ \
          libxcb1-dev \
          libxkbcommon-dev \
          libxcb-xinerama0-dev \
          libxcb-util-dev \
          libgl1-mesa-dev \
          qtbase5-dev \
          qttools5-dev-tools

    - name: Install Qt 6.6.3
      run: |
        QT_INSTALLER_URL="https://download.qt.io/official_releases/qt/6.6/6.6.3/qt-unified-linux-x64-6.6.3-online.run"
        wget -O qt-installer.run "$QT_INSTALLER_URL"
        chmod +x qt-installer.run
        echo "yes" | ./qt-installer.run \
          --accept-license \
          --skip-components x11extras,webengine,webview,webchannel,quick3d,quickcontrols2,quicktimedesigner,qtcanvas3d,qtconnectivity,qtdatavis3d,qtgamepad,qtlocation,qttools,qttranslations \
          --components qt.base,qt.qt6.6.3.qtbase,qt.qt6.6.3.qttools \
          --install-dir $HOME/Qt/6.6.3/gcc_64

    - name: Setup environment variables
      run: |
        echo "export PATH=\"$HOME/Qt/6.6.3/gcc_64/bin:\$PATH\"" >> $GITHUB_ENV
        echo "export QT_SELECT=6"

    - name: Configure build
      working-directory: build
      run: cmake .. \
        -DCMAKE_PREFIX_PATH="$HOME/Qt/6.6.3/gcc_64" \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTING=OFF \
        -DCMAKE_INSTALL_PREFIX=./install

    - name: Build plugin
      working-directory: build
      run: cmake --build . --target fcitx5platforminputcontextplugin

    - name: Upload artifact
      uses: actions/upload-artifact@v2
      with:
        name: fcitx5platforminputcontextplugin
        path: build/install/lib/libfcitx5platforminputcontextplugin.so
