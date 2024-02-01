# Pidgin Paste Image

Allows pasting images into a chat or IM window from clipboard

![Recording 2024-02-01 at 16 44 00](https://github.com/EionRobb/pidgin-paste-image/assets/1063865/1a44c490-ce3e-4634-86d0-089e9303ad5a)


For protocols that allow sending inline images, just enable this plugin through: Tools -> Plugins -> Paste Images

Then either Ctrl+V or right-click and Paste to insert

## Windows Users
Download the latest .dll from https://github.com/EionRobb/pidgin-paste-image/releases/latest
Save to the C:\Program Files (x86)\Pidgin\plugins folder

## Compiling
Needs dependencies libglib2.0-dev libgtk2.0-dev libpurple-dev pidgin-dev
```
git clone https://github.com/EionRobb/pidgin-paste-image
cd pidgin-paste-image
make
sudo make install
```
