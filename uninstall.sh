#!/usr/bin/env bash
# uninstall.sh

# 1) Remove installed files from install_manifest.txt
if [ -f build/install_manifest.txt ]; then
    while IFS= read -r f; do
        echo "Removing $f"
        rm -f "$f"
    done < build/install_manifest.txt
fi

# 2) Systemd service removal (Linux)
if command -v systemctl >/dev/null 2>&1; then
    echo "Removing systemd service jarvis.service..."
    sudo systemctl stop jarvis.service
    sudo systemctl disable jarvis.service
    sudo rm -f /lib/systemd/system/jarvis.service
    sudo systemctl daemon-reload
fi

# 3) Windows service removal (if you detect Windows in e.g. Git Bash / MSYS)
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    echo "Removing Windows service JarvisServerService..."
    sc stop JarvisServerService
    sc delete JarvisServerService
fi

echo "Uninstall complete."