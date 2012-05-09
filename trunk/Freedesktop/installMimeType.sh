# This is an attempt at installing Aria Maestosa in a Freedesktop-compliant way, with a .desktop file and correct file associations.
# At the moment, file associations "sort of" work (the file icon shows and opening works, but if you open multiple files, multiple
# Aria processes will be launched).
# This script will also install Aria in /usr/share/applications (path not configurable for now, sorry)

# copies the icon file into '~/.local/share/icons/hicolor/64x64/mimetypes'
xdg-icon-resource install --context mimetypes --size 64 ./aria_doc.png application-x-ariamaestosa

# this copies the file into '~/.local/share/mime/packages'
xdg-mime install ./application-x-ariamaestosa.xml

sudo install ./Aria.desktop /usr/share/applications
sudo update-desktop-database /usr/share/applications
