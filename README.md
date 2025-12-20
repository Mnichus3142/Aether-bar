# Aether-bar

<h3 align="center">Aether-bar is a bar designed for my hyprland setup.</h3>

<hr>

<div align="center">

![GitHub last commit](https://img.shields.io/github/last-commit/Mnichus3142/Aether-bar?style=for-the-badge&labelColor=000&color=fff)![GitHub Created At](https://img.shields.io/github/created-at/Mnichus3142/Aether-bar?style=for-the-badge&labelColor=000&color=fff)![GitHub License](https://img.shields.io/github/license/Mnichus3142/Aether-bar?style=for-the-badge&labelColor=000&color=fff)![GitHub top language](https://img.shields.io/github/languages/top/Mnichus3142/Aether-bar?style=for-the-badge&labelColor=000&color=fff)

</div>

<hr>

Aether-bar is a part of my Aether ecosystem made for Hyprland. Check ![here](https://github.com/Mnichus3142/Aether) for details. 

On my machine it is using around 200MB of RAM and around 0.7% of CPU, so it is pretty lightweight for webview based bar.

Here you have screenshot of my setup.

![Screenshot](Screenshot.png)

Config should automatically be created in ```.config/Aether-bar``` and that config is prefered, so if you want to change something, feel free to do it there. If you want more functionalities, you can edit ```static/app.js``` file and copy it to ```target/release/static/```. Sometimes you need to rebuild the app to see changes.

You are able to delete every component and move it from container to container and change clock format. You can delete hours, minutes or seconds and change clock format from 12-hour to 24-hour changing hh to HH

To build an app everything you need is 

```
git clone https://github.com/Mnichus3142/Aether-bar
cd Aether-bar
cargo build --release
cp -r static target/release/static
```

Then run an app

```
./target/release/Aether-bar
```

Made with ❤️ by [Mnichus3142](https://github.com/Mnichus3142)
