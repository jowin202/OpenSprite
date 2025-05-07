# OpenSprite

[![Windows C++ Qt Build](https://github.com/jowin202/OpenSprite/actions/workflows/windows-build.yml/badge.svg)](https://github.com/jowin202/OpenSprite/actions/workflows/windows-build.yml)

[![Mac OS C++ Qt Build](https://github.com/jowin202/OpenSprite/actions/workflows/mac-build.yml/badge.svg)](https://github.com/jowin202/OpenSprite/actions/workflows/mac-build.yml)

An open source sprite editor to generate sprites for the Commodore C64!

This GPL3 project uses the open source version of the [Qt Framework](https://download.qt.io).




### 🧑‍💻 macOS: First-time Launch Instructions

If you're downloading the app on macOS and receive a message saying:

> **"App is damaged and can’t be opened"**

Don’t worry — the app is **not actually corrupted**. This happens because it is **not signed with an Apple Developer ID**, and macOS adds a quarantine flag when downloading from the internet.

To fix this, follow these steps:

1. **Unpack the `.zip` or `.dmg` file** (e.g. by double-clicking it)
2. Open the **Terminal**
3. Run the following command to remove the quarantine flag:

```bash
xattr -d com.apple.quarantine /path/to/OpenSprite.app
```
