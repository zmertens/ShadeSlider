## Shade Slider
This game is about using a color picker to try and match the color with a reference image
https://i.imgur.com/NG5zh0I.gifv

## Install and Play
Android SDK+NDK. You don't need Android Studio but it makes updating easier!
    1. Download Android SDK, https://developer.android.com/sdk/index.html, and install API 18.
    2. Download latest Android NDK, https://developer.android.com/ndk/downloads/index.html, (tested with NDK-r15c).
    3. Export the Android SDK and NDK PATH:
        - `export PATH=$PATH:~/Android/android-ndk-r15c`
        - `export PATH=$PATH:~/Android/Sdk/platform-tools`
        - `export PATH=$PATH:~/Android/Sdk/tools`
        You should be able to run `adb logcat`, and `android` commands in terminal.
    4. Gradle Wrapper commands (from top-level android-project directory)
        - `./gradlew installDebug`, `./gradlew assemble`, `./gradlew build connectedCheck`,
        `./gradlew connectedAndroidTest`, `./gradlew clean`, `./gradlew test`
    5. adb commands: `adb devices`, `adb logcat`

See the GitHub release section for a downloadable APK
https://github.com/zmertens/ShadeSlider/releases/tag/1.0

## References and Links
 - [Android Asset Studio, Icon Generator](https://romannurik.github.io/AndroidAssetStudio/index.html)